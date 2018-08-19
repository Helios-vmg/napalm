#pragma once

#include "../common/decoder_module.h"
#include "utility.h"

typedef smart_c_struct<AudioBuffer> audio_buffer_t;

class DecoderSubstream;

class BufferSource{
public:
	virtual ~BufferSource(){}
	virtual audio_buffer_t read() = 0;
	virtual std::unique_ptr<BufferSource> unroll_chain() = 0;
	virtual DecoderSubstream &get_first_source() = 0;
	virtual void flush(){}
};
