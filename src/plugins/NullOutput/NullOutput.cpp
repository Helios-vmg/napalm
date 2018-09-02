#include "NullOutput.h"
#include <utf8.hpp>
#include <sha256.hpp>
#include <sstream>
#include <vector>

const std::string NullDevice::name = "Null Output Device";

NullOutput::NullOutput(): device(*this){}

NullOutput::~NullOutput(){}

void NullOutput::set_error(const std::string &error){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	this->errors[id] = error;
}

const char *NullOutput::get_error(){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	auto it = this->errors.find(id);
	if (it == this->errors.end())
		return nullptr;
	return it->second.c_str();
}

void NullOutput::clear_error(){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	auto it = this->errors.find(id);
	if (it == this->errors.end())
		return;
	this->errors.erase(it);
}


class NullOutputDeviceList{
	std::vector<std::string> strings;
	std::vector<OutputDeviceListItem> items;
	OutputDeviceList odl;

	static void release(void *This){
		delete (NullOutputDeviceList *)This;
	}
public:
	NullOutputDeviceList(const NullDevice &device){
		this->strings.push_back(device.get_name());
		this->items.resize(1);
		auto &back = this->items.back();
		back.name = this->strings.back().c_str();
		memcpy(back.unique_id.unique_id, device.get_hash().data(), sizeof(back.unique_id.unique_id));
		
		this->odl.opaque = this;
		this->odl.release_function = release;
		this->odl.length = 1;
		this->odl.items = &this->items[0];
	}
	OutputDeviceList *get_list(){
		return &this->odl;
	}
};

OutputDeviceList *NullOutput::get_device_list(){
	auto list = new NullOutputDeviceList(this->device);
	return list->get_list();
}

AudioFormat *NullOutput::get_supported_formats(const UniqueID &unique_id){
	return this->device.get_supported_formats();
}

OutputDevice *NullOutput::open_device(const UniqueID &unique_id, size_t format_index, const AudioCallbackData &callback){
	this->device.open(format_index, callback);
	return &this->device;
}

NullDevice::NullDevice(NullOutput &parent): parent(&parent), run(false){
	SHA256 hash;
	hash.input(this->name);
	this->hash = hash.result();
	this->formats[0] = {IntS16, 2, 44100};
	this->formats[1] = {Invalid, 0, 0};
}

NullDevice::~NullDevice(){
	this->close();
}

void NullDevice::close(){
	if (!this->thread.joinable())
		return;
	this->run = false;
	this->thread.join();
	this->thread = std::thread();
}

void NullDevice::open(size_t format_index, const AudioCallbackData &callback){
	if (format_index > 0)
		throw std::runtime_error("Invalid format index.");

	this->callback = callback;
	this->run = true;
	this->thread = std::thread([this](){ this->thread_func(); });
}

void NullDevice::thread_func(){
	auto initial = std::chrono::high_resolution_clock::now();
	std::uint64_t samples_read = 0;
	auto bytes_per_sample = sizeof_NumberFormat(this->formats[0].format) * this->formats[0].channels;
	std::vector<char> bitbucket;
	while (this->run){
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		auto now = std::chrono::high_resolution_clock::now();
		auto nanoseconds = (double)(now - initial).count();
		auto samples = (std::uint64_t)(nanoseconds * (441.0 / 10000000.0) + 0.5);
		auto samples_to_read = samples - samples_read;
		auto bytes_to_read = samples_to_read * bytes_per_sample;
		if (bytes_to_read){
			if (bitbucket.size() < bytes_to_read)
				bitbucket.resize(bytes_to_read);
			this->callback.audio_callback(this->callback.cb_data, &bitbucket[0], samples_to_read, 0);
		}
		samples_read = samples;
	}
}

AudioFormat *NullDevice::get_supported_formats(){
	return this->formats;
}
