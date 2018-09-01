#pragma once

#include <boost/rational.hpp>
#include <string>
#include <cstdint>
#include <memory>
#include <cstring>
#include <utf8.hpp>
#include <RationalValue.h>

enum NumberFormat;
typedef boost::rational<std::int64_t> rational_t;

template <typename T>
using smart_c_struct = std::unique_ptr<T, void(*)(T *)>;

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

template <typename T, size_t N>
struct basic_uniqueid_t{
	T data[N / sizeof(T)];
	basic_uniqueid_t(){
		static_assert(N % sizeof(T) == 0, "You can't use this T for that N.");
		std::fill(this->data, this->data + array_length(this->data), 0);
	}
	basic_uniqueid_t(const std::uint8_t src[N]){
		static_assert(N % sizeof(T) == 0, "You can't use this T for that N.");
		memcpy(this->data, src, N);
	}
	bool operator==(const basic_uniqueid_t<T, N> &other) const{
		static_assert(N % sizeof(T) == 0, "You can't use this T for that N.");
		for (size_t i = 0; i != N / sizeof(T); i++)
			if (this->data[i] != other.data[i])
				return false;
		return true;
	}
	bool operator<(const basic_uniqueid_t<T, N> &other) const{
		static_assert(N % sizeof(T) == 0 && std::is_unsigned<T>::value, "You can't use this T for that N.");
		for (size_t i = 0; i != N / sizeof(T); i++){
			auto a = this->data[i];
			auto b = other.data[i];
			if (a < b)
				return true;
			if (a > b)
				return false;
		}
		return false;
	}
	bool operator!=(const basic_uniqueid_t<T, N> &other) const{
		return !(*this == other);
	}
	bool operator>=(const basic_uniqueid_t<T, N> &other) const{
		return !(*this < other);
	}
	bool operator>(const basic_uniqueid_t<T, N> &other) const{
		return other < *this;
	}
	bool operator<=(const basic_uniqueid_t<T, N> &other) const{
		return !(other < *this);
	}
	bool operator!() const{
		for (auto i : this->data)
			if (i)
				return false;
		return true;
	}
};

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

struct AudioFormat;
typedef struct AudioFormat AudioFormat;
bool valid_format(const AudioFormat &);

bool operator==(const AudioFormat &a, const AudioFormat &b);
inline bool operator!=(const AudioFormat &a, const AudioFormat &b){
	return !(a == b);
}

std::ostream &operator<<(std::ostream &stream, NumberFormat nf);
std::ostream &operator<<(std::ostream &, const AudioFormat &);
