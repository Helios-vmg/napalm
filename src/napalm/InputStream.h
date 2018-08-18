#pragma once

#include "../common/external_io.h"
#include <cstdint>
#include <fstream>
#ifdef _WIN32
#include <boost/filesystem/fstream.hpp>
#endif

class InputStream{
protected:
	std::string path;
public:
	InputStream(const char *path): path(path){}
	virtual ~InputStream(){}
	virtual size_t read(void *dst, size_t dst_size) = 0;
	virtual std::int64_t seek(std::int64_t offset, int whence) = 0;
	virtual std::int64_t tell() = 0;
	virtual operator ExternalIO() = 0;
	const std::string &get_path() const{
		return this->path;
	}
};

class StdInputStream : public InputStream{
#ifdef _WIN32
	typedef boost::filesystem::ifstream file_t;
#else
	typedef std::ifstream file_t;
#endif
	file_t file;

	static size_t static_read(void *user_data, void *dst, size_t dst_size);
	static std::int64_t static_seek(void *user_data, std::int64_t offset, int whence);
	static std::int64_t static_tell(void *user_data);
public:
	StdInputStream(const char *path);
	size_t read(void *dst, size_t dst_size) override;
	std::int64_t seek(std::int64_t offset, int whence) override;
	std::int64_t tell() override;
	operator ExternalIO() override;
};