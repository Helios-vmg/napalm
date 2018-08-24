#pragma once

#include "WrappedExternalIO.hpp"
#include "decoder_module.h"
#include "audio_format.h"
#include <limits>
#include <cstdint>
#include <boost/rational.hpp>
#include <vector>
#include <memory>

typedef boost::rational<std::int64_t> rational_t;

template <typename T>
double to_double(const boost::rational<T> &r){
	return (double)r.numerator() / (double)r.denominator();
}

template <typename T, T limit>
boost::rational<T> to_rational(double x){
	typedef boost::rational<T> R;
	if (!x)
		return R(0, 1);
	bool negative = x < 0;
	if (negative)
		x = -x;
	R ret;
	{
		R hi(1, 1);
		R lo(0, 1);
		while (x > to_double(hi)){
			if (hi >= std::numeric_limits<T>::max() / 2)
				throw std::overflow_error("double out of bounds");
			lo = hi;
			hi *= 2;
		}

		auto delta = hi - lo;
		while (true){
			delta /= 2;
			auto c = lo + delta;
			if (c.numerator() >= limit || c.denominator() >= limit){
				ret = lo;
				break;
			}
			if (x < to_double(c))
				hi = c;
			else if (x > to_double(c))
				lo = c;
			else{
				ret = c;
				break;
			}
		}
	}
	return negative ? -ret : ret;
}

class DecoderSubstream;
class Module;

class Decoder{
	std::int64_t position = 0;
protected:
	struct StreamRange{
		rational_t time_begin;
		rational_t time_length;
		std::int64_t sample_begin;
		std::int64_t sample_length;
		int frequency;
	};
	std::vector<StreamRange> stream_ranges;
	std::int64_t time_resolution = -1;
	Module *module;

	virtual AudioBuffer *internal_read(const AudioFormat &, size_t extra_data, int substream_index) = 0;
	std::int64_t time_to_sample(const rational_t &time) const{
		auto beg = this->stream_ranges.begin();
		auto end = this->stream_ranges.end();
		auto it = std::find_if(beg, end, [&time](const StreamRange &sr){ return sr.time_begin + sr.time_length >= time; });
		if (it == end){
			it = beg + (end - beg - 1);
			if (!(it->time_begin <= time && time < it->time_begin + it->time_length))
				return -1;
		}
		auto conv = (time - it->time_begin) * it->frequency;
		return it->sample_begin + conv.numerator() / conv.denominator();
	}
	rational_t sample_to_time(std::int64_t sample) const{
		if (sample < 0)
			return -1;
		rational_t accum = 0;
		for (auto &sr : this->stream_ranges){
			if (sr.sample_begin + sr.sample_length >= sample)
				return accum + rational_t(sample - sr.sample_begin, sr.frequency);
			accum += sr.time_length;
		}
		return -1;
	}
	virtual std::int64_t seek_to_sample_internal(std::int64_t pos, bool fast){
		return -1;
	}
public:
	Decoder(Module *module = nullptr): module(module){}
	virtual ~Decoder(){}
	AudioBuffer *read(const AudioFormat &af, size_t extra_data, int substream_index){
		auto ret = this->internal_read(af, extra_data, substream_index);
		if (!ret)
			return ret;
		this->position += ret->sample_count;
		return ret;
	}
	std::int64_t seek_to_sample(std::int64_t pos, bool fast){
		if (pos == this->position)
			return pos;
		auto ret = this->seek_to_sample_internal(pos, fast);
		if (ret < 0)
			return -1;
		this->position = ret;
		return ret;
	}
	virtual rational_t seek_to_second(const rational_t &second, bool fast){
		auto sample = this->time_to_sample(second);
		if (sample < 0)
			return rational_t(-1, 1);
		auto ret = this->seek_to_sample(sample, fast);
		if (ret < 0)
			return rational_t(-1, 1);
		return this->sample_to_time(ret);
	}
	virtual std::int64_t sample_tell(){
		return this->position;
	}
	virtual int get_substream_count(){
		return 1;
	}
	virtual DecoderSubstream *get_substream(int index) = 0;
	Module *get_module() const{
		return this->module;
	}
};

class DecoderSubstream{
protected:
	Decoder &parent;
	int index;
	std::uint64_t first_sample = 0;
	rational_t first_moment = {0, 1};
	std::uint64_t position = 0;
	std::uint64_t length = std::numeric_limits<std::uint64_t>::max();
	rational_t seconds_length = {-1, 1};
	AudioFormat format;
public:
	DecoderSubstream(
		Decoder &parent,
		int index = 0,
		std::uint64_t first_sample = 0,
		std::uint64_t length = std::numeric_limits<std::uint64_t>::max(),
		const rational_t &first_moment = 0,
		const rational_t &seconds_length = {-1, 1}
	):
		parent(parent),
		index(index),
		first_sample(first_sample),
		first_moment(first_moment),
		length(length),
		seconds_length(seconds_length){}
	virtual ~DecoderSubstream() = 0;
	virtual RationalValue get_length_in_seconds(){
		return to_RationalValue(this->seconds_length);
	}
	virtual std::uint64_t get_length_in_samples(){
		return this->length;
	}
	virtual std::int64_t seek_to_sample(std::int64_t pos, bool fast){
		if (pos < 0 || (std::uint64_t)pos >= this->length)
			return -1;
		auto new_pos = this->parent.seek_to_sample(this->first_sample + pos, fast);
		if (new_pos < 0)
			return new_pos;
		this->position = (std::uint64_t)new_pos - this->first_sample;
		return this->position;
	}
	virtual RationalValue seek_to_second(const RationalValue &pos, bool fast){
		auto r = to_rational(pos);
		RationalValue ret{-1, 1};
		if (r < this->seconds_length){
			auto new_pos = this->parent.seek_to_second(this->first_moment + r, fast);
			if (new_pos >= 0)
				this->position = (std::uint64_t)this->parent.sample_tell() - this->first_sample;
			ret = to_RationalValue(new_pos - this->first_moment);
		}
		return ret;
	}
	virtual AudioBuffer *read(size_t extra_data){
		if (this->position >= this->length)
			return nullptr;
		auto sample = this->position + this->first_sample;
		if (this->parent.seek_to_sample(sample, false) != sample)
			return nullptr;
		auto ret = this->parent.read(this->format, extra_data, this->index);
		if (!ret)
			return nullptr;
		if ((std::uint64_t)ret->sample_count > this->length - this->position)
			ret->sample_count = (size_t)(this->length - this->position);
		this->position += ret->sample_count;
		return ret;
	}
	virtual AudioFormat get_audio_format(){
		return this->format;
	}
	virtual void set_number_format_hint(NumberFormat nf){}
	virtual Decoder &get_parent(){
		return this->parent;
	}
};

inline DecoderSubstream::~DecoderSubstream(){}
