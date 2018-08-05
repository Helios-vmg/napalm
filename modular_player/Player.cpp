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
}

void Player::load_file(const char *path){
	this->decoder = this->decoders.front()->open(std::make_unique<StdInputStream>(path));
}

template <typename T>
void basic_convert(std::vector<float> &ret, const ReadResult &rr, float sub, float mul){
	auto data = (const T *)rr.data;
	auto n = ret.size();
	for (size_t i = 0; i < n; i++){
		auto sample = data[i];
		ret[i] = ((float)sample - sub) * mul;
	}
}

template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, void>::type
convert(std::vector<float> &ret, const ReadResult &rr){
	const auto max = -(float)std::numeric_limits<typename std::make_signed<T>::type>::min();
	basic_convert<T>(ret, rr, max, 1.f / max);
}

template <typename T>
typename std::enable_if<std::is_signed<T>::value, void>::type
convert(std::vector<float> &ret, const ReadResult &rr){
	const auto max = -(float)std::numeric_limits<T>::min();
	basic_convert<T>(ret, rr, 0, 1.f / max);
}

void unsigned_convert24(std::vector<float> &ret, const ReadResult &rr){
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

void signed_convert24(std::vector<float> &ret, const ReadResult &rr){
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

std::vector<float> convert(const ReadResult &rr, const AudioFormat &format){
	std::vector<float> ret(rr.sample_count * format.channels);
	switch (format.format){
		case IntU8:
			convert<std::uint8_t>(ret, rr);
			break;
		case IntU16:
			convert<std::uint16_t>(ret, rr);
			break;
		case IntU24:
			unsigned_convert24(ret, rr);
			break;
		case IntU32:
			convert<std::uint32_t>(ret, rr);
			break;
		case IntS8:
			convert<std::int8_t>(ret, rr);
			break;
		case IntS16:
			convert<std::int16_t>(ret, rr);
			break;
		case IntS24:
			signed_convert24(ret, rr);
			break;
		case IntS32:
			convert<std::int32_t>(ret, rr);
			break;
		case Float32:
			memcpy(&ret[0], rr.data, ret.size() * sizeof(float));
			break;
		case Float64:
			{
				auto data = (const double *)rr.data;
				auto n = ret.size();
				std::copy(data, data + n, ret.begin());
			}
			break;
		default:
			break;
	}
	return ret;
}

class SamplerateState{
	DecoderSubstream *stream;
	AudioFormat format;
	std::vector<float> data;
	long samplerate_callback(float *&data){
		auto result = this->stream->read();
		if (!result)
			return 0;
		this->data = convert(*result, this->format);
		data = &this->data[0];
		return result->sample_count;
	}
public:
	SamplerateState(DecoderSubstream &stream, const AudioFormat &format):
		stream(&stream),
		format(format){}
	SamplerateState(const SamplerateState &) = default;
	SamplerateState(SamplerateState &&) = default;
	SamplerateState &operator=(const SamplerateState &) = default;
	SamplerateState &operator=(SamplerateState &&) = default;
	static long samplerate_callback(void *cb_data, float **data){
		return ((SamplerateState *)cb_data)->samplerate_callback(*data);
	}
};

void Player::play(){
	std::ofstream file("output.raw", std::ios::binary);
	int n = this->decoder->get_substreams_count();

	AudioFormat final_format;
	final_format.format = Float32;
	final_format.freq = 44100;
	final_format.channels = 2;

	for (int index = 0; index < n; index++){
		auto stream = this->decoder->get_substream(index);
		std::cout << "Playing stream " << index << std::endl;
		auto length = stream->get_length_in_seconds();

		auto t0 = std::chrono::high_resolution_clock::now();
		{
			auto format = stream->get_audio_format();
			if (format.freq != final_format.freq){
				stream->set_number_format_hint(Float32);
				format = stream->get_audio_format();
			}
			auto filter = build_filter_chain(std::move(stream), format, final_format);
			while (auto buffer = filter->read()){
				if (!buffer->sample_count)
					break;
				file.write((const char *)buffer->data, buffer->sample_count * final_format.channels * sizeof_NumberFormat(final_format.format));
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
		if (module->module_supports("decoder"))
			this->decoders.push_back(std::make_unique<DecoderModule>(module));
	}
}
