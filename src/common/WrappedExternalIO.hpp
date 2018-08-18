#pragma once

#include "external_io.h"

class WrappedExternalIO{
	ExternalIO io;
public:
	WrappedExternalIO(const ExternalIO &io): io(io){}
	size_t read(void *dst, size_t dst_size){
		if (!this->io.read)
			return 0;
		return this->io.read(this->io.user_data, dst, dst_size);
	}
	size_t write(const void *src, size_t src_size){
		if (!this->io.write)
			return 0;
		return this->io.write(this->io.user_data, src, src_size);
	}
	int64_t seek(int64_t offset, int whence){
		if (!this->io.seek)
			return -1;
		return this->io.seek(this->io.user_data, offset, whence);
	}
	int64_t tell(){
		if (!this->io.tell)
			return -1;
		return this->io.tell(this->io.user_data);
	}
};
