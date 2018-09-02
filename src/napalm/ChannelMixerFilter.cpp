#include "Filter.h"
#include <stdexcept>
#include <sstream>

#define UTYPE(n1, n2, n3) template <> struct type_selector<n1, false>{ typedef std::uint##n2##_t type; typedef std::uint##n3##_t type2; }
#define STYPE(n1, n2, n3) template <> struct type_selector<n1, true>{  typedef std:: int##n2##_t type; typedef std:: int##n3##_t type2; }

template <int N, bool B>
struct type_selector{};

UTYPE(1, 8, 16);
UTYPE(2, 16, 32);
UTYPE(3, 32, 32);
UTYPE(4, 32, 64);

STYPE(1, 8, 16);
STYPE(2, 16, 32);
STYPE(3, 32, 32);
STYPE(4, 32, 64);

static void read_sample24(std::uint32_t &dst, const std::uint8_t *data){
	dst  = data[0];
	dst |= data[1] << 8;
	dst |= data[2] << 16;
}

static void read_sample24(std::int32_t &dst, const std::uint8_t *data){
	dst  = (std::int32_t)data[0];
	dst |= (std::int32_t)data[1] << 8;
	dst |= (std::int32_t)data[2] << 16;
}

template <typename T>
static void write_sample24(std::uint8_t *dst, T n){
	*(dst++) = n & 0xFF;
	n >>= 8;
	*(dst++) = n & 0xFF;
	n >>= 8;
	*dst = n & 0xFF;
}

template <int N, bool B>
struct channel_mixer{
	typedef typename type_selector<N, B>::type T;
	typedef typename type_selector<N, B>::type2 T2;
	static audio_buffer_t f(audio_buffer_t &&buffer, size_t bytes_per_sample){
		auto src = (T *)buffer->data;
		auto dst = (T *)buffer->data;
		auto n = buffer->sample_count;
		for (size_t i = 0; i < n; i++){
			*(dst++) = (T)(((T2)src[0] + (T2)src[1]) / 2);
			src += 2;
		}
		return std::move(buffer);
	}
};

template <bool B>
struct channel_mixer<3, B>{
	typedef typename type_selector<3, B>::type T;
	typedef typename type_selector<3, B>::type2 T2;
	static audio_buffer_t f(audio_buffer_t &&buffer, size_t bytes_per_sample){
		auto src = buffer->data;
		auto dst = buffer->data;
		auto n = buffer->sample_count;
		for (size_t i = 0; i < n; i++){
			T2 left, right;
			read_sample24(left, src);
			read_sample24(right, src + 3);
			write_sample24(dst, (left + right) / 2);
			dst += 3;
			src += 6;
		}
		return std::move(buffer);
	}
};

template <typename T>
struct channel_mixer_f{
	static audio_buffer_t f(audio_buffer_t &&buffer, size_t bytes_per_sample){
		auto src = (T *)buffer->data;
		auto dst = (T *)buffer->data;
		auto n = buffer->sample_count;
		for (size_t i = 0; i < n; i++){
			*(dst++) = (((T)src[0] + (T)src[1]) * (T)0.5);
			src += 2;
		}
		return std::move(buffer);
	}
};

template <int N, bool B>
struct channel_doubler{
	typedef typename type_selector<N, B>::type T;
	typedef typename type_selector<N, B>::type2 T2;
	static audio_buffer_t f(audio_buffer_t &&buffer, size_t bytes_per_sample){
		auto n = buffer->sample_count;
		audio_buffer_t ret = allocate_buffer_by_clone(buffer, n * bytes_per_sample);
		ret->sample_count = n;
		auto src = (T *)buffer->data;
		auto dst = (T *)ret->data;
		for (size_t i = 0; i < n; i++){
			dst[1] = dst[0] = *(src++);
			dst += 2;
		}
		return std::move(ret);
	}
};

template <bool B>
struct channel_doubler<3, B>{
	typedef typename type_selector<3, B>::type T;
	typedef typename type_selector<3, B>::type2 T2;
	static audio_buffer_t f(audio_buffer_t &&buffer, size_t bytes_per_sample){
		auto n = buffer->sample_count;
		audio_buffer_t ret = allocate_buffer_by_clone(buffer, n * bytes_per_sample);
		ret->sample_count = n;
		auto src = buffer->data;
		auto dst = ret->data;
		for (size_t i = 0; i < n; i++){
			T2 center;
			read_sample24(center, src);
			write_sample24(dst, center);
			write_sample24(dst + 3, center);
			dst += 6;
			src += 3;
		}
		return std::move(ret);
	}
};

template <typename T>
struct channel_doubler_f{
	static audio_buffer_t f(audio_buffer_t &&buffer, size_t bytes_per_sample){
		auto n = buffer->sample_count;
		audio_buffer_t ret = allocate_buffer_by_clone(buffer, n * bytes_per_sample);
		ret->sample_count = n;
		auto src = (T *)buffer->data;
		auto dst = (T *)ret->data;
		for (size_t i = 0; i < n; i++){
			dst[1] = dst[0] = *(src++);
			dst += 2;
		}
		return std::move(ret);
	}
};

#define UCASE1(n) case IntU##n: this->converter = channel_mixer<n / 8, false>::f; break
#define SCASE1(n) case IntS##n: this->converter = channel_mixer<n / 8, true >::f; break
#define UCASE2(n) case IntU##n: this->converter = channel_doubler<n / 8, false>::f; break
#define SCASE2(n) case IntS##n: this->converter = channel_doubler<n / 8, true >::f; break

ChannelMixerFilter::ChannelMixerFilter(const AudioFormat &af, int target_channels):
		TrueFilter(af),
		channels(target_channels){

	if (af.channels != 1 && af.channels != 2){
		std::stringstream stream;
		stream << "Unsupposed channels: " << af.channels;
		throw std::runtime_error(stream.str());
	}
	if (target_channels != 1 && target_channels != 2){
		std::stringstream stream;
		stream << "Unsupported output channels: " << target_channels;
		throw std::runtime_error(stream.str());
	}

	this->bytes_per_dest_sample = this->channels * sizeof_NumberFormat(this->source_format.format);

	if (this->channels == 1){
		switch (this->source_format.format){
			UCASE1(8);
			UCASE1(16);
			UCASE1(24);
			UCASE1(32);
			SCASE1(8);
			SCASE1(16);
			SCASE1(24);
			SCASE1(32);
			case Float32:
				this->converter = channel_mixer_f<float>::f;
				break;
			case Float64:
				this->converter = channel_mixer_f<double>::f;
				break;
			default:
				throw std::exception();
		}
	}else{
		switch (this->source_format.format){
			UCASE2(8);
			UCASE2(16);
			UCASE2(24);
			UCASE2(32);
			SCASE2(8);
			SCASE2(16);
			SCASE2(24);
			SCASE2(32);
			case Float32:
				this->converter = channel_doubler_f<float>::f;
				break;
			case Float64:
				this->converter = channel_doubler_f<double>::f;
				break;
			default:
				throw std::exception();
		}
	}
}

audio_buffer_t ChannelMixerFilter::apply(audio_buffer_t &&buffer){
	return this->converter(std::move(buffer), this->bytes_per_dest_sample);
}
