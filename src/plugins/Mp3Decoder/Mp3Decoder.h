#pragma once

#include "../common/decoder.hpp"
#include "Module.h"
#include <map>

struct mpg123_handle_struct;

typedef struct mpg123_handle_struct mpg123_handle;

class Mp3Decoder : public Decoder{
	WrappedExternalIO io;
	Module *module;
	std::string path;
	static mp3_static_data static_data;
	mpg123_handle *handle;
	bool has_played;
	std::uint64_t length;
	double seconds_per_frame;
	void *id3v1;
	void *id3v2;
	bool metadata_done;
	bool seekable;
	int stream_count;
	AudioFormat format;

	void check_for_metadata();
	void set_format();
	AudioBuffer *internal_read(const AudioFormat &, size_t extra_data, int substream_index) override;
	std::int64_t seek_to_sample_internal(std::int64_t pos, bool fast) override;
public:
	Mp3Decoder(const char *path, const ExternalIO &io, Module *module);
	~Mp3Decoder(){}
	int get_substream_count() override{
		return this->stream_count;
	}
	DecoderSubstream *get_substream(int index) override;
	const std::string &get_path() const{
		return this->path;
	}
	Module &get_module(){
		return *this->module;
	}
	AudioFormat get_format() const{
		return this->format;
	}
};

class Mp3MetadataIterator;

class Mp3Metadata{
	friend class Mp3Decoder;
	std::string id3_title;
	std::string id3_artist;
	std::string id3_album;
	std::string id3_year;
	std::string id3_genre;
	std::string id3_comment;
	std::map<std::string, std::string> texts;
	std::vector<unsigned char> id3_picture;
public:
	void add_mp3_text(const void *text);
	void add_mp3_extra(const void *text);
	void add_picture(const void *buffer, size_t length);
	std::string album(){
		return this->id3_album;
	}
	std::string track_title(){
		return this->id3_title;
	}
	std::string track_number();
	std::string track_artist(){
		return this->id3_artist;
	}
	std::string date(){
		return this->id3_year;
	}
	bool front_cover(unsigned char *&buffer, size_t &length);
	bool track_gain(double &);
	bool track_peak(double &);
	bool album_gain(double &);
	bool album_peak(double &);
	template <typename T>
	std::unique_ptr<std::pair<Mp3MetadataIterator, T>> get_iterator(const T &);
};

class Mp3MetadataIterator{
public:
	typedef std::map<std::string, std::string>::iterator T;
private:
	T current, end;
public:
	Mp3MetadataIterator(const T &begin, const T &end):
		current(begin),
		end(end){}
	bool next(std::string &key, std::string &value){
		if (this->current == this->end)
			return false;
		auto it = this->current++;
		key = it->first;
		value = it->second;
		return true;
	}
};

template <typename T>
std::unique_ptr<std::pair<Mp3MetadataIterator, T>> Mp3Metadata::get_iterator(const T &x){
	return std::make_unique<std::pair<Mp3MetadataIterator, T>>(Mp3MetadataIterator(this->texts.begin(), this->texts.end()), x);
}

struct Mp3MetadataWrapper{
	Mp3Decoder *decoder;
	Mp3Metadata metadata;
	Mp3MetadataWrapper(Mp3Decoder &decoder, const Mp3Metadata &metadata):
		decoder(&decoder),
		metadata(metadata){}
};

class Mp3DecoderSubstream : public DecoderSubstream{
	Mp3Decoder &mp3_parent;
	Mp3Metadata metadata;
public:
	Mp3DecoderSubstream(
			Mp3Decoder &parent,
			int index,
			std::int64_t first_sample = 0,
			std::int64_t length = std::numeric_limits<std::uint64_t>::max(),
			const rational_t &first_moment = 0,
			const rational_t &seconds_length = {-1, 1}): DecoderSubstream(parent, index, first_sample, length, first_moment, seconds_length), mp3_parent(parent){
		this->format = parent.get_format();
	}
	Mp3Decoder &get_parent(){
		return this->mp3_parent;
	}
	Mp3MetadataWrapper *get_metadata(){
		return new Mp3MetadataWrapper(this->mp3_parent, this->metadata);
	}
};
