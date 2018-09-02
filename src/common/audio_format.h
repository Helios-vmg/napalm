#pragma once

enum NumberFormat{
	Invalid = 0,
	IntU8,
	IntU16,
	IntU24,
	IntU32,
	IntS8,
	IntS16,
	IntS24,
	IntS32,
	Float32,
	Float64,
};

#ifdef __cplusplus
#include <cstddef>

inline size_t sizeof_NumberFormat(NumberFormat nf){
	switch (nf){
		case IntU8:
			return 1;
		case IntU16:
			return 2;
		case IntU24:
			return 3;
		case IntU32:
			return 4;
		case IntS8:
			return 1;
		case IntS16:
			return 2;
		case IntS24:
			return 3;
		case IntS32:
			return 4;
		case Float32:
			return 4;
		case Float64:
			return 8;
		default:
			return 0;
	}
}

#endif

typedef struct AudioFormat{
	NumberFormat format;
	int channels;
	int freq;
} AudioFormat;
