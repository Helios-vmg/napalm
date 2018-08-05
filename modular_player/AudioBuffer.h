#pragma once

#include <vector>
#include <cstdint>
#include <boost/rational.hpp>

#if defined _MSC_VER || defined __GNUC__
#pragma pack(push, 1)
#define PACKING_ATTRIBUTE
#elif defined __clang__
#define PACKING_ATTRIBUTE __attribute__((packed))
#endif

template <typename NumberT, unsigned Channels>
struct PACKING_ATTRIBUTE sample_t{
	NumberT values[Channels];
};

template <typename NumberT>
struct PACKING_ATTRIBUTE sample_t<NumberT, 0>{
	NumberT values;
};

#if defined _MSC_VER || defined __GNUC__
#pragma pack(pop)
#endif

typedef boost::rational<std::int64_t> rational_t;
extern const rational_t invalid_audio_position;
extern const std::uint64_t invalid_sample_count;

class AudioBuffer{
	std::uint8_t *data;
	size_t length;
	int bps;
	size_t sample_count;
	int channel_count;

	void alloc(size_t bytes);
	void alloc(int bytes_per_channel, int channels, size_t samples);
	void free();
public:
	AudioBuffer():
		data(nullptr),
		length(0),
		bps(0),
		sample_count(0),
		channel_count(0){}
	AudioBuffer(int bytes_per_channel, int channels, size_t samples);
	~AudioBuffer();
	AudioBuffer(AudioBuffer &&other){
		*this = std::move(other);
	}
	const AudioBuffer &operator=(AudioBuffer &&other){
		this->data = other.data;
		this->length = other.length;
		this->bps = other.bps;
		this->sample_count = other.sample_count;
		this->channel_count = other.channel_count;
		other.data = nullptr;
		other.length = 0;
		return *this;
	}
	AudioBuffer(const AudioBuffer &other) = delete;
	const AudioBuffer &operator=(const AudioBuffer &other) = delete;
	template <typename NumberT, unsigned Channels>
	sample_t<NumberT, Channels> &get(size_t sample_no){
		return *(sample_t<NumberT, Channels> *)(this->data + sample_no * sizeof(sample_t<NumberT, Channels>));
	}
	size_t get_sample_count() const{
		return this->sample_count;
	}
	void set_sample_count(size_t n){
		this->sample_count = n;
	}
	operator bool() const{
		return !!this->data;
	}
};
