#include "FlacDecoder.h"
#include <stdexcept>
#include <boost/integer.hpp>
#include <libcue.h>

static const int cue_divisions_per_second = 75;

const char *to_string(FLAC__StreamDecoderInitStatus status){
	switch (status){
		case FLAC__STREAM_DECODER_INIT_STATUS_OK:
			return "OK";
		case FLAC__STREAM_DECODER_INIT_STATUS_UNSUPPORTED_CONTAINER:
			return "UNSUPPORTED_CONTAINER";
		case FLAC__STREAM_DECODER_INIT_STATUS_INVALID_CALLBACKS:
			return "INVALID_CALLBACKS";
		case FLAC__STREAM_DECODER_INIT_STATUS_MEMORY_ALLOCATION_ERROR:
			return "MEMORY_ALLOCATION_ERROR";
		case FLAC__STREAM_DECODER_INIT_STATUS_ERROR_OPENING_FILE:
			return "ERROR_OPENING_FILE";
		case FLAC__STREAM_DECODER_INIT_STATUS_ALREADY_INITIALIZED:
			return "ALREADY_INITIALIZED";
		default:
			return "unknown error";
	}
}

void release_buffer(FlacAudioBuffer *rr){
	if (rr)
		rr->buffer.release_function(rr->buffer.opaque);
}

UniqueFlacAudioBuffer allocate_buffer(size_t extra_size, size_t extra_data){
	auto ret = (FlacAudioBuffer *)malloc(sizeof(FlacAudioBuffer) + extra_size + extra_data);
	if (!ret)
		throw std::bad_alloc();
	ret->buffer.opaque = ret;
	ret->buffer.release_function = free;
	ret->buffer.samples_size = extra_size;
	ret->buffer.extra_data_size = extra_data;
	return {ret, release_buffer};
}

bool is_native_flac(const std::string &path){
	auto dot = path.rfind('.');
	if (dot == path.npos)
		return false;
	dot++;
	if (path.size() - dot != 4)
		return false;
	return
		tolower(path[dot + 0]) == 'f' &&
		tolower(path[dot + 1]) == 'l' &&
		tolower(path[dot + 2]) == 'a' &&
		tolower(path[dot + 3]) == 'c';
}

FlacDecoder::FlacDecoder(const char *path, const SlicedIO &io, Module *module):
		Decoder(module),
		io(io),
		path(path){
	this->seekable = this->io.can_seek();
	this->io.seek(0, SEEK_SET);
	this->set_md5_checking(false);
	this->set_metadata_respond_all();
#if 0
	if (!is_native_flac(this->path)){
		ogg_sync_state sync;
		ogg_sync_init(&sync);
		
		auto buffer = ogg_sync_buffer(&sync, 4096);
		auto bytes = this->io.read(buffer, 4096);
		ogg_sync_wrote(&sync, (long)bytes);
		ogg_page page;
		if (ogg_sync_pageout(&sync, &page) == 1){
			ogg_stream_state stream;
			ogg_stream_init(&stream, ogg_page_serialno(&page));
			vorbis_info info;
			vorbis_info_init(&info);
			vorbis_comment comment;
			vorbis_comment_init(&comment);
			if (ogg_stream_pagein(&stream, &page) >= 0){
				ogg_packet packet;
				auto ogg_stream_packetout_result = ogg_stream_packetout(&stream, &packet);
				if (ogg_stream_packetout_result == 1){
					__debugbreak();
				}
				ogg_packet_clear(&packet);
			}
			vorbis_comment_clear(&comment);
			vorbis_info_clear(&info);
			ogg_stream_destroy(&stream);
		}

		//...
		ogg_sync_destroy(&sync);
	}
#endif
	auto status = is_native_flac(this->path) ? this->init() : this->init_ogg();
	if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
		throw std::runtime_error((std::string)"FLAC initialization failed: " + to_string(status));
	auto status2 = this->process_until_end_of_metadata();
	if (!this->parse_cuesheet() && !this->native_cuesheet){
		StreamRange sr;
		sr.time_begin = 0;
		sr.sample_begin = 0;
		sr.frequency = this->format->freq;
		sr.time_length = this->get_seconds_length_internal();
		this->stream_ranges.push_back(sr);
	}
}

FlacDecoder::~FlacDecoder(){
	this->free_buffers();
}

template <NumberFormat Dst>
struct FlacCopier{};

template <>
struct FlacCopier<IntS8>{
	static void copy(std::uint8_t *&dst, FLAC__int32 src){
		*(std::int8_t *)(dst++) = (std::int8_t)src;
	}
};

template <>
struct FlacCopier<IntS16>{
	static void copy(std::uint8_t *&dst, FLAC__int32 src){
		*(dst++) = (std::uint8_t)(src & 0xFF);
		*(std::int8_t *)(dst++) = (std::int8_t)(src >> 8);
	}
};

template <>
struct FlacCopier<IntS24>{
	static void copy(std::uint8_t *&dst, FLAC__int32 src){
		*(dst++) = (std::uint8_t)(src & 0xFF);
		*(dst++) = (std::uint8_t)((src >> 8) & 0xFF);
		*(std::int8_t *)(dst++) = (std::int8_t)(src >> 16);
	}
};

template <>
struct FlacCopier<IntS32>{
	static void copy(std::uint8_t *&dst, FLAC__int32 src){
		*(dst++) = (std::uint8_t)(src & 0xFF);
		*(dst++) = (std::uint8_t)((src >> 8) & 0xFF);
		*(dst++) = (std::uint8_t)((src >> 16) & 0xFF);
		*(std::int8_t *)(dst++) = (std::int8_t)(src >> 24);
	}
};

template <NumberFormat Format>
UniqueFlacAudioBuffer copy_to_new_buffer(size_t extra_data, const FLAC__Frame *frame, const FLAC__int32 * const *buffer){
	auto channels = frame->header.channels;
	auto samples = frame->header.blocksize;
	auto ret = allocate_buffer(sizeof_NumberFormat(Format) * channels * samples, extra_data);
	ret->next = nullptr;
	ret->buffer.sample_count = samples;
	auto dst = ret->buffer.data;
	for (decltype(samples) sample = 0; sample < samples; sample++){
		for (decltype(channels) channel = 0; channel < channels; channel++){
			FlacCopier<Format>::copy(dst, buffer[channel][sample]);
		}
	}
	return ret;
}

FlacDecoder::allocator_func FlacDecoder::allocator_functions[] = {
	nullptr,
	copy_to_new_buffer<IntS8>,
	copy_to_new_buffer<IntS16>,
	copy_to_new_buffer<IntS24>,
	copy_to_new_buffer<IntS32>,
};

void FlacDecoder::free_buffers(){
	while (auto node = this->queue_head){
		this->queue_head = this->queue_head->next;
		release_buffer(node);
	}
	this->queue_tail = nullptr;
}

std::int64_t FlacDecoder::seek_to_sample_internal(std::int64_t pos, bool fast){
	bool result = this->seek_absolute(pos);
	if (result)
		this->free_buffers();
	return result ? pos : -1;
}

UniqueFlacAudioBuffer FlacDecoder::read_more_internal(){
	bool ok = true;
	auto state = this->get_state();
	while (!this->queue_head && (ok = this->process_single()) && (state = this->get_state()) != FLAC__STREAM_DECODER_END_OF_STREAM);
	if (state == FLAC__STREAM_DECODER_END_OF_STREAM || !ok)
		return {nullptr, release_buffer};
	auto ret = this->queue_head;
	this->queue_head = ret->next;
	if (!this->queue_head)
		this->queue_tail = nullptr;
	ret->next = nullptr;
	return {ret, release_buffer};
}

std::uint64_t FlacDecoder::get_pcm_length_internal(){
	return this->get_total_samples();
}

rational_t FlacDecoder::get_seconds_length_internal(){
	if (!this->format)
		return rational_t(-1, 1);
	return rational_t(this->get_pcm_length_internal(), this->time_resolution);
}

FLAC__StreamDecoderWriteStatus FlacDecoder::write_callback(const FLAC__Frame *frame, const FLAC__int32 * const *buffer){
	auto b = allocator_functions[frame->header.bits_per_sample / 8](this->extra_data_size, frame, buffer);
	if (!this->queue_tail)
		this->queue_head = this->queue_tail = b.release();
	else{
		this->queue_tail->next = b.release();
		this->queue_tail = this->queue_tail->next;
	}
	this->set_af();
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

FLAC__StreamDecoderReadStatus FlacDecoder::read_callback(FLAC__byte *buffer, size_t *bytes){
	if (this->at_eof)
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	auto read = this->io.read(buffer, *bytes);
	*bytes = read;
	if (!read){
		this->at_eof = true;
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	}
	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderSeekStatus FlacDecoder::seek_callback(FLAC__uint64 absolute_byte_offset){
	if (!this->seekable)
		return FLAC__STREAM_DECODER_SEEK_STATUS_UNSUPPORTED;
	auto pos = this->io.seek(absolute_byte_offset, SEEK_SET);
	this->at_eof = false;
	return pos == absolute_byte_offset ? FLAC__STREAM_DECODER_SEEK_STATUS_OK : FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
}

FLAC__StreamDecoderTellStatus FlacDecoder::tell_callback(FLAC__uint64 *absolute_byte_offset){
	auto pos = this->io.tell();
	if (pos < 0)
		return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
	*absolute_byte_offset = pos;
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus FlacDecoder::length_callback(FLAC__uint64 *stream_length){
	if (this->length == length_unsupported)
		return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
	if (this->length != length_unset){
		*stream_length = this->length;
		return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
	}
	if (!this->io.can_seek()){
		this->length = length_unsupported;
		return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
	}
	auto saved = this->io.tell();
	if (saved < 0)
		return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
	auto length = this->io.seek(0, SEEK_END);
	if (length < 0)
		return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
	*stream_length = length;
	if (saved != this->io.seek(saved, SEEK_SET))
		return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
	this->length = length;
	return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

void FlacDecoder::read_stream_info(const FLAC__StreamMetadata_StreamInfo &stream_info){
	AudioFormat format;
	switch (stream_info.bits_per_sample){
		case 8:
			format.format = IntS8;
			break;
		case 16:
			format.format = IntS16;
			break;
		case 24:
			format.format = IntS24;
			break;
		case 32:
			format.format = IntS32;
			break;
		default:
			format.format = Invalid;
			break;
	}
	format.channels = stream_info.channels;
	format.freq = stream_info.sample_rate;
	this->format = format;
	this->time_resolution = this->format->freq;
}

void FlacDecoder::read_cuesheet(const FLAC__StreamMetadata_CueSheet &cuesheet){
	this->native_cuesheet = true;
	for (decltype(cuesheet.num_tracks) i = 0; i < cuesheet.num_tracks - 1; i++){
		StreamRange range;
		range.sample_begin = cuesheet.tracks[i].offset;
		range.time_begin = rational_t(range.sample_begin, 1);
		range.time_length = rational_t(cuesheet.tracks[i + 1].offset, 1) - range.time_begin;
		range.frequency = 0;
		this->stream_ranges.push_back(range);
		//Chapters are ignored (for now).
	}
}

void FlacDecoder::read_picture(const FLAC__StreamMetadata_Picture &picture){
	this->metadata.set_front_cover(picture.data, picture.data_length);
}

void FlacDecoder::metadata_callback(const FLAC__StreamMetadata *metadata){
	switch (metadata->type){
		case FLAC__METADATA_TYPE_VORBIS_COMMENT:
			this->read_vorbis_comments(metadata->data.vorbis_comment);
			break;
		case FLAC__METADATA_TYPE_STREAMINFO:
			this->read_stream_info(metadata->data.stream_info);
			break;
		case FLAC__METADATA_TYPE_CUESHEET:
			this->read_cuesheet(metadata->data.cue_sheet);
			break;
		case FLAC__METADATA_TYPE_PICTURE:
			this->read_picture(metadata->data.picture);
			break;
		default:
			break;
	}
}

void process_cdtext(OggMetadata &metadata, Cdtext *cdtext){
	if (!cdtext)
		return;
	auto artist = cdtext_get(PTI_PERFORMER, cdtext);
	if (artist)
		metadata.add(OggMetadata::ARTIST, artist);
}

bool FlacDecoder::parse_cuesheet(){
	auto &cuesheet = this->metadata.cuesheet();
	if (!cuesheet.size())
		return false;
	std::unique_ptr<Cd, void (*)(Cd *)> cd(cue_parse_string(cuesheet.c_str()), cd_delete);
	if (!cd)
		return false;
	this->stream_count = cd_get_ntrack(cd.get());
	if (this->stream_count <= 0){
		this->stream_count = 1;
		return false;
	}

	if (!this->native_cuesheet){
		std::vector<StreamRange> ranges;
		for (int i = 0; i < this->stream_count; i++){
			auto track = cd_get_track(cd.get(), i + 1);
			if (!track)
				return false;
			StreamRange range;
			range.time_begin = rational_t(track_get_start(track), cue_divisions_per_second);
			range.time_length = rational_t(track_get_length(track), cue_divisions_per_second);
			range.frequency = this->format->freq;
			auto sample = range.time_begin * range.frequency;
			range.sample_begin = sample.numerator() / sample.denominator();
			ranges.push_back(range);
		}
		this->stream_ranges = std::move(ranges);
	}

	process_cdtext(this->metadata, cd_get_cdtext(cd.get()));

	for (int i = 0; i < this->stream_count; i++){
		auto track = cd_get_track(cd.get(), i + 1);
		if (!track)
			continue;
		OggMetadata metadata;
		process_cdtext(metadata, track_get_cdtext(track));
		
		auto rem = track_get_rem(track);
		if (rem){
			auto gain = rem_get(REM_REPLAYGAIN_TRACK_GAIN, rem);
			if (gain){
				metadata.add(OggMetadata::REPLAYGAIN_ALBUM_GAIN, gain);
			}
			auto peak = rem_get(REM_REPLAYGAIN_TRACK_PEAK, rem);
			if (peak){
				metadata.add(OggMetadata::REPLAYGAIN_ALBUM_PEAK, peak);
			}
		}
	}

	return true;
}

void FlacDecoder::read_vorbis_comments(const FLAC__StreamMetadata_VorbisComment &comments){
	for (auto i = comments.num_comments; i--;)
		this->metadata.add_vorbis_comment(comments.comments[i].entry, comments.comments[i].length);
}

void FlacDecoder::set_af(){
	AudioFormat format;
	switch (this->get_bits_per_sample()){
		case 8:
			format.format = IntS8;
			break;
		case 16:
			format.format = IntS16;
			break;
		case 24:
			format.format = IntS24;
			break;
		case 32:
			format.format = IntS32;
			break;
		default:
			format.format = Invalid;
			break;
	}
	format.channels = this->get_channels();
	format.freq = this->get_sample_rate();
	this->time_resolution = this->format->freq;
	if (!this->format)
		this->format = format;
	else if (memcmp(&*this->format, &format, sizeof(format)))
		throw std::exception();
}

DecoderSubstream *FlacDecoder::get_substream(int index){
	if (index < 0 || index >= this->stream_count)
		return nullptr;
	return new FlacDecoderSubstream(*this, 0, 0, this->get_pcm_length_internal(), 0, this->get_seconds_length_internal());
}

AudioBuffer *FlacDecoder::internal_read(const AudioFormat &format, size_t extra_data, int substream_index){
	auto ret = this->read_more_internal();
	if (!ret || !ret->buffer.sample_count)
		return nullptr;
	if (ret->buffer.extra_data_size < extra_data){
		this->extra_data_size = extra_data;
		auto copy = allocate_buffer(ret->buffer.samples_size, this->extra_data_size);
		auto copy2 = copy->buffer;
		memcpy(copy.get(), ret.get(), sizeof(*copy) + copy->buffer.samples_size + copy->buffer.extra_data_size);
		copy->buffer.extra_data_size = copy2.extra_data_size;
		copy->buffer.opaque = copy2.opaque;
		copy->buffer.release_function = copy2.release_function;
		copy->buffer.samples_size = copy2.samples_size;
		copy->buffer.extra_data_size = copy2.extra_data_size;
		ret = std::move(copy);
	}
	return &ret.release()->buffer;
}

FlacDecoderSubstream::FlacDecoderSubstream(
		FlacDecoder &parent,
		int index,
		std::int64_t first_sample,
		std::int64_t length,
		const rational_t &first_moment,
		const rational_t &seconds_length):
			AbstractFlacDecoderSubstream(parent, index, first_sample, length, first_moment, seconds_length),
			flac_parent(parent){
	this->format = this->flac_parent.get_format();
}
