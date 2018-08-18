#pragma once

#include "module.h"
#include "external_io.h"
#include "audio_format.h"
#include <stdint.h>

#define DECODER_MODULE_TYPE "decoder"

typedef void *DecoderPtr;
typedef void *DecoderSubstreamPtr;

//Variable length struct.
typedef struct{
	void *opaque;
	void (*release_function)(void *);
	size_t sample_count;
	size_t samples_size;
	size_t extra_data_size;
	uint8_t data[1];
} AudioBuffer;

typedef struct{
	int64_t numerator;
	int64_t denominator;
} RationalValue;



//decoder_get_supported_extensions (required)
typedef const char **(*decoder_get_supported_extensions_f)(ModulePtr);

//decoder_open (required)
typedef DecoderPtr (*decoder_open_f)(ModulePtr, const char *, const ExternalIO *);

//decoder_close (required)
typedef void (*decoder_close_f)(DecoderPtr);

//decoder_get_substreams_count (optional)
typedef int (*decoder_get_substreams_count_f)(DecoderPtr);

//decoder_get_substream (required)
typedef DecoderSubstreamPtr (*decoder_get_substream_f)(DecoderPtr, int substream_index);

//substream_close (required)
typedef void (*substream_close_f)(DecoderSubstreamPtr);

//substream_get_audio_format (required)
typedef AudioFormat (*substream_get_audio_format_f)(DecoderSubstreamPtr);

//substream_set_number_format_hint (optional)
typedef void (*substream_set_number_format_hint_f)(DecoderSubstreamPtr, NumberFormat);

//substream_read (required)
typedef AudioBuffer *(*substream_read_f)(DecoderSubstreamPtr, size_t extra_data);

//substream_get_length_in_seconds (optional)
typedef RationalValue (*substream_get_length_in_seconds_f)(DecoderSubstreamPtr);

//substream_get_length_in_samples (optional)
typedef uint64_t (*substream_get_length_in_samples_f)(DecoderSubstreamPtr);

//substream_seek_to_sample (optional)
typedef int64_t (*substream_seek_to_sample_f)(DecoderSubstreamPtr, int64_t, int fast);

//substream_seek_to_second (optional)
typedef RationalValue (*substream_seek_to_second_f)(DecoderSubstreamPtr, RationalValue, int fast);
