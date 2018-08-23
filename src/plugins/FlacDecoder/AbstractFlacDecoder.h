#pragma once

#include <Decoder.hpp>
#include "../OggMetadata/OggMetadata.h"

class FlacDecoder;

struct OggMetadataWrapper{
	FlacDecoder *decoder;
	OggMetadata metadata;
	OggMetadataWrapper(FlacDecoder &decoder, const OggMetadata &metadata):
		decoder(&decoder),
		metadata(metadata){}
};

class AbstractFlacDecoderSubstream : public DecoderSubstream{
public:
	AbstractFlacDecoderSubstream(
		Decoder &parent,
		int index = 0,
		std::uint64_t first_sample = 0,
		std::uint64_t length = std::numeric_limits<std::uint64_t>::max(),
		const rational_t &first_moment = 0,
		const rational_t &seconds_length = {-1, 1}
	): DecoderSubstream(parent, index, first_sample, length, first_moment, seconds_length){}
	virtual ~AbstractFlacDecoderSubstream(){}
	virtual OggMetadataWrapper *get_metadata() = 0;
};
