#pragma once

#include "CircularBuffer.h"
#include "FlacDecoder.h"
#include <WrappedExternalIO.hpp>
#include <Decoder.hpp>
#include <memory>
#include <ogg/ogg.h>
#include <boost/optional.hpp>
#include <map>
#include <deque>

class OggFlacDecoderSubstream;

class OggFlacDecoder : public Decoder{
	friend class OggFlacDecoderSubstream;
	WrappedExternalIO io;
	bool seekable;
	ogg_sync_state os;
	boost::optional<std::pair<int, ogg_page>> returned_page;
	std::vector<std::pair<std::int64_t, int>> ogg_streams;
	struct FlacStream{
		int ogg_stream;
		int flac_stream;
		std::int64_t file_offset;
	};
	std::vector<FlacStream> flac_streams;
	std::int64_t length = std::numeric_limits<std::int64_t>::max();
	std::string path;

	//Returns false on EOF.
	bool read_from_input(long read_size);
	void find_all_ogg_streams();
	void find_all_flac_streams();
	std::int64_t find_next_stream(int last_serial, std::int64_t lo, std::int64_t hi, int &serial);
	std::int64_t get_page_boundary(std::int64_t offset);
	std::int64_t get_page_boundary(std::int64_t offset, int &serial);
	std::int64_t final_search(int last_serial, std::int64_t lo, std::int64_t hi, int &serial);
	AudioBuffer *internal_read(const AudioFormat &, size_t extra_data, int substream_index) override{
		return nullptr;
	}
public:
	OggFlacDecoder(const char *path, const ExternalIO &io, Module *module);
	~OggFlacDecoder();
	bool can_seek(){
		return this->seekable;
	}
	boost::optional<ogg_page> read_page(int serial, bool any = false);
	void return_page(const ogg_page &, int serial);
	int get_substream_count() override{
		if (!this->seekable)
			return 1;
		return (int)this->flac_streams.size();
	}
	DecoderSubstream *get_substream(int i) override;
};

class OggFlacDecoderSubstream : public AbstractFlacDecoderSubstream{
	friend class OggFlacDecoder;
	OggFlacDecoder &flac_parent;
	FlacDecoder flac;
	std::unique_ptr<FlacDecoderSubstream> substream;
	int ogg_stream;
	int flac_stream;
	int compound_stream;
	
	static SlicedIO get_io(OggFlacDecoder &decoder, int serial);
	OggFlacDecoderSubstream(OggFlacDecoder &parent, int ogg_stream, int flac_stream, int compound_stream);
public:
	std::int64_t seek_to_sample(std::int64_t pos, bool fast) override{
		return this->substream->seek_to_sample(pos, fast);
	}
	AudioBuffer *read(size_t extra_data) override{
		return this->substream->read(extra_data);
	}
	RationalValue seek_to_second(const RationalValue &pos, bool fast) override{
		return this->substream->seek_to_second(pos, fast);
	}
	OggMetadataWrapper *get_metadata() override{
		return this->substream->get_metadata();
	}
};
