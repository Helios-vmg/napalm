#pragma once

#include <string>
#include <cstring>
#include <cstdint>

struct BasicTrackInfoHelper{
	std::string album,
		track_title,
		track_number,
		track_artist,
		date,
		track_id,
		path;
};

typedef const char *cstr;

struct NumericTrackInfo{
	std::int32_t track_number_int;
	RationalValue duration;
	double track_gain;
	double track_peak;
	double album_gain;
	double album_peak;

	NumericTrackInfo(){
		this->track_number_int = -1;
		this->track_gain = -1;
		this->track_peak = -1;
		this->album_gain = -1;
		this->album_peak = -1;
	}
};

struct BasicTrackInfo{
	cstr album,
		track_title,
		track_number,
		track_artist,
		date,
		track_id,
		path;
	NumericTrackInfo numeric_track_info;
	BasicTrackInfoHelper *helper;

	BasicTrackInfo(){
		memset(this, 0, sizeof(*this));
		this->numeric_track_info = NumericTrackInfo();
	}
	BasicTrackInfo(const NumericTrackInfo &numeric_track_info, BasicTrackInfoHelper &&helper){
		this->helper = new BasicTrackInfoHelper(std::move(helper));
		this->album = this->helper->album.c_str();
		this->track_title = this->helper->track_title.c_str();
		this->track_number = this->helper->track_number.c_str();
		this->track_artist = this->helper->track_artist.c_str();
		this->date = this->helper->date.c_str();
		this->track_id = this->helper->track_id.c_str();
		this->path = this->helper->path.c_str();
		this->numeric_track_info = numeric_track_info;
	}
	BasicTrackInfo(const BasicTrackInfo &other) = delete;
	const BasicTrackInfo &operator=(const BasicTrackInfo &other) = delete;
	BasicTrackInfo(BasicTrackInfo &&other){
		memcpy(this, &other, sizeof(*this));
		memset(&other, 0, sizeof(other));
	}
	const BasicTrackInfo &operator=(BasicTrackInfo &&other){
		memcpy(this, &other, sizeof(*this));
		memset(&other, 0, sizeof(other));
		return *this;
	}
	~BasicTrackInfo(){
		delete this->helper;
	}
};
