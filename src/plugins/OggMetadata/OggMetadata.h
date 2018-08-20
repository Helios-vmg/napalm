#pragma once

#include <map>
#include <vector>
#include <memory>

class OggMetadataIterator;

class OggMetadata{
	std::map<std::string, std::string> map;
	std::vector<unsigned char> ogg_picture;
	static std::string ALBUM,
		TITLE,
		ARTIST,
		TRACKNUMBER,
		DATE,
		OPUS,
		PART,
		METADATA_BLOCK_PICTURE;
	void add(const std::string &key, std::string &value){
		this->map[key] = value;
	}
	static std::string empty_string;
public:
	OggMetadata() = default;
	OggMetadata(const OggMetadata &) = default;
	OggMetadata(OggMetadata &&other) = delete;
	const OggMetadata &operator=(OggMetadata &&other) = delete;
	void add_vorbis_comment(const void *, size_t);
	template <typename F>
	void iterate(F &f){
		for (auto &p : this->map){
			f(p.first, p.second);
		}
	}
	const std::string &get_string_or_nothing(const std::string &key) const{
		auto i = this->map.find(key);
		return i == this->map.end() ? empty_string : i->second;
	}
	const std::string &album(){
		return this->get_string_or_nothing(ALBUM);
	}
	std::string track_title();
	const std::string &track_number(){
		return this->get_string_or_nothing(TRACKNUMBER);
	}
	const std::string &track_artist(){
		return this->get_string_or_nothing(ARTIST);
	}
	const std::string &date(){
		return this->get_string_or_nothing(DATE);
	}
	const std::vector<std::uint8_t> &front_cover() const{
		return this->ogg_picture;
	}
	double track_gain();
	double track_peak();
	double album_gain();
	double album_peak();
	template <typename T>
	std::unique_ptr<std::pair<OggMetadataIterator, T>> get_iterator(const T &);
};

class OggMetadataIterator{
public:
	typedef std::map<std::string, std::string>::iterator T;
private:
	T current, end;
public:
	OggMetadataIterator(const T &begin, const T &end):
		current(begin),
		end(end){}
	bool next(std::string &key, std::string &value);
};

	template <typename T>
std::unique_ptr<std::pair<OggMetadataIterator, T>> OggMetadata::get_iterator(const T &x){
	return std::make_unique<std::pair<OggMetadataIterator, T>>(OggMetadataIterator(this->map.begin(), this->map.end()), x);
}
