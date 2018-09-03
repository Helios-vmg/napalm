#include "CircularBuffer.h"
#include <algorithm>
#include <cstring>

void VariableCircularBuffer::copy(const VariableCircularBuffer &other, size_t n){
	if (!n)
		n = other.buffer.size();
	this->buffer.resize(n);
	this->buffer_head = 0;
	this->buffer_size = other.buffer_size;
	if (n){
		if (this->buffer_head + this->buffer_size <= n)
			memcpy(&this->buffer[0], &other.buffer[other.buffer_head], this->buffer_size);
		else{
			auto m = n - other.buffer_head;
			memcpy(&this->buffer[0], &other.buffer[other.buffer_head], m);
			memcpy(&this->buffer[m], &other.buffer[0], other.buffer_size - m);
		}
	}
}

const VariableCircularBuffer &VariableCircularBuffer::operator=(VariableCircularBuffer &&other){
	this->buffer = std::move(other.buffer);
	this->buffer_head = other.buffer_head;
	this->buffer_size = other.buffer_size;
	other.buffer_head = 0;
	other.buffer_size = 0;
	return *this;
}

void VariableCircularBuffer::write(const std::uint8_t *src, size_t src_size){
	if (!src_size)
		return;
	if (!this->buffer_head){
		if (this->buffer.size() < src_size + this->buffer_size)
			this->buffer.resize(src_size + this->buffer_size);
		memcpy(&this->buffer[0], src, src_size);
		this->buffer_size += src_size;
		return;
	}
	while (src_size){
		auto n = this->buffer.size();
		if (this->buffer_size == n){
			*this = VariableCircularBuffer(*this, n + src_size);
			continue;
		}

		auto write_position = (this->buffer_head + this->buffer_size) % n;
		auto write_size = std::min(n - write_position, n - this->buffer_size);
		write_size = std::min(write_size, src_size);
		memcpy(&this->buffer[this->buffer_head], src, write_size);
		this->buffer_size += write_size;
		src += write_size;
		src_size -= write_size;
	}
}

size_t VariableCircularBuffer::read(std::uint8_t *dst, size_t dst_size){
	auto n = this->buffer.size();
	size_t ret = 0;
	while (this->buffer_size && dst_size){
		auto read_size = std::min(n - this->buffer_head, this->buffer_size);
		read_size = std::min(read_size, dst_size);
		memcpy(dst, &this->buffer[this->buffer_head], read_size);
		dst += read_size;
		dst_size -= read_size;
		this->buffer_head = (this->buffer_head + read_size) % n;
		this->buffer_size -= read_size;
		ret += read_size;
	}
	if (!this->buffer_size)
		this->buffer_head = 0;
	return ret;
}
