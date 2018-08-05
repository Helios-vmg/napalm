#pragma once

#include "Decoder.h"
#include "BufferSource.h"
#include <deque>

class TrueFilter{
protected:
	AudioFormat source_format;
	size_t bytes_per_source_sample;
public:
	TrueFilter(const AudioFormat &af);
	virtual ~TrueFilter(){}
	virtual read_buffer_t apply(read_buffer_t &&) = 0;
};

class FilterSource : public BufferSource{
protected:
	std::unique_ptr<BufferSource> source;
public:
	FilterSource(std::unique_ptr<BufferSource> &&source): source(std::move(source)){}
	virtual ~FilterSource() = 0{}
};

template <typename T>
class GenericFilterSource : public FilterSource{
	T filter;
public:
	template <typename... Args>
	GenericFilterSource(std::unique_ptr<BufferSource> &&source, Args &&... args):
		FilterSource(std::move(source)),
		filter(std::forward<Args>(args)...){}
	read_buffer_t read() override{
		auto buffer = this->source->read();
		if (!buffer)
			return buffer;
		return this->filter.apply(std::move(buffer));
	}
};

class ChannelMixerFilter : public TrueFilter{
	int channels;
	typedef read_buffer_t (*f)(read_buffer_t &&, size_t);
	f converter;
	size_t bytes_per_dest_sample;
public:
	ChannelMixerFilter(const AudioFormat &af, int target_channels);
	read_buffer_t apply(read_buffer_t &&);
};

class SampleConverterFilter : public TrueFilter{
	NumberFormat type;
	size_t bytes_per_dest_sample;
	typedef read_buffer_t (*f)(read_buffer_t &&, size_t, size_t);
	f converter;
public:
	SampleConverterFilter(const AudioFormat &af, NumberFormat type);
	read_buffer_t apply(read_buffer_t &&);
};

typedef GenericFilterSource<ChannelMixerFilter> ChannelMixerFilterSource;
typedef GenericFilterSource<SampleConverterFilter> SampleConverterFilterSource;

struct SRC_STATE_tag;
typedef struct SRC_STATE_tag SRC_STATE;

/*
 * The presets are sorted in order of decreasing output quality and increasing speed.
 * Below are the approximate relative speeds and distortion of each setting:
 *
 *                        speed          distortion
 * SincBestQuality:        1.0 (base)    -39    dB
 * SincMediumQuality:      3.1	         -24    dB
 * SincFastest:            5.2	         -16    dB
 * LinearInterpolation:   12.1	          -2.4  dB
 * ZeroOrderHold:         12.6	           0.17 dB
 *
 * Methodology: The distortion was measured by resampling an 8 kHz track to
 * 44.1 kHz with each setting and subtracting each result with the output of
 * Audacity's resample function. Then for each subtracted track, the ReplayGain
 * was computed. The distortion is the RG value for the track with the sign
 * inverted.
 * The distortion value is therefore approximately the worst case distortion
 * for that algorithm.
 */
enum class ResamplerPreset{
	SincBestQuality = 0,
	SincMediumQuality = 1,
	SincFastest = 2,
	LinearInterpolation = 4,
	ZeroOrderHold = 3,
};

class ResamplingFilter : public FilterSource{
	AudioFormat source_filter;
	int frequency;
	double ratio;
	size_t bytes_per_sample;
	std::unique_ptr<SRC_STATE, SRC_STATE *(*)(SRC_STATE *)> resampler;
	read_buffer_t passed;

	static long samplerate_callback(void *cb_data, float **data){
		return ((ResamplingFilter *)cb_data)->callback(*data);
	}
	long callback(float *&data);
public:
	ResamplingFilter(std::unique_ptr<BufferSource> &&, const AudioFormat &af, int frequency, ResamplerPreset preset = ResamplerPreset::SincBestQuality);
	read_buffer_t read() override;
};

std::unique_ptr<BufferSource> build_filter_chain(std::unique_ptr<BufferSource> &&source, const AudioFormat &saf, const AudioFormat &daf);
