#pragma once

#include "external_io.h"
#include <vector>
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <cstdio>

class WrappedExternalIO{
	ExternalIO io;
public:
	WrappedExternalIO(const ExternalIO &io): io(io){}
	bool can_read() const{
		return !!this->io.read;
	}
	bool can_write() const{
		return !!this->io.write;
	}
	bool can_seek() const{
		return !!this->io.seek && !!this->io.tell;
	}
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
	std::int64_t seek(std::int64_t offset, int whence){
		if (!this->io.seek)
			return -1;
		return this->io.seek(this->io.user_data, offset, whence);
	}
	std::int64_t tell(){
		if (!this->io.tell)
			return -1;
		return this->io.tell(this->io.user_data);
	}
	const ExternalIO &get_io() const{
		return this->io;
	}
};

class SlicedIO{
	ExternalIO io;
	std::int64_t beginning = 0,
		length = std::numeric_limits<std::int64_t>::max(),
		position = 0;
public:
	SlicedIO(const ExternalIO &io): io(io){
		if (!this->io.read)
			throw std::runtime_error("Invalid initialization of SlicedIO.");
		if (this->can_seek()){
			auto current = this->io.tell(this->io.user_data);
			this->length = this->io.seek(this->io.user_data, 0, SEEK_END);
			this->io.seek(this->io.user_data, current, SEEK_SET);
		}
	}
	SlicedIO(const WrappedExternalIO &io): SlicedIO(io.get_io()){}
	SlicedIO(const ExternalIO &io, std::int64_t beginning, std::int64_t length):
			io(io),
			beginning(beginning),
			length(length){
		if (this->io.read){
			if (this->can_seek()){
				auto old = this->io.tell(this->io.user_data);
				auto size = this->io.seek(this->io.user_data, 0, SEEK_END);
				this->io.seek(this->io.user_data, old, SEEK_SET);
				if (size >= this->beginning + this->length)
					return;
			}
		}
		throw std::runtime_error("Invalid initialization of SlicedIO.");
	}
	bool can_read() const{
		return !!this->io.read;
	}
	bool can_write() const{
		return false;
	}
	bool can_seek() const{
		return !!this->io.seek && !!this->io.tell;
	}
	size_t read(void *dst, size_t dst_size){
		if (!this->io.read)
			return 0;
		auto read_size = std::min<size_t>(dst_size, this->length - this->position);
		auto ret = this->io.read(this->io.user_data, dst, read_size);
		this->position += ret;
		return ret;
	}
	size_t write(const void *src, size_t src_size){
		return 0;
	}
	std::int64_t seek(std::int64_t offset, int whence){
		if (!this->io.seek)
			return -1;
		while (true){
			switch (whence){
				case SEEK_SET:
					if (offset < 0 || offset > this->length)
						return -1;
					this->position = this->io.seek(this->io.user_data, offset + this->beginning, whence) - this->beginning;
					return this->position;
				case SEEK_CUR:
					offset += this->tell();
					whence = SEEK_SET;
					continue;
				case SEEK_END:
					offset += this->length;
					whence = SEEK_SET;
					continue;
			}
			throw std::exception();
		}
	}
	std::int64_t tell(){
		if (!this->io.tell)
			return -1;
		return this->position = this->io.tell(this->io.user_data) - this->beginning;
	}
};
