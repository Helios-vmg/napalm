#pragma once

#include "module.h"
#include "audio_format.h"
#include "decoder_module.h"
#include <stdint.h>

#define OUTPUT_MODULE_TYPE "output"

typedef void *OutputPtr;
typedef void *OutputDevicePtr;

typedef struct{
	uint8_t unique_id[32];
} UniqueID;

typedef struct{
	const char *name;
	UniqueID unique_id;
} OutputDeviceListItem;

typedef struct{
	void *opaque;
	void (*release_function)(void *);
	size_t length;
	OutputDeviceListItem *items;
} OutputDeviceList;

/*
 * AudioCallback:
 * The OutputDevice should call this callback when more data is required by
 * the device.
 *
 * cb_data: The pointer that was passed to output_open_device().
 *
 * dst: Pointer to the audio buffer. The buffer must be in the format
 * specified by output_device_get_preferred_format().
 *
 * size: The length in samples of the buffer pointed to by dst.
 *
 * samples_queued: The depth of the audio queue in samples. For example, in a
 * double buffering situation where both buffers are the same length and the
 * callback is called on the back buffer as soon as playback begins on the front
 * buffer, this parameter should be set to the length of the front buffer. If
 * instead the device uses a queue of variable depth (e.g. because of
 * unpredictable latency), the caller should pass the count of all samples
 * contained in the queue.
 *
 * Return value: The number of samples written to dst. Resouce starvation may
 * cause this value to be less than the size parameter. The OutputDevice should
 * handle this appropriately.
 *
 * Note: A "sample" refers to a complete instant in an audio stream, including
 * all channels, so for example a single sample from a 7 channel float stream
 * contains 7 floats, not 1 float.
 */
typedef size_t (*AudioCallback)(void *cb_data, void *dst, size_t size, size_t samples_queued);

typedef void (*AudioFormatChangedCallback)(void *cb_data, const AudioFormat *af);

typedef void (*AudioVolumeChangedCallback)(void *cb_data, RationalValue);

typedef struct{
	void *cb_data;
	AudioCallback audio_callback;
	AudioFormatChangedCallback format_changed;
	AudioVolumeChangedCallback volume_changed;
} AudioCallbackData;


//output_get_device_list (required)
typedef OutputDeviceList *(*output_get_device_list_f)(ModulePtr);

//output_get_supported_formats (required)
typedef AudioFormat *(*output_get_supported_formats_f)(ModulePtr, const UniqueID *unique_id);

//output_open_device (required)
typedef OutputDevicePtr (*output_open_device_f)(ModulePtr, const UniqueID *unique_id, size_t format_index, const AudioCallbackData *callback);

//output_device_close (required)
typedef void (*output_device_close_f)(OutputDevicePtr);

//output_device_flush (optional)
//Not necessary if the device doesn't have its own queue. When called, the
//device should flush its queue, discarding the data in it.
typedef void (*output_device_flush_f)(OutputDevicePtr);

//output_device_pause (optional)
//Not necessary if the device doesn't have its own queue. When called with a
//nonzero pause_active value, the device should momentarily stop processing
//its queue. While in the paused state, the device should not call its
//AudioCallback, as no data may be produced in this state. The device may exit
//its paused state when this function is called with a zero pause_active value.
//If the module doesn't implement this function, the device may continue
//calling its AudioCallback, but it must handle the callee sometimes writing no
//data to the buffer.
typedef void (*output_device_pause_f)(OutputDevicePtr, int pause_active);

//output_device_set_volume (optional)
typedef void (*output_device_set_volume_f)(OutputDevicePtr, RationalValue volume);
