#pragma once

#include <GenericMetadata.hpp>
#include <map>
#include <vector>
#include <memory>

class OggMetadata : public GenericMetadata{
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
public:
	OggMetadata(const std::string &path = std::string()): GenericMetadata(path){}
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
	//std::shared_ptr<OggMetadata> clone(){
	//	return std::shared_ptr<OggMetadata>(new OggMetadata(*this));
	//}
	std::string get_string_or_nothing(const std::string &key) const{
		auto i = this->map.find(key);
		return i == this->map.end() ? std::string() : i->second;
	}
	std::string album() override{
		return this->get_string_or_nothing(ALBUM);
	}
	std::string track_title() override;
	std::string track_number() override{
		return this->get_string_or_nothing(TRACKNUMBER);
	}
	std::string track_artist() override{
		return this->get_string_or_nothing(ARTIST);
	}
	std::string date() override{
		return this->get_string_or_nothing(DATE);
	}
	bool picture(unsigned char *&buffer, size_t &length) override;
	bool track_gain(double &) override;
	bool track_peak(double &) override;
	bool album_gain(double &) override;
	bool album_peak(double &) override;
};
