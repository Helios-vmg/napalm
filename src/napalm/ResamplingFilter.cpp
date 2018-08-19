#include "Filter.h"
#include "utility.h"
#include <samplerate.h>

ResamplingFilter::ResamplingFilter(std::unique_ptr<BufferSource> &&source, const AudioFormat &af, int frequency, ResamplerPreset preset):
		FilterSource(std::move(source)),
		source_filter(af),
		resampler(nullptr, src_delete),
		passed(nullptr, release_buffer){

	this->preset = preset;
	if (af.format != Float32)
		throw std::runtime_error("ResamplingFilter only resamples float samples.");
	this->frequency = frequency;
	this->ratio = (double)this->frequency / af.freq;
	this->rational_ratio = {this->frequency, af.freq};
	this->bytes_per_sample = sizeof(float) * af.channels;
	this->reset();
}

void ResamplingFilter::reset(){
	int error;
	this->resampler.reset(src_callback_new(samplerate_callback, (int)this->preset, this->source_filter.channels, &error, this));
	if (error)
		throw std::runtime_error((std::string)"libsamplerate error: " + src_strerror(error));
	this->extra_info.reset();
}

audio_buffer_t ResamplingFilter::read(){
	const long max_samples = 1024;
	auto ret = allocate_buffer(max_samples * this->bytes_per_sample, sizeof(BufferExtraData));
	auto samples = src_callback_read(this->resampler.get(), this->ratio, max_samples, (float *)ret->data);
	if (samples < 0){
		auto error = (int)samples;
		throw std::runtime_error((std::string)"libsamplerate error: " + src_strerror(error));
	}
	if (!samples)
		ret.reset();
	else{
		ret->sample_count = (size_t)samples;

		assert(this->extra_info);
		auto &extra_data = get_extra_data<BufferExtraData>(ret);
		extra_data.timestamp = to_RationalValue(this->extra_info->current_time);
		extra_data.stream_id = this->extra_info->stream_id;
		extra_data.next = nullptr;
		this->extra_info->current_time += rational_t(ret->sample_count, frequency);
	}

	return ret;
}

void ResamplingFilter::flush(){
	this->source->flush();
	src_reset(this->resampler.get());
	this->extra_info.reset();
	//this->reset();
}

long ResamplingFilter::callback(float *&data){
	this->passed = this->source->read();
	if (!this->passed)
		return 0;
	if (this->passed->sample_count > (size_t)std::numeric_limits<long>::max())
		throw std::runtime_error("ResamplingFilter overflow");
	if (!this->extra_info){
		this->extra_info.emplace();
		auto &extra_data = get_extra_data<BufferExtraData>(this->passed);
		this->extra_info->current_time = rational_t{extra_data.timestamp.numerator, extra_data.timestamp.denominator};
		this->extra_info->stream_id = extra_data.stream_id;
	}
	data = (float *)this->passed->data;
	return (long)this->passed->sample_count;
}
