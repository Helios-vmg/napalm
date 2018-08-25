#pragma once

#include "../common/decoder.hpp"
#include "../OggMetadata/OggMetadata.h"
#include "Module.h"
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

class OggDecoder : public Decoder{
	WrappedExternalIO io;
	Module *module;
	std::string path;
	OggVorbis_File ogg_file;
	bool seekable;
	int stream_count;

	AudioBuffer *internal_read(const AudioFormat &, size_t extra_data, int substream_index) override;
	void init_time_resolution();
	int ov_read(const AudioFormat &af, std::uint8_t *dst, size_t size, size_t &samples, int substream_index);
	std::int64_t seek_to_sample_internal(std::int64_t pos, bool fast) override;
public:
	OggDecoder(const char *path, const ExternalIO &io, Module *module);
	~OggDecoder();
	int get_substream_count() override{
		return this->stream_count;
	}
	DecoderSubstream *get_substream(int index) override;
	OggVorbis_File *get_ogg_file(){
		return &this->ogg_file;
	}
	const std::string &get_path() const{
		return this->path;
	}
	Module &get_module(){
		return *this->module;
	}

	static size_t read(void *buffer, size_t size, size_t nmemb, void *s);
	static int seek(void *s, ogg_int64_t offset, int whence);
	static long tell(void *s);
};

struct OggMetadataWrapper{
	OggDecoder *decoder;
	OggMetadata metadata;
	OggMetadataWrapper(OggDecoder &decoder, const OggMetadata &metadata):
		decoder(&decoder),
		metadata(metadata){}
};

class OggDecoderSubstream : public DecoderSubstream{
	OggDecoder &ogg_parent;
	OggMetadata metadata;
public:
	OggDecoderSubstream(
		OggDecoder &parent,
		int index,
		std::int64_t first_sample = 0,
		std::int64_t length = std::numeric_limits<std::uint64_t>::max(),
		const rational_t &first_moment = 0,
		const rational_t &seconds_length = {-1, 1});
	OggDecoder &get_parent(){
		return this->ogg_parent;
	}
	OggMetadataWrapper *get_metadata(){
		return new OggMetadataWrapper(this->ogg_parent, this->metadata);
	}
};

void release(AudioBuffer *);
