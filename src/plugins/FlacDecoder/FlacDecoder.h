#pragma once

#include "../common/decoder.hpp"
#include "../OggMetadata/OggMetadata.h"
#include "Module.h"
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include <FLAC++/decoder.h>
#include <boost/optional.hpp>
#include "AbstractFlacDecoder.h"

struct FlacAudioBuffer{
	FlacAudioBuffer *next;
	AudioBuffer buffer;
};

typedef std::unique_ptr<FlacAudioBuffer, void (*)(FlacAudioBuffer *)> UniqueFlacAudioBuffer;

class FlacDecoder : public Decoder, public FLAC::Decoder::Stream{
	SlicedIO io;
	std::string path;
	FlacAudioBuffer *queue_head = nullptr,
		*queue_tail = nullptr;
	typedef UniqueFlacAudioBuffer (*allocator_func)(size_t, const FLAC__Frame *, const FLAC__int32 * const *);
	static allocator_func allocator_functions[];
	boost::optional<AudioFormat> format;
	bool seekable,
		at_eof = false;
	static const std::int64_t length_unset = -1;
	static const std::int64_t length_unsupported = -2;
	std::int64_t byte_length = length_unset;
	std::int64_t sample_length = length_unset;
	size_t extra_data_size = 32;
	OggMetadata metadata;
	std::vector<OggMetadata> track_metadata;
	bool native_cuesheet = false;

	FLAC__StreamDecoderWriteStatus write_callback(const FLAC__Frame *frame, const FLAC__int32 * const *buffer) override;
	FLAC__StreamDecoderReadStatus read_callback(FLAC__byte *buffer, size_t *bytes) override;
	FLAC__StreamDecoderSeekStatus seek_callback(FLAC__uint64 absolute_byte_offset) override;
	bool eof_callback() override{
		return this->at_eof;
	}
	FLAC__StreamDecoderTellStatus tell_callback(FLAC__uint64 *absolute_byte_offset) override;
	FLAC__StreamDecoderLengthStatus length_callback(FLAC__uint64 *stream_length) override;
	void error_callback(::FLAC__StreamDecoderErrorStatus status) override{
		throw std::runtime_error(FLAC__StreamDecoderErrorStatusString[status]);
	}
	void metadata_callback(const ::FLAC__StreamMetadata *metadata) override;
	
	UniqueFlacAudioBuffer read_more_internal();
	std::uint64_t get_pcm_length_internal();
	rational_t get_seconds_length_internal();

	void free_buffers();

	void set_af();
	AudioBuffer *internal_read(const AudioFormat &, size_t extra_data, int substream_index) override;
	std::int64_t seek_to_sample_internal(std::int64_t pos, bool fast) override;
	bool parse_cuesheet();
	void read_vorbis_comments(const FLAC__StreamMetadata_VorbisComment &);
	void read_stream_info(const FLAC__StreamMetadata_StreamInfo &);
	void read_cuesheet(const FLAC__StreamMetadata_CueSheet &);
	void read_picture(const FLAC__StreamMetadata_Picture &);
public:
	FlacDecoder(const char *path, const SlicedIO &io, Module *module);
	~FlacDecoder();
	int get_substream_count() override{
		return (int)this->stream_ranges.size();
	}
	DecoderSubstream *get_substream(int index) override;
	const std::string &get_path() const{
		return this->path;
	}
	Module &get_module(){
		return *this->module;
	}
	const OggMetadata &get_metadata(){
		return this->metadata;
	}
	AudioFormat get_format() const{
		return *this->format;
	}
};

class FlacDecoderSubstream : public AbstractFlacDecoderSubstream{
	FlacDecoder &flac_parent;
	OggMetadata metadata;
public:
	FlacDecoderSubstream(
		FlacDecoder &parent,
		int index,
		const OggMetadata &metadata,
		std::int64_t first_sample = 0,
		std::int64_t length = std::numeric_limits<std::uint64_t>::max(),
		const rational_t &first_moment = 0,
		const rational_t &seconds_length = {-1, 1});
	FlacDecoder &get_parent(){
		return this->flac_parent;
	}
	void set_number_format_hint(NumberFormat nf){}
	OggMetadataWrapper *get_metadata() override{
		return new OggMetadataWrapper(this->flac_parent, this->metadata);
	}
};

void release(AudioBuffer *);

bool is_native_flac(const std::string &path);
