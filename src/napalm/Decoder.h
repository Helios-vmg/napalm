#pragma once

#include "InputStream.h"
#include "Module.h"
#include "BufferSource.h"
#include "utility.h"
#include <memory>
#include <unordered_set>
#include <string>
#include <boost/optional.hpp>
#include <atomic>

class InputStream;
class DecoderSubstream;
class GenericMetadata;

namespace BufferExtraData_flags{
static const std::uint64_t seek_complete = (std::uint64_t)1 << 0;
}

struct BufferExtraData{
	RationalValue timestamp;
	AudioBuffer *next;
	size_t sample_offset;
	std::uint64_t stream_id;
	std::uint64_t flags;
};

class MetadataModule;

class DecoderModule{
	friend class Decoder;
	friend class DecoderSubstream;
	std::shared_ptr<Module> module;
	std::shared_ptr<MetadataModule> metadata_module;
	std::unordered_set<std::string> supported_extensions;

	DEFINE_FP(decoder_get_supported_extensions);
	DEFINE_FP(decoder_open);
	DEFINE_FP(decoder_close);
	DEFINE_FP(decoder_get_substreams_count);
	DEFINE_FP(decoder_get_substream);
	DEFINE_FP(substream_close);
	DEFINE_FP(substream_get_audio_format);
	DEFINE_FP(substream_set_number_format_hint);
	DEFINE_FP(substream_read);
	DEFINE_FP(substream_get_length_in_seconds);
	DEFINE_FP(substream_get_length_in_samples);
	DEFINE_FP(substream_seek_to_sample);
	DEFINE_FP(substream_seek_to_second);
public:
	DecoderModule(const std::shared_ptr<Module> &module);
	~DecoderModule();
	const std::unordered_set<std::string> &get_supported_extensions() const{
		return this->supported_extensions;
	}
	std::shared_ptr<Decoder> open(std::unique_ptr<InputStream> &&stream);
};

class Decoder{
	friend class DecoderModule;
	friend class DecoderSubstream;
	DecoderModule &module;
	std::unique_ptr<std::remove_pointer<DecoderPtr>::type, decoder_close_f> decoder_ptr;
	std::unique_ptr<InputStream> stream;
	std::shared_ptr<MetadataModule> metadata_module;

	Decoder(DecoderModule &module, std::unique_ptr<InputStream> &&stream, const std::shared_ptr<MetadataModule> &metadata_module);
public:
	int get_substreams_count();
	std::unique_ptr<DecoderSubstream> get_substream(int index);
	const InputStream &get_stream() const{
		return *this->stream;
	}
};

class DecoderSubstream : public BufferSource{
	friend class Decoder;
	friend class ExternalMetadata;
	DecoderModule &module;
	std::unique_ptr<std::remove_pointer<DecoderSubstreamPtr>::type, substream_close_f> substream_ptr;
	std::shared_ptr<MetadataModule> metadata_module;
	boost::optional<AudioFormat> format;
	rational_t current_time = {0, 1};
	rational_t length;
	static std::atomic<std::uint64_t> next_stream_id;
	std::uint64_t stream_id;
	
	DecoderSubstream(Decoder &decoder, int index, const std::shared_ptr<MetadataModule> &metadata_module);
public:
	AudioFormat get_audio_format();
	void set_number_format_hint(NumberFormat);
	audio_buffer_t read() override;
	rational_t get_length_in_seconds();
	std::uint64_t get_length_in_samples();
	std::int64_t seek_to_sample(std::int64_t sample, bool fast);
	rational_t seek_to_second(const rational_t &time, bool fast);
	std::unique_ptr<BufferSource> unroll_chain() override{
		return nullptr;
	}
	std::uint64_t get_stream_id() const{
		return this->stream_id;
	}
	std::unique_ptr<GenericMetadata> get_metadata();
	DecoderSubstream &get_first_source() override{
		return *this;
	}
};

void release_buffer(AudioBuffer *rr);
audio_buffer_t allocate_buffer(size_t byte_size, size_t extra_data);
audio_buffer_t allocate_buffer_by_clone(const audio_buffer_t &, size_t byte_size = 0);

template <typename T>
T &get_extra_data(AudioBuffer *buffer){
	if (sizeof(T) > buffer->extra_data_size)
		throw std::exception();
	return *(T *)(buffer->data + buffer->samples_size);
}

template <typename T>
T &get_extra_data(audio_buffer_t &buffer){
	return get_extra_data<T>(buffer.get());
}
