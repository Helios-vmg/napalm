#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

class GenericMetadata{
	std::string path;
public:
	GenericMetadata(const std::string &path): path(path){}
	virtual ~GenericMetadata(){}
	virtual std::string album() = 0;
	virtual std::string track_title() = 0;
	virtual int track_number_int();
	virtual std::string track_number() = 0;
	virtual std::string track_artist() = 0;
	virtual std::string date() = 0;
	virtual double track_gain() = 0;
	virtual double track_peak() = 0;
	virtual double album_gain() = 0;
	virtual double album_peak() = 0;
	std::string track_id();
	virtual std::unique_ptr<std::vector<std::uint8_t>> picture() = 0;
	const std::string &get_path() const{
		return this->path;
	}
	virtual std::map<std::string, std::string> get_full_metadata() = 0;
};
