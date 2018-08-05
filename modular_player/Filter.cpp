#include "Filter.h"
#include <iostream>

TrueFilter::TrueFilter(const AudioFormat &af): source_format(af){
	this->bytes_per_source_sample = this->source_format.channels * sizeof_NumberFormat(this->source_format.format);
}

std::unique_ptr<BufferSource> build_filter_chain(std::unique_ptr<BufferSource> &&source, const AudioFormat &saf, const AudioFormat &daf){
	auto f = saf;
	auto ret = std::move(source);
	if (f.format != daf.format && f.freq == daf.freq){
		ret = std::make_unique<SampleConverterFilterSource>(std::move(ret), f, daf.format);
		f.format = daf.format;
	}
	if (f.channels != daf.channels){
		ret = std::make_unique<ChannelMixerFilterSource>(std::move(ret), f, daf.channels);
		f.channels = daf.channels;
	}
	if (f.freq != daf.freq){
		if (f.format != Float32){
			ret = std::make_unique<SampleConverterFilterSource>(std::move(ret), f, Float32);
			f.format = Float32;
		}
		ret = std::make_unique<ResamplingFilter>(std::move(ret), f, daf.freq, ResamplerPreset::SincBestQuality);
		f.freq = daf.freq;
		f.format = Float32;
	}
	if (f.format != daf.format){
		ret = std::make_unique<SampleConverterFilterSource>(std::move(ret), f, daf.format);
		f.format = daf.format;
	}
	return ret;
}
