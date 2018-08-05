#pragma once

#include <boost/rational.hpp>
#include <string>
#include <cstdint>

typedef boost::rational<std::int64_t> rational_t;

template <typename T>
void utf8_to_string(std::basic_string<T> &dst, const std::uint8_t *buffer, size_t n){
	static const std::uint8_t utf8_lengths[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 0, 0,
		0xFF, 0x1F, 0x0F, 0x07, 0x03, 0x01
	};
	static const std::uint8_t bom[] = { 0xEF, 0xBB, 0xBF };
	static const std::uint8_t *masks = utf8_lengths + 0x100;
	if (n >= 3 && !memcmp(buffer, bom, 3)){
		buffer += 3;
		n -= 3;
	}
	dst.resize(n);
	T *temp_pointer = &dst[0];
	size_t writer = 0;
	for (size_t i = 0; i != n;){
		std::uint8_t byte = buffer[i++];
		std::uint8_t length = utf8_lengths[byte];
		if (length > n - i)
			break;
		unsigned wc = byte & masks[length];
		for (;length; length--){
			wc <<= 6;
			wc |= (T)(buffer[i++] & 0x3F);
		}
		temp_pointer[writer++] = wc;
	}
	dst.resize(writer);
}

inline unsigned utf8_bytes(unsigned c){
	static const unsigned masks[] = {
		0x0000007F,
		0x000007FF,
		0x0000FFFF,
		0x001FFFFF,
		0x03FFFFFF,
	};
	for (unsigned i = 0; i != sizeof(masks) / sizeof(*masks); i++)
		if ((c & masks[i]) == c)
			return i + 1;
	return 6;
}

template <typename T>
size_t utf8_size(const std::basic_string<T> &s){
	size_t ret = 0;
	for (auto c : s)
		ret += utf8_bytes(c);
	return ret;
}

template <typename T>
void string_to_utf8(std::string &dst, const std::basic_string<T> &src){
	static const unsigned char masks[] = { 0, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
	dst.resize(utf8_size(src));
	auto pointer = (unsigned char *)&dst[0];
	const T *src_pointer = &src[0];
	for (size_t i = 0, n = src.size(); i != n; i++){
		unsigned c = src_pointer[i];
		unsigned size = utf8_bytes(c) - 1;

		if (!size){
			*(pointer++) = (unsigned char)c;
			continue;
		}

		unsigned char temp[10];
		unsigned temp_size = 0;

		do{
			temp[temp_size++] = c & 0x3F | 0x80;
			c >>= 6;
		}while (temp_size != size);
		*(pointer++) = c | masks[temp_size];
		while (temp_size)
			*(pointer++) = temp[--temp_size];
	}
}

template <typename T, typename F>
bool glob_implementation(const T *pattern, const T *string, F f, bool ignore_f){
glob_start:
	if (!*pattern && !*string)
		return 1;
	switch (*pattern){
		case '*':
			while (1){
				if (glob(pattern + 1, string, f))
					return 1;
				if (!*string)
					return 0;
				string++;
			}
		case '?':
			pattern++;
			string++;
			goto glob_start;
		default:
			if (ignore_f && *pattern == *string || !ignore_f && f(*pattern) == f(*string)){
				pattern++;
				string++;
				goto glob_start;
			}
			return 0;
	}
}

template <typename T, typename F>
bool glob(const T *pattern, const T *string){
	return glob_implementation(pattern, string, nullptr, true);
}

template <typename T, typename F>
bool glob(const T *pattern, const T *string, F f){
	return glob_implementation(pattern, string, f, false);
}

template <typename T, size_t N>
constexpr size_t array_length(const T (&)[N]){
	return N;
}

std::wstring utf8_to_string(const std::string &src);
std::string string_to_utf8(const std::wstring &src);
