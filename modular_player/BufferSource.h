#pragma once

#include "../common/decoder_module.h"
#include <memory>

typedef std::unique_ptr<ReadResult, decoder_release_ReadResult_f> read_buffer_t;

class BufferSource{
public:
	virtual ~BufferSource(){}
	virtual read_buffer_t read() = 0;
};
