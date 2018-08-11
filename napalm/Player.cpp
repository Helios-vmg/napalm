#include "Player.h"
#include "Module.h"
#include "utility.h"
#include "Filter.h"
#include <samplerate.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include <type_traits>
#include <chrono>

Player::Player(){
	this->load_plugins();
	this->open_output();
}

void Player::load_file(const char *path){
	this->decoder = this->decoders.front()->open(std::make_unique<StdInputStream>(path));
}

template <typename T>
void basic_convert(std::vector<float> &ret, const AudioBuffer &rr, float sub, float mul){
	auto data = (const T *)rr.data;
	auto n = ret.size();
	for (size_t i = 0; i < n; i++){
		auto sample = data[i];
		ret[i] = ((float)sample - sub) * mul;
	}
}

void unsigned_convert24(std::vector<float> &ret, const AudioBuffer &rr){
	auto data = rr.data;
	const auto max = -(float)(1U << 23);
	auto n = ret.size();
	for (size_t i = 0; i < n; i++){
		auto p = data + i * 3;
		std::uint32_t sample = p[0];
		sample |= p[1] << 8;
		sample |= p[2] << 16;
		ret[i] = ((float)sample - max) * (1.f / max);
	}
}

void signed_convert24(std::vector<float> &ret, const AudioBuffer &rr){
	auto data = rr.data;
	const auto max = -(float)(1U << 23);
	auto n = ret.size();
	for (size_t i = 0; i < n; i++){
		auto p = data + i * 3;
		std::int32_t sample = (std::int32_t)p[0];
		sample |= (std::int32_t)p[1] << 8;
		sample |= (std::int32_t)p[2] << 16;
		sample = (sample << 8) >> 8;
		ret[i] = (float)sample * (1.f / max);
	}
}

#define NEXT(x) get_extra_data<BufferExtraData>(x).next

void Player::play(){
	int n = this->decoder->get_substreams_count();

	auto bps = sizeof_NumberFormat(this->final_format.format) * this->final_format.channels;

	for (int i = 0; i < n; i++){
		auto stream = this->decoder->get_substream(i);
		stream->seek_to_second(stream->get_length_in_seconds() - rational_t(5, 1), false);
		std::cout << "Playing stream " << i << std::endl;
		auto length = stream->get_length_in_seconds();

		std::ofstream file("output.raw", std::ios::binary);

		auto t0 = std::chrono::high_resolution_clock::now();
		{
			auto format = stream->get_audio_format();
			if (format.freq != this->final_format.freq){
				stream->set_number_format_hint(Float32);
				format = stream->get_audio_format();
			}
			auto filter = build_filter_chain(std::move(stream), format, this->final_format);
			while (auto buffer = filter->read()){
				if (!buffer->sample_count)
					break;
				file.write((const char *)buffer->data, buffer->sample_count * bps);
				file.flush();
				while (true){
					{
						std::lock_guard<std::mutex> lg(this->queue_mutex);
						if (this->queue_size < 1){
							this->queue_size += rational_t(buffer->sample_count, this->final_format.freq);
							if (this->queue_tail){
								NEXT(this->queue_tail) = buffer.get();
								this->queue_tail = buffer.release();
							}else
								this->queue_tail = this->queue_head = buffer.release();
							get_extra_data<BufferExtraData>(this->queue_tail).sample_offset = 0;
							break;
						}
					}
					this->queue_event.wait();
				}
				//if (!this->output_device->send_audio(buffer))
				//	std::cout << "send_audio() failed!\n";
			}
		}
		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout << (double)length.numerator() / (double)length.denominator() / ((t1 - t0).count() * 1e-9) << "x\n";
	}
}

void Player::load_plugins(){
	typedef boost::filesystem::directory_iterator D;
	for (D i("./plugins"), end; i != end; ++i){
		auto path = i->path().wstring();
		if (!glob(L"*.dll", path.c_str(), tolower))
			continue;
		auto module = Module::load(path);
		if (!module)
			continue;
		if (module->module_supports(DECODER_MODULE_TYPE))
			this->decoders.emplace_back(std::make_unique<DecoderModule>(module));
		if (module->module_supports(OUTPUT_MODULE_TYPE))
			this->outputs.emplace_back(std::make_unique<OutputModule>(module));
	}
}

void Player::open_output(){
	if (!this->outputs.size())
		throw std::runtime_error("No outputs available.");
	for (auto &output : this->outputs)
		for (auto &kv : output->get_devices())
			this->devices[kv.first] = kv.second;
	if (!this->devices.size())
		throw std::runtime_error("No devices available.");

	//Test code:
	for (auto &output : this->outputs){
		for (auto &kv : output->get_devices()){
			try{
				auto formats = kv.second->get_preferred_formats();
				if (!formats.size())
					continue;
				kv.second->open(
					0,
					[this](void *dst, size_t size, size_t samples_queued){
						return this->audio_callback(dst, size, samples_queued);
					},
					[this](const AudioFormat &af){
						this->audio_format_changed(af);
					}
				);
				this->output_device = kv.second;
				this->final_format = formats.front();
				return;
			}catch (std::exception &e){}
		}
	}
}

#include <iostream>

size_t Player::audio_callback(void *void_dst, size_t size, size_t samples_queued){
	size_t ret = 0;
	bool signal = false;
	{
		std::lock_guard<std::mutex> lg(this->queue_mutex);
		auto dst = (std::uint8_t *)void_dst;
		auto sample_size = sizeof_NumberFormat(this->final_format.format) * this->final_format.channels;
		while (size){
			if (!this->queue_head)
				break;
			auto &extra = get_extra_data<BufferExtraData>(this->queue_head);
			auto copy_size = std::min(size, this->queue_head->sample_count - extra.sample_offset);
			memcpy(dst, this->queue_head->data + extra.sample_offset * sample_size, copy_size * sample_size);
			extra.sample_offset += copy_size;
			ret += copy_size;
			dst += copy_size * sample_size;
			size -= copy_size;
			if (extra.sample_offset >= this->queue_head->sample_count){
				auto old = this->queue_head;
				this->queue_size -= rational_t(old->sample_count, this->final_format.freq);
				this->queue_head = NEXT(this->queue_head);
				if (!this->queue_head)
					this->queue_tail = nullptr;
				release_buffer(old);
				signal = true;
			}
		}
		
	}
	if (signal || !ret)
		this->queue_event.signal();
	return ret;
}

void Player::audio_format_changed(const AudioFormat &af){
	__debugbreak();
}
