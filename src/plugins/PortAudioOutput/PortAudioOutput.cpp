#include "PortAudioOutput.h"
#include <utf8.hpp>
#include <sha256.hpp>
#include <sstream>

PortAudioOutput::PortAudioOutput(){
	auto error = Pa_Initialize();
	if (error != paNoError)
		throw std::runtime_error((std::string)"Failed to initialize PortAudio: " + Pa_GetErrorText(error));
}

void PortAudioOutput::set_error(const std::string &error){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	this->errors[id] = error;
}

const char *PortAudioOutput::get_error(){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	auto it = this->errors.find(id);
	if (it == this->errors.end())
		return nullptr;
	return it->second.c_str();
}

void PortAudioOutput::clear_error(){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	auto it = this->errors.find(id);
	if (it == this->errors.end())
		return;
	this->errors.erase(it);
}

class PortAudioDeviceList{
	friend class PortAudioOutput;
	std::vector<std::string> names;
	std::vector<PaDeviceIndex> device_indices;
	std::vector<AudioFormat> formats_helper;
	std::vector<size_t> formats;
	std::vector<OutputDeviceListItem> items;
	OutputDeviceList odl;

	static void release(void *This){
		//delete (PortAudioDeviceList *)This;
	}
public:
	PortAudioDeviceList(){
		this->device_indices.push_back(Pa_GetDefaultOutputDevice());
		this->names.push_back("Default output device");
		auto device_count = Pa_GetDeviceCount();
		for (PaDeviceIndex i = 0; i < device_count; i++){
			auto info = Pa_GetDeviceInfo(i);
			if (info->maxOutputChannels < 1)
				continue;

			static const NumberFormat formats[] = {
				Float32,
				IntS16,
				IntS24,
				IntS32,
				IntS8,
				IntU8,
				Invalid,
			};
			this->formats.push_back(this->formats_helper.size());
			AudioFormat format = {Invalid, info->maxOutputChannels, (int)info->defaultSampleRate};
			for (auto f : formats){
				auto copy = format;
				copy.format = f;
				this->formats_helper.push_back(copy);
			}
			this->device_indices.push_back(i);
			auto host_api = Pa_GetHostApiInfo(info->hostApi);
			std::string name = info->name;
			name += " | ";
			name += host_api->name;
			this->names.push_back(name);
		}

		this->items.resize(this->names.size());

		for (size_t i = 0; i < this->names.size(); i++){
			auto &item = this->items[i];
			auto &name = this->names[i];
			item.name = name.c_str();
			SHA256 hash;
			std::stringstream hash_input;
			hash_input << name << '/' << this->device_indices[i];
			hash.input(hash_input.str());
			memcpy(item.unique_id.unique_id, hash.result().data(), sizeof(item.unique_id.unique_id));
		}

		this->odl.opaque = this;
		this->odl.release_function = release;
		this->odl.length = this->items.size();
		this->odl.items = &this->items[0];
	}
	OutputDeviceList *get_list(){
		return &this->odl;
	}
};

PortAudioOutput::~PortAudioOutput(){
	this->device_list.reset();
	Pa_Terminate();
}

OutputDeviceList *PortAudioOutput::get_device_list(){
	this->device_list = std::make_unique<PortAudioDeviceList>();
	return this->device_list->get_list();
}

AudioFormat *PortAudioOutput::get_supported_formats(const UniqueID &unique_id){
	auto &list = *this->device_list;
	for (size_t i = 0; i < list.items.size(); i++){
		if (memcmp(list.items[i].unique_id.unique_id, unique_id.unique_id, sizeof(unique_id.unique_id)))
			continue;
		return &list.formats_helper[list.formats[i]];
	}
	return nullptr;
}

OutputDevice *PortAudioOutput::open_device(const UniqueID &unique_id, size_t format_index, const AudioCallbackData &callback){
	auto &list = *this->device_list;
	for (size_t i = 0; i < list.items.size(); i++){
		if (memcmp(list.items[i].unique_id.unique_id, unique_id.unique_id, sizeof(unique_id.unique_id)))
			continue;
		auto index = list.device_indices[i];
		return new PortAudioDevice(*this, callback, index, list.formats_helper[list.formats[format_index]]);
	}
	return nullptr;
}

PaSampleFormat to_sample_format(NumberFormat format){
	switch (format){
		case Float32:
			return paFloat32;
		case IntS16:
			return paInt16;
		case IntS24:
			return paInt24;
		case IntS32:
			return paInt32;
		case IntS8:
			return paInt8;
		case IntU8:
			return paUInt8;
		default:
			break;
	}
	throw std::exception();
}

PortAudioDevice::PortAudioDevice(PortAudioOutput &parent, const AudioCallbackData &callback, PaDeviceIndex index, const AudioFormat &format):
		OutputDevice(callback),
		parent(&parent),
		index(index),
		format(format){

	this->bytes_per_sample = sizeof_NumberFormat(format.format) * format.channels;
	PaStreamParameters params;
	params.device = index;
	params.channelCount = format.channels;
	params.sampleFormat = to_sample_format(format.format);
	params.suggestedLatency = .01;
	params.hostApiSpecificStreamInfo = nullptr;
	PaError error = Pa_OpenStream(&this->stream, nullptr, &params, format.freq, 1024, paNoFlag, audio_callback, this);
	if (error != paNoError)
		throw std::runtime_error((std::string)"Failed to initialized PortAudio stream: " + Pa_GetErrorText(error));
	error = Pa_StartStream(this->stream);
	if (error != paNoError)
		throw std::runtime_error((std::string)"Failed to open PortAudio stream: " + Pa_GetErrorText(error));
}

PortAudioDevice::~PortAudioDevice(){
	this->close();
}

void PortAudioDevice::close(){
	if (!this->stream)
		return;
	Pa_CloseStream(this->stream);
	this->stream = nullptr;
}

static auto initial = std::chrono::high_resolution_clock::now();

int PortAudioDevice::audio_callback(const void *input, void *output, unsigned long frame_count, const PaStreamCallbackTimeInfo *time_info, PaStreamCallbackFlags status_flags, void *user_data){
	static decltype(initial) last = std::chrono::high_resolution_clock::now();;
	auto This = (PortAudioDevice *)user_data;
	auto dst = (std::uint8_t *)output;
	size_t size = frame_count;
	auto bps = This->bytes_per_sample;
	auto now = std::chrono::high_resolution_clock::now();
	if (This->callback.audio_callback){
		auto read = This->callback.audio_callback(This->callback.cb_data, dst, size, 0);
		size -= read;
		if (!size){
			auto delta = (now - last).count() * 1e-9;
			last = now;
			return paContinue;
		}
		dst += bps * read;
	}
	if (size)
		memset(dst, 0, size * bps);
	auto delta = (now - last).count() * 1e-9;
	last = now;
	return paContinue;
}
