#pragma once

#include <string>
#include <type_traits>
#include <sstream>

class GenericMetadata{
protected:
	std::string path;
public:
	GenericMetadata(const std::string &path = std::string()): path(path){}
	virtual ~GenericMetadata(){}
	GenericMetadata(GenericMetadata &&other): path(std::move(other.path)){}
	virtual std::string album() = 0;
	virtual std::string track_title() = 0;
	virtual int track_number_int(){
		std::stringstream stream(this->track_number());
		int ret;
		if (!(stream >>ret))
			ret = -1;
		return ret;
	}
	virtual std::string track_number() = 0;
	virtual std::string track_artist() = 0;
	virtual std::string date() = 0;
	virtual bool track_gain(double &) = 0;
	virtual bool track_peak(double &) = 0;
	virtual bool album_gain(double &) = 0;
	virtual bool album_peak(double &) = 0;
	std::string track_id(){
		std::string ret = this->album();
		ret += " (";
		ret += this->date();
		ret += ") - ";
		ret += this->track_number();
		ret += " - ";
		ret += this->track_artist();
		ret += " - ";
		ret += this->track_title();
		return ret;
	}
	virtual bool picture(unsigned char *&buffer, size_t &length) = 0;
	const std::string &get_path() const{
		return this->path;
	}
};

template <typename T1, typename T2, typename T3>
typename std::enable_if<std::is_same<T2, T3>::value, bool>::type
find_in_map(const T1 &map, const T2 &what, T3 &dst){
	auto it = map.find(what);
	if (it == map.end())
		return false;
	dst = it->second;
	return true;
}

template <typename T1, typename T2, typename T3, typename F>
typename std::enable_if<!std::is_same<T2, T3>::value, bool>::type
find_in_map(const T1 &map, const T2 &what, T3 &dst, const F &f){
	auto it = map.find(what);
	if (it == map.end())
		return false;
	return f(dst, it->second);
}
