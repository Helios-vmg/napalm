#pragma once

#include "decoder_module.h"
#include <stdint.h>

#define METADATA_MODULE_TYPE "metadata"

typedef void *MetadataPtr;
typedef void *MetadataIteratorPtr;

typedef struct{
	uint8_t *ptr;
	size_t size;
} MetadataBuffer;

typedef struct{
	MetadataBuffer key, value;
} MetadataKeyValue;

#define DECLARE_METADATA_STRING_FUNCTION(x) typedef MetadataBuffer (*metadata_get_##x##_f)(MetadataPtr)
#define DECLARE_METADATA_DOUBLE_FUNCTION(x) typedef double (*metadata_get_##x##_f)(MetadataPtr)
#define DECLARE_METADATA_BUFFER_FUNCTION(x) typedef MetadataBuffer (*metadata_get_##x##_f)(MetadataPtr)

//decoder_substream_get_metadata (required)
typedef MetadataPtr (*decoder_substream_get_metadata_f)(DecoderSubstreamPtr);

//metadata_destroy (required)
typedef void (*metadata_destroy_f)(MetadataPtr);

//metadata_release_buffer (required)
typedef void (*metadata_release_buffer_f)(MetadataBuffer);

//All required:
DECLARE_METADATA_STRING_FUNCTION(album);
DECLARE_METADATA_STRING_FUNCTION(track_title);
DECLARE_METADATA_STRING_FUNCTION(track_number);
DECLARE_METADATA_STRING_FUNCTION(track_artist);
DECLARE_METADATA_STRING_FUNCTION(date);
DECLARE_METADATA_DOUBLE_FUNCTION(track_gain);
DECLARE_METADATA_DOUBLE_FUNCTION(track_peak);
DECLARE_METADATA_DOUBLE_FUNCTION(album_gain);
DECLARE_METADATA_DOUBLE_FUNCTION(album_peak);
DECLARE_METADATA_BUFFER_FUNCTION(front_cover);

//metadata_get_track_number_int (optional)
typedef int (*metadata_get_track_number_int_f)(MetadataPtr);

//metadata_get_iterator (optional)
typedef MetadataIteratorPtr (*metadata_get_iterator_f)(MetadataPtr);

//metadata_iterator_release (required if metadata_get_iterator is implemented)
typedef void (*metadata_iterator_release_f)(MetadataIteratorPtr);

//metadata_iterator_get_next (required if metadata_get_iterator is implemented)
//Must return >0 when data has been written to kv.
//Must return 0 if there's no more data in the iterator.
//Must return <0 if there was an error.
typedef int (*metadata_iterator_get_next_f)(MetadataIteratorPtr, MetadataKeyValue *kv);
