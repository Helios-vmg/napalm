#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <GenericMetadata.hpp>
#include <metadata_module.h>
#include "Module.h"

class DecoderSubstream;

class MetadataModule{
	friend class ExternalMetadata;
	friend class ExternalMetadataIterator;
	std::shared_ptr<Module> module;

	DEFINE_FP(decoder_substream_get_metadata);
	DEFINE_FP(metadata_destroy);
	DEFINE_FP(metadata_release_buffer);
	DEFINE_FP(metadata_get_album);
	DEFINE_FP(metadata_get_track_title);
	DEFINE_FP(metadata_get_track_number);
	DEFINE_FP(metadata_get_track_artist);
	DEFINE_FP(metadata_get_date);
	DEFINE_FP(metadata_get_track_gain);
	DEFINE_FP(metadata_get_track_peak);
	DEFINE_FP(metadata_get_album_gain);
	DEFINE_FP(metadata_get_album_peak);
	DEFINE_FP(metadata_get_front_cover);
	DEFINE_FP(metadata_get_track_number_int);
	DEFINE_FP(metadata_get_iterator);
	DEFINE_FP(metadata_iterator_release);
	DEFINE_FP(metadata_iterator_get_next);
public:
	MetadataModule(const std::shared_ptr<Module> &module);
	std::unique_ptr<GenericMetadata> get_metadata(DecoderSubstream &);
};

class ExternalMetadataBuffer : public AbstractMetadataBuffer{
	MetadataBuffer buffer;
	metadata_release_buffer_f releaser;
public:
	ExternalMetadataBuffer(){
		this->buffer.ptr = nullptr;
		this->buffer.size = 0;
		this->releaser = nullptr;
	}
	~ExternalMetadataBuffer(){
		if (this->buffer.ptr)
			this->releaser(this->buffer);
	}
	ExternalMetadataBuffer(const MetadataBuffer &buffer, metadata_release_buffer_f releaser){
		this->buffer = buffer;
		this->releaser = releaser;
	}
	ExternalMetadataBuffer(const ExternalMetadataBuffer &other) = delete;
	ExternalMetadataBuffer &operator=(const ExternalMetadataBuffer &other) = delete;
	ExternalMetadataBuffer(ExternalMetadataBuffer &&other){
		*this = std::move(other);
	}
	const ExternalMetadataBuffer &operator=(ExternalMetadataBuffer &&other){
		this->buffer = other.buffer;
		this->releaser = other.releaser;
		other.buffer.ptr = nullptr;
		other.buffer.size = 0;
		other.releaser = nullptr;
		return *this;
	}
	size_t size() const override{
		return this->buffer.size;
	}
	const std::uint8_t *data() const override{
		return this->buffer.ptr;
	}
};

class ExternalMetadata : public GenericMetadata{
	friend class MetadataModule;
	MetadataModule *parent;
	std::unique_ptr<std::remove_pointer<MetadataPtr>::type, metadata_destroy_f> metadata;
	ExternalMetadataBuffer emb_album;
	ExternalMetadataBuffer emb_track_title;
	ExternalMetadataBuffer emb_track_number;
	ExternalMetadataBuffer emb_track_artist;
	ExternalMetadataBuffer emb_date;
	ExternalMetadataBuffer emb_front_cover;

	ExternalMetadata(MetadataModule &parent, DecoderSubstream &substream);
public:
	~ExternalMetadata();
	AbstractMetadataBuffer &album() override;
	AbstractMetadataBuffer &track_title() override;
	AbstractMetadataBuffer &track_number() override;
	AbstractMetadataBuffer &track_artist() override;
	AbstractMetadataBuffer &date() override;
	double track_gain() override;
	double track_peak() override;
	double album_gain() override;
	double album_peak() override;
	AbstractMetadataBuffer &front_cover() override;
	int track_number_int() override;
	std::unique_ptr<AbstractMetadataIterator> get_iterator() override;
};

class ExternalMetadataIterator : public AbstractMetadataIterator{
	friend class ExternalMetadata;
	MetadataModule *module;
	std::unique_ptr<std::remove_pointer<MetadataPtr>::type, metadata_destroy_f> iterator;

	ExternalMetadataIterator(MetadataModule &module, MetadataIteratorPtr);
public:
	bool next(std::string &key, std::string &value) override;
};
