#include "OggMetadata.h"
#include <base64.hpp>
#include <sstream>

std::string OggMetadata::ALBUM                  = "ALBUM";
std::string OggMetadata::ARTIST                 = "ARTIST";
std::string OggMetadata::DATE                   = "DATE";
std::string OggMetadata::METADATA_BLOCK_PICTURE = "METADATA_BLOCK_PICTURE";
std::string OggMetadata::OPUS                   = "OPUS";
std::string OggMetadata::PART                   = "PART";
std::string OggMetadata::TITLE                  = "TITLE";
std::string OggMetadata::TRACKNUMBER            = "TRACKNUMBER";
static std::string REPLAYGAIN_TRACK_GAIN        = "REPLAYGAIN_TRACK_GAIN";
static std::string REPLAYGAIN_TRACK_PEAK        = "REPLAYGAIN_TRACK_PEAK";
static std::string REPLAYGAIN_ALBUM_GAIN        = "REPLAYGAIN_ALBUM_GAIN";
static std::string REPLAYGAIN_ALBUM_PEAK        = "REPLAYGAIN_ALBUM_PEAK";
static const std::string * const replaygain_strings[] = {
	&REPLAYGAIN_TRACK_GAIN,
	&REPLAYGAIN_TRACK_PEAK,
	&REPLAYGAIN_ALBUM_GAIN,
	&REPLAYGAIN_ALBUM_PEAK,
};

bool string_to_double(double &dst, const std::string &src){
	std::stringstream stream(src);
	return !!(stream >> dst);
}

bool OggMetadata::track_gain(double &dst){
	return find_in_map(this->map, REPLAYGAIN_TRACK_GAIN, dst, string_to_double);
}

bool OggMetadata::track_peak(double &dst){
	return find_in_map(this->map, REPLAYGAIN_TRACK_PEAK, dst, string_to_double);
}

bool OggMetadata::album_gain(double &dst){
	return find_in_map(this->map, REPLAYGAIN_ALBUM_GAIN, dst, string_to_double);
}

bool OggMetadata::album_peak(double &dst){
	return find_in_map(this->map, REPLAYGAIN_ALBUM_PEAK, dst, string_to_double);
}

std::string OggMetadata::track_title(){
	std::string ret;
	auto i = this->map.find(TITLE);
	if (i == this->map.end())
		return "";
	ret += i->second;
	i = this->map.find(OPUS);
	if (i != this->map.end()){
		ret += " (";
		ret += i->second;
		ret += ')';
	}
	i = this->map.find(PART);
	if (i != this->map.end()){
		ret += ", ";
		ret += i->second;
	}
	return ret;
}

template <typename T>
void toupper_inplace(std::basic_string<T> &s){
	for (auto &c : s)
		c = ::toupper(c);
}

void read_32_big_bits(std::uint32_t &dst, const void *buf){
	auto p = (const unsigned char *)buf;
	dst = 0;
	for (int i = 4; i--; p++){
		dst <<= 8;
		dst |= *p;
	}
}

bool read_32_big_bits(std::uint32_t &dst, const std::vector<unsigned char> &src, size_t offset){
	if (src.size() - offset < 4)
		return 0;
	read_32_big_bits(dst, &src[offset]);
	return 1;
}

enum mpg123_id3_pic_type{
	 mpg123_id3_pic_other          =  0
	,mpg123_id3_pic_icon           =  1
	,mpg123_id3_pic_other_icon     =  2
	,mpg123_id3_pic_front_cover    =  3
	,mpg123_id3_pic_back_cover     =  4
	,mpg123_id3_pic_leaflet        =  5
	,mpg123_id3_pic_media          =  6
	,mpg123_id3_pic_lead           =  7
	,mpg123_id3_pic_artist         =  8
	,mpg123_id3_pic_conductor      =  9
	,mpg123_id3_pic_orchestra      = 10
	,mpg123_id3_pic_composer       = 11
	,mpg123_id3_pic_lyricist       = 12
	,mpg123_id3_pic_location       = 13
	,mpg123_id3_pic_recording      = 14
	,mpg123_id3_pic_performance    = 15
	,mpg123_id3_pic_video          = 16
	,mpg123_id3_pic_fish           = 17
	,mpg123_id3_pic_illustration   = 18
	,mpg123_id3_pic_artist_logo    = 19
	,mpg123_id3_pic_publisher_logo = 20
};

void OggMetadata::add_vorbis_comment(const void *buffer, size_t length){
	std::string comment((char *)buffer, length);
	size_t equals = comment.find('=');
	if (equals == comment.npos)
		//invalid comment
		return;
	auto field_name = comment.substr(0, equals);
	toupper_inplace(field_name);
	equals++;
	if (field_name == METADATA_BLOCK_PICTURE){
		std::vector<unsigned char> decoded_buffer;
		b64_decode(decoded_buffer, (const char *)buffer + equals, length - equals);
		std::uint32_t picture_type,
			mime_length,
			description_length,
			picture_width,
			picture_height,
			picture_bit_depth,
			picture_colors,
			picture_size;
		size_t offset = 0;

		if (!read_32_big_bits(picture_type, decoded_buffer, offset) || picture_type != (std::uint32_t)mpg123_id3_pic_front_cover)
			return;
		offset += 4;

		if (!read_32_big_bits(mime_length, decoded_buffer, offset))
			return;
		offset += 4 + mime_length;

		if (!read_32_big_bits(description_length, decoded_buffer, offset))
			return;
		offset += 4 + description_length;

		if (!read_32_big_bits(picture_width, decoded_buffer, offset))
			return;
		offset += 4;

		if (!read_32_big_bits(picture_height, decoded_buffer, offset))
			return;
		offset += 4;

		if (!read_32_big_bits(picture_bit_depth, decoded_buffer, offset))
			return;
		offset += 4;

		if (!read_32_big_bits(picture_colors, decoded_buffer, offset))
			return;
		offset += 4;

		if (!read_32_big_bits(picture_size, decoded_buffer, offset))
			return;
		offset += 4;

		if (decoded_buffer.size() - offset < picture_size)
			return;

		this->ogg_picture.assign(decoded_buffer.begin() + offset, decoded_buffer.begin() + (offset + picture_size));
	}
	auto field_value = comment.substr(equals);
	this->add(field_name, field_value);
}

bool OggMetadata::picture(unsigned char *&buffer, size_t &length){
	if (!this->ogg_picture.size())
		return false;
	buffer = &this->ogg_picture[0];
	length = this->ogg_picture.size();
	return true;
}
