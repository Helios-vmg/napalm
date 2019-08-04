#include "Mp3Decoder.h"
#include <stdexcept>
#include <boost/integer.hpp>
#include <mpg123.h>

static const NumberFormat number_format = Float32;

static const std::string REPLAYGAIN_TRACK_GAIN = "REPLAYGAIN_TRACK_GAIN";
static const std::string REPLAYGAIN_TRACK_PEAK = "REPLAYGAIN_TRACK_PEAK";
static const std::string REPLAYGAIN_ALBUM_GAIN = "REPLAYGAIN_ALBUM_GAIN";
static const std::string REPLAYGAIN_ALBUM_PEAK = "REPLAYGAIN_ALBUM_PEAK";

static const std::string *replaygain_strings[] = {
	&REPLAYGAIN_TRACK_GAIN,
	&REPLAYGAIN_TRACK_PEAK,
	&REPLAYGAIN_ALBUM_GAIN,
	&REPLAYGAIN_ALBUM_PEAK,
};

static ssize_t mp3_read(void *handle, void *dst, size_t size){
	auto &io = *(ExternalIO *)handle;
	auto ret = io.read(io.user_data, dst, size);
	return ret ? ret : -1;
}

static off_t mp3_lseek(void *handle, off_t offset, int whence){
	auto &io = *(ExternalIO *)handle;
	return (off_t)io.seek(io.user_data, offset, whence);
}

Mp3Decoder::Mp3Decoder(const char *path, const ExternalIO &io, Module *module):
		Decoder(module),
		io(io),
		module(module),
		path(path){
	this->has_played = false;
	this->handle = nullptr;
	int error;

	this->handle = mpg123_new(nullptr, &error);
	if (!this->handle)
		throw std::runtime_error((std::string)"mpg123_new() failed with error " + mpg123_error_to_string(error));

	error = mpg123_replace_reader_handle(this->handle, mp3_read, mp3_lseek, nullptr);
	if (error != MPG123_OK)
		throw std::runtime_error((std::string)"mpg123_replace_reader() failed with error " + mpg123_error_to_string(error));

	error = mpg123_open_handle(this->handle, &this->io);
	if (error != MPG123_OK)
		throw std::runtime_error("MP3 read failed.");

	error = mpg123_format_none(this->handle);
	if (error != MPG123_OK)
		throw std::runtime_error("mpg123_format_none() failed???");

	error = mpg123_format_all(this->handle);
	if (error != MPG123_OK)
		throw std::runtime_error((std::string)"mpg123_format_all() failed with error " + mpg123_error_to_string(error));

	this->length = std::numeric_limits<decltype(this->length)>::max();
	this->seconds_per_frame = 0;

	long long_param;
	double double_param;
	mpg123_getparam(this->handle, MPG123_FLAGS, &long_param, &double_param);
	long_param |= MPG123_PICTURE;
	mpg123_param(this->handle, MPG123_FLAGS, long_param, double_param);

	this->set_format();

	error = mpg123_scan(this->handle);
	if (error != MPG123_OK)
		return;
	this->length = mpg123_length(this->handle);
	this->seconds_per_frame = mpg123_tpf(this->handle);
	this->stream_count = 1;
	this->stream_ranges.push_back({rational_t(0, 1), rational_t((std::int64_t)this->length, this->format.freq), 0, (std::int64_t)this->length, this->format.freq});

	mpg123_id3(this->handle, (mpg123_id3v1 **)&this->id3v1, (mpg123_id3v2 **)&this->id3v2);
	this->metadata_done = false;
	this->check_for_metadata();
}

void Mp3Decoder::set_format(){
	long frequency;
	int channels;
	int encoding;
	auto error = mpg123_getformat(this->handle, &frequency, &channels, &encoding);
	if (error != MPG123_OK)
		throw std::runtime_error((std::string)"mpg123_getformat() failed with error " + mpg123_error_to_string(error));
	if (frequency < 1 || channels < 1)
		throw std::runtime_error("mpg123_getformat() returned invalid data");
	this->format.freq = frequency;
	this->format.channels = channels;
	switch (encoding){
		case MPG123_ENC_UNSIGNED_8:
			this->format.format = IntU8;
			break;
		case MPG123_ENC_SIGNED_8:
			this->format.format = IntS8;
			break;
		case MPG123_ENC_UNSIGNED_16:
			this->format.format = IntU16;
			break;
		case MPG123_ENC_SIGNED_16:
			this->format.format = IntS16;
			break;
		case MPG123_ENC_UNSIGNED_24:
			this->format.format = IntU24;
			break;
		case MPG123_ENC_SIGNED_24:
			this->format.format = IntS24;
			break;
		case MPG123_ENC_UNSIGNED_32:
			this->format.format = IntU32;
			break;
		case MPG123_ENC_SIGNED_32:
			this->format.format = IntS32;
			break;
		case MPG123_ENC_FLOAT_32:
			this->format.format = Float32;
			break;
		case MPG123_ENC_FLOAT_64:
			this->format.format = Float64;
			break;
		default:
			throw std::runtime_error("mpg123_getformat() returned an unsupported format");
	}
}

void Mp3Decoder::check_for_metadata(){
	if (this->metadata_done)
		return;
	auto result = mpg123_meta_check(this->handle);
	if (!(result & MPG123_ID3))
		return;
	auto p = (mpg123_id3v2 *)this->id3v2;
	auto meta = std::make_unique<Mp3Metadata>();
#define SET_ID3_DATA(x) if (p->x) meta->id3_##x = std::string((const char *)p->x->p, strnlen(p->x->p, p->x->size))
	SET_ID3_DATA(title);
	SET_ID3_DATA(artist);
	SET_ID3_DATA(album);
	SET_ID3_DATA(year);
	SET_ID3_DATA(genre);
	SET_ID3_DATA(comment);
	for (size_t i = 0; i < p->texts; i++)
		meta->add_mp3_text(p->text + i);
	for (size_t i = 0; i < p->extras; i++)
		meta->add_mp3_extra(p->extra + i);
	for (size_t i = 0; i < p->pictures; i++){
		if (p->picture[i].type != mpg123_id3_pic_front_cover)
			continue;
		meta->add_picture(p->picture[i].data, p->picture[i].size);
		break;
	}
	mpg123_meta_free(this->handle);
	this->metadata_done = true;
}

void Mp3Metadata::add_mp3_text(const void *voidtext){
	auto text = (mpg123_text *)voidtext;
	std::string key((const char *)text->id, strnlen(text->id, 4));
	std::string value((const char *)text->text.p, strnlen(text->text.p, text->text.size));
	this->texts.emplace(std::move(key), std::move(value));
}

template <typename T>
int strcmp_case(const std::basic_string<T> &a, const std::basic_string<T> &b){
	const T * const ap = &a[0];
	const T * const bp = &b[0];
	const auto n = std::min(a.size(), b.size());
	for (size_t i = 0; i < n; i++){
		int d = tolower(ap[i]) - tolower(bp[i]);
		if (d)
			return d;
	}
	return (int)a.size() - (int)b.size();
}

template <typename T>
int strcmp_case(const std::basic_string<T> &a, const T *b){
	const T * const ap = &a[0];
	auto bl = (size_t)strlen(b);
	const auto n = std::min(a.size(), bl);
	for (size_t i = 0; i < n; i++){
		int d = tolower(ap[i]) - tolower(b[i]);
		if (d)
			return d;
	}
	return (int)a.size() - (int)bl;
}

template <typename T>
void toupper_inplace(std::basic_string<T> &s){
	for (auto &c : s)
		c = ::toupper(c);
}

static bool is_replaygain_tag(const std::string &s){
	for (auto p : replaygain_strings)
		if (!strcmp_case(s, *p))
			return true;
	return false;
}

void Mp3Metadata::add_mp3_extra(const void *voidtext){
	auto text = (mpg123_text *)voidtext;
	std::string key((const char *)text->id, strnlen(text->id, 4));
	std::string value((const char *)text->text.p, strnlen(text->text.p, text->text.size));
	if (is_replaygain_tag(key))
		toupper_inplace(key);
	this->texts.emplace(std::move(key), std::move(value));
}

void Mp3Metadata::add_picture(const void *buffer, size_t length){
	this->id3_picture.resize(length);
	memcpy(&this->id3_picture[0], buffer, length);
}

std::string Mp3Metadata::track_number(){
	auto i = this->texts.find("TRCK");
	return i == this->texts.end() ? std::string() : i->second;
}

bool Mp3Metadata::front_cover(unsigned char *&buffer, size_t &length){
	if (!this->id3_picture.size())
		return false;
	buffer = &this->id3_picture[0];
	length = this->id3_picture.size();
	return true;
}

template <typename T>
bool find_in_map(const T &map, const std::string &what, double &dst){
	auto it = map.find(what);
	if (it == map.end())
		return false;
	return !!(std::stringstream(it->second) >> dst);
}

bool Mp3Metadata::track_gain(double &dst){
	return find_in_map(this->texts, REPLAYGAIN_TRACK_GAIN, dst);
}

bool Mp3Metadata::track_peak(double &dst){
	return find_in_map(this->texts, REPLAYGAIN_TRACK_PEAK, dst);
}

bool Mp3Metadata::album_gain(double &dst){
	return find_in_map(this->texts, REPLAYGAIN_ALBUM_GAIN, dst);
}

bool Mp3Metadata::album_peak(double &dst){
	return find_in_map(this->texts, REPLAYGAIN_ALBUM_PEAK, dst);
}

DecoderSubstream *Mp3Decoder::get_substream(int index){
	if (index < 0 || index >= this->stream_count)
		return nullptr;
	return new Mp3DecoderSubstream(*this, index, 0, this->length, 0, rational_t(this->length, this->format.freq));
}

struct Mp3AudioBuffer{
	AudioBuffer read_result;
};

void release(Mp3AudioBuffer *rr){
	if (rr)
		rr->read_result.release_function(rr->read_result.opaque);
}

static std::unique_ptr<Mp3AudioBuffer, void(*)(Mp3AudioBuffer *)> alloc(size_t extra_size, size_t extra_data){
	auto ret = (Mp3AudioBuffer *)malloc(sizeof(Mp3AudioBuffer) + extra_size + extra_data);
	if (!ret)
		throw std::bad_alloc();
	ret->read_result.opaque = ret;
	ret->read_result.release_function = free;
	ret->read_result.samples_size = extra_size;
	ret->read_result.extra_data_size = extra_data;
	return { ret, release };
}

AudioBuffer *Mp3Decoder::internal_read(const AudioFormat &af, size_t extra_data, int substream_index){
	const size_t bytes_per_sample = af.channels * sizeof_NumberFormat(af.format);
	unsigned char *buffer;
	off_t offset;
	size_t size;
	int error = mpg123_decode_frame(this->handle, &offset, &buffer, &size);
	this->check_for_metadata();
	if (error == MPG123_DONE || error == MPG123_NEW_FORMAT)
		return nullptr;
	if (error != MPG123_OK)
		throw std::runtime_error((std::string)"Mp3Decoder: " + mpg123_error_to_string(error));
	const auto samples = size / bytes_per_sample;
	size = samples * bytes_per_sample;
	auto ret = alloc(size, extra_data);
	memcpy(ret->read_result.data, buffer, size);
	ret->read_result.sample_count = samples;
	return &ret.release()->read_result;
}

std::int64_t Mp3Decoder::seek_to_sample_internal(std::int64_t pos, bool fast){
	if (!fast)
		return mpg123_seek(this->handle, (off_t)pos, SEEK_SET);
	auto frame = mpg123_timeframe(this->handle, (double)pos / this->format.freq);
	return (std::int64_t)mpg123_seek_frame(this->handle, frame, SEEK_SET);
}
