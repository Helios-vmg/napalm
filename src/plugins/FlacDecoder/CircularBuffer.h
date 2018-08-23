#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>

template <size_t N>
class FixedCircularBuffer{
	std::uint8_t buffer[N];
	size_t buffer_head = 0,
		buffer_size = 0;
public:
	FixedCircularBuffer() = default;
	FixedCircularBuffer(const FixedCircularBuffer &) = delete;
	FixedCircularBuffer(FixedCircularBuffer &&) = delete;
	const FixedCircularBuffer &operator=(const FixedCircularBuffer &) = delete;
	const FixedCircularBuffer &operator=(FixedCircularBuffer &&) = delete;
	size_t write(const std::uint8_t *src, size_t src_size){
		size_t ret = 0;
		while (this->buffer_size < N && src_size){
			auto write_position = (this->buffer_head + this->buffer_size) % N;
			auto write_size = std::min(N - write_position, N - this->buffer_size);
			write_size = std::min(write_size, src_size);
			memcpy(this->buffer + write_position, src, write_size);
			this->buffer_size += write_size;
			src += write_size;
			src_size -= write_size;
			ret += write_size;
		}
		return ret;
	}
	size_t read(std::uint8_t *dst, size_t dst_size){
		size_t ret = 0;
		while (this->buffer_size && dst_size){
			auto read_size = std::min(N - this->buffer_head, this->buffer_size);
			read_size = std::min(read_size, dst_size);
			memcpy(dst, this->buffer + this->buffer_head, read_size);
			dst += read_size;
			dst_size -= read_size;
			this->buffer_head = (this->buffer_head + read_size) % N;
			this->buffer_size -= read_size;
			ret += read_size;
		}
		if (!this->buffer_size)
			this->buffer_head = 0;
		return ret;
	}
	size_t size() const{
		return this->buffer_size;
	}
	size_t capacity() const{
		return N;
	}
	void clear(){
		this->buffer_head = 0;
		this->buffer_size = 0;
	}
};

class VariableCircularBuffer{
	std::vector<std::uint8_t> buffer;
	size_t buffer_head = 0,
		buffer_size = 0;
	void copy(const VariableCircularBuffer &other, size_t n = 0);
public:
	VariableCircularBuffer(){
		this->buffer.resize(4096);
	}
	VariableCircularBuffer(const VariableCircularBuffer &other, size_t n = 0){
		this->copy(other, n);
	}
	VariableCircularBuffer(VariableCircularBuffer &&other){
		*this = std::move(other);
	}
	const VariableCircularBuffer &operator=(const VariableCircularBuffer &other){
		this->copy(other);
		return *this;
	}
	const VariableCircularBuffer &operator=(VariableCircularBuffer &&other);
	void write(const std::uint8_t *src, size_t src_size);
	size_t read(std::uint8_t *dst, size_t dst_size);
	template <size_t N>
	void copy_from(FixedCircularBuffer<N> &src){
		std::uint8_t temp[N];
		size_t n = src.read(temp, N);
		this->write(temp, n);
	}
	template <size_t N>
	size_t copy_to(FixedCircularBuffer<N> &dst){
		std::uint8_t temp[N];
		size_t read = N - dst.size();
		read = this->read(temp, read);
		dst.write(temp, read);
		return read;
	}
	size_t size() const{
		return this->buffer_size;
	}
	size_t capacity() const{
		return this->buffer.size();
	}
	void clear(){
		this->buffer_head = 0;
		this->buffer_size = 0;
	}
};
