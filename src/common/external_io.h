#pragma once

#include <stdint.h>

//Any of the function pointers may be null, in which case the stream doesn't
//support that operation. Any stream at least supports reading or writing.
typedef struct{
	void *user_data;
	//Return value is the number of bytes written to dst.
	//EOF is signalled by returning 0.
	size_t (*read)(void *user_data, void *dst, size_t dst_size);
	//Return value is the number of bytes read from src.
	//Returning a zero means there's some kind of error in the
	//destination (for example, out of disk space).
	size_t (*write)(void *user_data, const void *src, size_t src_size);
	//Return value is new offset from the beginning. Error is signalled by
	//returning a negative value.
	int64_t (*seek)(void *user_data, int64_t offset, int whence);
	//Return value is current position from the beginning.
	int64_t (*tell)(void *user_data);
} ExternalIO;
