#include "InputStream.h"
#include <stdexcept>
#include <sstream>
#include <utf8.hpp>

static void bad_whence(int whence){
	std::stringstream stream;
	stream << "Incorrect usage of seek. Invalid whence value: " << whence;
	throw std::runtime_error(stream.str());
}

StdInputStream::StdInputStream(const char *path): InputStream(path){
#ifdef _WIN32
	this->file.open(utf8_to_string(this->path), std::ios::binary);
#else
	this->file.open(this->path, std::ios::binary);
#endif
	if (!this->file)
		throw std::runtime_error("File not found: " + this->path);
}

size_t StdInputStream::read(void *dst, size_t dst_size){
	this->file.read((char *)dst, dst_size);
	if (!this->file)
		this->file.clear();
	size_t ret = this->file.gcount();
	return ret;
}

std::int64_t StdInputStream::seek(std::int64_t offset, int whence){
	switch (whence){
		case SEEK_SET:
			this->file.seekg(offset, std::ios::beg);
			break;
		case SEEK_CUR:
			this->file.seekg(offset, std::ios::cur);
			break;
		case SEEK_END:
			this->file.seekg(offset, std::ios::end);
			break;
		default:
			bad_whence(whence);
	}
	if (!this->file)
		this->file.clear();
	std::int64_t ret = this->file.tellg();
	return ret;
}

std::int64_t StdInputStream::tell(){
	return this->file.tellg();
}

StdInputStream::operator ExternalIO(){
	ExternalIO ret;
	ret.user_data = this;
	ret.write = nullptr;
	ret.read = static_read;
	ret.seek = static_seek;
	ret.tell = static_tell;
	return ret;
}

size_t StdInputStream::static_read(void *user_data, void *dst, size_t dst_size){
	return ((StdInputStream *)user_data)->StdInputStream::read(dst, dst_size);
}

std::int64_t StdInputStream::static_seek(void *user_data, std::int64_t offset, int whence){
	return ((StdInputStream *)user_data)->StdInputStream::seek(offset, whence);
}

std::int64_t StdInputStream::static_tell(void *user_data){
	return ((StdInputStream *)user_data)->StdInputStream::tell();
}
