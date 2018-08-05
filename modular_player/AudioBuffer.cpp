#include "AudioBuffer.h"
#include <cstdlib>
#include <limits>

const rational_t invalid_audio_position(-1, 1);
const std::uint64_t invalid_sample_count = std::numeric_limits<std::uint64_t>::max();

AudioBuffer::AudioBuffer(int bytes_per_channel, int channels, size_t samples){
	this->alloc(bytes_per_channel, channels, samples);
}

AudioBuffer::~AudioBuffer(){
	this->free();
}

void AudioBuffer::alloc(size_t bytes){
	this->data = (std::uint8_t *)malloc(bytes);
}

void AudioBuffer::alloc(int bytes_per_channel, int channels, size_t samples){
	auto bps = bytes_per_channel * channels;
	size_t n = samples * bps;
	this->alloc(n);
	this->bps = bps;
	this->length = n;
	this->sample_count = samples;
	this->channel_count = channels;
}

void AudioBuffer::free(){
	if (this->data)
		::free(this->data);
}
