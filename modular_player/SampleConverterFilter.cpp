#include "Filter.h"

namespace{

template <NumberFormat NF>
struct loader{
	typedef std::uint32_t type;
};

template <>
struct loader<IntU8>{
	typedef std::uint32_t type;
	static type f(const std::uint8_t *&src){
		return *(src++);
	}
};

template <>
struct loader<IntS8>{
	typedef std::uint32_t type;
	static type f(const std::uint8_t *&src){
		return loader<IntU8>::f(src);
	}
};

template <>
struct loader<IntU16>{
	typedef std::uint32_t type;
	static type f(const std::uint8_t *&src){
		type ret = src[0] | (src[1] << 8);
		src += 2;
		return ret;
	}
};

template <>
struct loader<IntS16>{
	typedef std::uint32_t type;
	static type f(const std::uint8_t *&src){
		return loader<IntU16>::f(src);
	}
};

template <>
struct loader<IntU24>{
	typedef std::uint32_t type;
	static type f(const std::uint8_t *&src){
		type ret = src[0] | (src[1] << 8) | (src[2] << 16);
		src += 3;
		return ret;
	}
};

template <>
struct loader<IntS24>{
	typedef std::uint32_t type;
	static type f(const std::uint8_t *&src){
		return loader<IntU24>::f(src);
	}
};

template <>
struct loader<IntU32>{
	typedef std::uint32_t type;
	static type f(const std::uint8_t *&src){
		type ret = src[0] | (src[1] << 8) | (src[2] << 16) | (src[2] << 24);
		src += 4;
		return ret;
	}
};

template <>
struct loader<IntS32>{
	typedef std::uint32_t type;
	static type f(const std::uint8_t *&src){
		return loader<IntU32>::f(src);
	}
};

template <>
struct loader<Float32>{
	typedef float type;
	static type f(const std::uint8_t *&src){
		type ret = *(type *)src;
		src += sizeof(type);
		return ret;
	}
};

template <>
struct loader<Float64>{
	typedef double type;
	static type f(const std::uint8_t *&src){
		type ret = *(type *)src;
		src += sizeof(type);
		return ret;
	}
};

template <NumberFormat Src, NumberFormat Dst>
struct converter{};

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
saturate(T x){
	if (x < -1)
		return -1;
	if (x > 1)
		return 1;
	return x;
}

#include "converters.inl"

template <NumberFormat NF>
struct writer{
	typedef std::uint32_t type;
};

template <>
struct writer<IntU8>{
	typedef std::uint32_t type;
	static void f(std::uint8_t *&dst, type src){
		*(dst++) = src;
	}
};

template <>
struct writer<IntS8>{
	typedef std::uint32_t type;
	static void f(std::uint8_t *&dst, type src){
		writer<IntU8>::f(dst, src);
	}
};

template <>
struct writer<IntU16>{
	typedef std::uint32_t type;
	static void f(std::uint8_t *&dst, type src){
		dst[0] = src & 0xFF;
		dst[1] = (src >> 8) & 0xFF;
		dst += 2;
	}
};

template <>
struct writer<IntS16>{
	typedef std::uint32_t type;
	static void f(std::uint8_t *&dst, type src){
		writer<IntU16>::f(dst, src);
	}
};

template <>
struct writer<IntU24>{
	typedef std::uint32_t type;
	static void f(std::uint8_t *&dst, type src){
		dst[0] = src & 0xFF;
		dst[1] = (src >> 8) & 0xFF;
		dst[2] = (src >> 16) & 0xFF;
		dst += 3;
	}
};

template <>
struct writer<IntS24>{
	typedef std::uint32_t type;
	static void f(std::uint8_t *&dst, type src){
		writer<IntU24>::f(dst, src);
	}
};

template <>
struct writer<IntU32>{
	typedef std::uint32_t type;
	static void f(std::uint8_t *&dst, type src){
		dst[0] = src & 0xFF;
		dst[1] = (src >> 8) & 0xFF;
		dst[2] = (src >> 16) & 0xFF;
		dst[3] = (src >> 24) & 0xFF;
		dst += 4;
	}
};

template <>
struct writer<IntS32>{
	typedef std::uint32_t type;
	static void f(std::uint8_t *&dst, type src){
		writer<IntU32>::f(dst, src);
	}
};

template <>
struct writer<Float32>{
	typedef float type;
	static void f(std::uint8_t *&dst, type src){
		*(type *)dst = src;
		dst += sizeof(type);
	}
};

template <>
struct writer<Float64>{
	typedef double type;
	static void f(std::uint8_t *&dst, type src){
		*(type *)dst = src;
		dst += sizeof(type);
	}
};

template <NumberFormat Src, NumberFormat Dst>
struct sample_converter{
	static read_buffer_t f(read_buffer_t &&buffer, size_t bytes_per_sample2, size_t channels){
		read_buffer_t ret(nullptr, release_buffer);
		std::uint8_t *dst;

		const std::uint8_t *src = buffer->data;
		bool b = sizeof_NumberFormat(Src) < sizeof_NumberFormat(Dst);

		if (b){
			ret = allocate_buffer(buffer->sample_count * bytes_per_sample2);
			ret->sample_count = buffer->sample_count;
			dst = ret->data;
		} else
			dst = buffer->data;

		auto elements = buffer->sample_count * channels;
		for (size_t i = 0; i < elements; i++){
			auto x = loader<Src>::f(src);
			auto y = converter<Src, Dst>::f(x);
			writer<Dst>::f(dst, y);
		}
		return b ? std::move(ret) : std::move(buffer);
	}
};

typedef read_buffer_t(*converter_f)(read_buffer_t &&, size_t, size_t);

template <NumberFormat Src, NumberFormat Dst>
struct loop{
	static converter_f f(NumberFormat src, NumberFormat dst){
		if (Dst == Invalid)
			return nullptr;
		if (src == Src){
			if (dst == Dst)
				return sample_converter<Src, Dst>::f;
			return loop<Src, (NumberFormat)(Dst - 1)>::f(src, dst);
		}
		return loop<(NumberFormat)(Src - 1), Dst>::f(src, dst);
	}
};

template <>
struct loop<Invalid, Invalid>{
	static converter_f f(NumberFormat src, NumberFormat dst){
		return nullptr;
	}
};

template <NumberFormat Dst>
struct loop<Invalid, Dst>{
	static converter_f f(NumberFormat src, NumberFormat dst){
		return nullptr;
	}
};

template <NumberFormat Src>
struct loop<Src, Invalid>{
	static converter_f f(NumberFormat src, NumberFormat dst){
		return nullptr;
	}
};

}

#define C(x, y) ((int)x | ((int)y << 8))

SampleConverterFilter::SampleConverterFilter(const AudioFormat &af, NumberFormat type): TrueFilter(af), type(type){
	if (type == this->source_format.format)
		throw std::runtime_error("Source and destination formations must be different!");
	this->bytes_per_dest_sample = this->source_format.channels * sizeof_NumberFormat(type);
	this->converter = loop<Float64, Float64>::f(this->source_format.format, type);
	if (!this->converter)
		throw std::runtime_error("Unsupported format conversion.");
}

read_buffer_t SampleConverterFilter::apply(read_buffer_t &&buffer){
	return this->converter(std::move(buffer), this->bytes_per_dest_sample, this->source_format.channels);
}
