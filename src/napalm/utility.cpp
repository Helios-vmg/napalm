#include "utility.h"
#include <audio_format.h>
#include <utf8.cpp>
#include <ostream>

bool valid_format(const AudioFormat &af){
	return af.format != Invalid && af.channels >= 0 && af.freq;
}

bool operator==(const AudioFormat &a, const AudioFormat &b){
	return !memcmp(&a, &b, sizeof(a));
}

std::ostream &operator<<(std::ostream &stream, NumberFormat nf){
	static const char * const table[] = {
		"invalid",
		" 8-bit unsigned",
		"16-bit unsigned",
		"24-bit unsigned",
		"32-bit unsigned",
		" 8-bit   signed",
		"16-bit   signed",
		"24-bit   signed",
		"32-bit   signed",
		"32-bit    float",
		"64-bit    float",
	};
	if ((size_t)nf > array_length(table))
		return stream;
	return stream << table[nf];
}

std::ostream &operator<<(std::ostream &stream, const AudioFormat &af){
	return stream << af.format << ", " << af.channels << " channels, " << af.freq << " Hz";
}
