#include "Filter.h"
#include "utility.h"
#include <samplerate.h>

ResamplingFilter::ResamplingFilter(std::unique_ptr<BufferSource> &&source, const AudioFormat &af, int frequency, ResamplerPreset preset):
		FilterSource(std::move(source)),
		source_filter(af),
		resampler(nullptr, src_delete),
		passed(nullptr, release_buffer){

	if (af.format != Float32)
		throw std::runtime_error("ResamplingFilter only resamples float samples.");
	this->frequency = frequency;
	this->ratio = (double)this->frequency / af.freq;
	int error;
	resampler.reset(src_callback_new(samplerate_callback, (int)preset, af.channels, &error, this));
	if (error)
		throw std::runtime_error((std::string)"libsamplerate error: " + src_strerror(error));
	this->bytes_per_sample = sizeof(float) * af.channels;
}

read_buffer_t ResamplingFilter::read(){
	size_t max_samples = 1024 * 32;
	auto ret = allocate_buffer(max_samples * this->bytes_per_sample);
	auto samples = src_callback_read(resampler.get(), this->ratio, max_samples, (float *)ret->data);
	if (samples < 0){
		auto error = (int)samples;
		throw std::runtime_error((std::string)"libsamplerate error: " + src_strerror(error));
	}
	if (!samples)
		ret.reset();
	else
		ret->sample_count = (size_t)samples;
	return ret;
}

long ResamplingFilter::callback(float *&data){
	this->passed = this->source->read();
	if (!this->passed)
		return 0;
	if (this->passed->sample_count > (size_t)std::numeric_limits<long>::max())
		throw std::runtime_error("ResamplingFilter overflow");
	data = (float *)this->passed->data;
	return (long)this->passed->sample_count;
}
