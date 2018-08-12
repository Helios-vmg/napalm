#include "Player.h"
#include "Module.h"
#include "utility.h"
#include "Filter.h"
#include <samplerate.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include <type_traits>
#include <chrono>

Player::Player(): queue(this->final_format){
	this->load_plugins();
	this->open_output();
}

void Player::load_file(const char *path){
	if (!decoders.size())
		throw std::runtime_error((std::string)"No decoders to load file " + path);
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

void Player::play(){
	int n = this->decoder->get_substreams_count();

	auto bps = sizeof_NumberFormat(this->final_format.format) * this->final_format.channels;

	for (int i = 0; i < n; i++){
		auto stream = this->decoder->get_substream(i);
		//stream->seek_to_second(stream->get_length_in_seconds() - rational_t(5, 1), false);
		std::cout << "Playing stream " << i << std::endl;

		auto format = stream->get_audio_format();
		if (format.freq != this->final_format.freq){
			stream->set_number_format_hint(Float32);
			format = stream->get_audio_format();
		}
		{
			std::lock_guard<std::mutex> lg(this->mutex);
			this->now_playing = build_filter_chain(std::move(stream), format, this->final_format);
		}
		while (true){
			audio_buffer_t buffer(nullptr, release_buffer);
			{
				std::lock_guard<std::mutex> lg(this->mutex);
				format = this->final_format;
				buffer = this->now_playing->read();
			}
			if (!buffer || !buffer->sample_count)
				break;
			this->queue.push_to_queue(std::move(buffer), format);
		}
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
				auto &queue = this->queue;
				kv.second->open(
					0,
					[&queue](void *dst, size_t size, size_t samples_queued){
						return queue.pop_buffer(dst, size, samples_queued);
					},
					[this](const AudioFormat &af){
						this->audio_format_changed(af);
					}
				);
				this->output_device = kv.second;
				this->final_format = formats.front();
				this->queue.set_expected_format(this->final_format);
				return;
			}catch (std::exception &e){}
		}
	}
}

void Player::audio_format_changed(const AudioFormat &af){
	auto old_format = this->final_format;
	if (af == old_format){
		std::cout << "Default device changed. Nothing special to do.\n";
		return;
	}

	this->queue.set_expected_format(af);

	std::lock_guard<std::mutex> lg(this->mutex);
	auto temp = this->now_playing->unroll_chain();
	if (temp)
		this->now_playing = std::move(temp);
	auto extra = this->queue.flush_queue();
	auto substream = static_cast<DecoderSubstream *>(this->now_playing.get());
	if (substream->get_stream_id() == extra.stream_id){
		rational_t time(extra.timestamp.numerator, extra.timestamp.denominator);
		time += rational_t(extra.sample_offset, old_format.freq);
		substream->seek_to_second(time, false);
	}
	auto format = substream->get_audio_format();
	this->final_format = af;
	std::cout << "Default device changed. Format was changed.\n"
		"Old format: " << old_format << "\n"
		"New format: " << af << std::endl;
	this->now_playing = build_filter_chain(std::move(this->now_playing), format, this->final_format);
}
