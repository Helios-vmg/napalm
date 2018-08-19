#pragma once

#include <string>
#include <type_traits>
#include <sstream>
#include <vector>

class AbstractMetadataBuffer{
public:
	virtual ~AbstractMetadataBuffer(){}
	virtual size_t size() const = 0;
	virtual const std::uint8_t *data() const = 0;
	std::string to_string() const{
		auto d = (const char *)this->data();
		auto s = this->size();
		if (!s || !d)
			return std::string();
		return std::string(d, s);
	}
};

class StringMetadataBuffer : public AbstractMetadataBuffer{
	std::string string;
	static std::uint8_t static_byte;
public:
	StringMetadataBuffer() = default;
	StringMetadataBuffer(std::string &&s): string(std::move(s)){}
	StringMetadataBuffer(const StringMetadataBuffer &other) = delete;
	StringMetadataBuffer &operator=(const StringMetadataBuffer &other) = delete;
	StringMetadataBuffer(StringMetadataBuffer &&other){
		this->string = std::move(other.string);
	}
	const StringMetadataBuffer &operator=(StringMetadataBuffer &&other){
		this->string = std::move(other.string);
		return *this;
	}
	size_t size() const override{
		return this->string.size();
	}
	const std::uint8_t *data() const override{
		if (!this->string.size())
			return &this->static_byte;
		return (const std::uint8_t *)&this->string[0];
	}
};

class VectorMetadataBuffer : public AbstractMetadataBuffer{
	std::vector<std::uint8_t> vector;
	static std::uint8_t static_byte;
public:
	VectorMetadataBuffer() = default;
	VectorMetadataBuffer(std::vector<std::uint8_t> &&v): vector(std::move(v)){}
	VectorMetadataBuffer(const StringMetadataBuffer &other) = delete;
	VectorMetadataBuffer &operator=(const VectorMetadataBuffer &other) = delete;
	VectorMetadataBuffer(VectorMetadataBuffer &&other){
		this->vector = std::move(other.vector);
	}
	const VectorMetadataBuffer &operator=(VectorMetadataBuffer &&other){
		this->vector = std::move(other.vector);
		return *this;
	}
	size_t size() const override{
		return this->vector.size();
	}
	const std::uint8_t *data() const override{
		if (!this->vector.size())
			return &this->static_byte;
		return &this->vector[0];
	}
};

class AbstractMetadataIterator{
public:
	virtual ~AbstractMetadataIterator(){}
	virtual bool next(std::string &key, std::string &value) = 0;
};

class GenericMetadata{
protected:
	StringMetadataBuffer path;
public:
	GenericMetadata(const std::string &path = std::string()){
		auto temp = path;
		this->path = StringMetadataBuffer(std::move(temp));
	}
	virtual ~GenericMetadata(){}
	GenericMetadata(GenericMetadata &&other): path(std::move(other.path)){}
	virtual AbstractMetadataBuffer &album() = 0;
	virtual AbstractMetadataBuffer &track_title() = 0;
	virtual AbstractMetadataBuffer &track_number() = 0;
	virtual AbstractMetadataBuffer &track_artist() = 0;
	virtual AbstractMetadataBuffer &date() = 0;
	virtual double track_gain() = 0;
	virtual double track_peak() = 0;
	virtual double album_gain() = 0;
	virtual double album_peak() = 0;
	virtual AbstractMetadataBuffer &front_cover() = 0;
	const AbstractMetadataBuffer &get_path() const{
		return this->path;
	}
	virtual int track_number_int(){
		std::stringstream stream(this->track_number().to_string());
		int ret;
		if (!(stream >> ret))
			ret = -1;
		return ret;
	}
	virtual std::unique_ptr<AbstractMetadataIterator> get_iterator(){
		return nullptr;
	}

	std::string track_id(){
		std::string ret = this->album().to_string();
		ret += " (";
		ret += this->date().to_string();
		ret += ") - ";
		ret += this->track_number().to_string();
		ret += " - ";
		ret += this->track_artist().to_string();
		ret += " - ";
		ret += this->track_title().to_string();
		return ret;
	}
};
