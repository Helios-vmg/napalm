#include "MetadataModule.h"
#include "Decoder.h"

std::uint8_t StringMetadataBuffer::static_byte;

MetadataModule::MetadataModule(const std::shared_ptr<Module> &module): module(module){
	GET_REQUIRED_FUNCTION(decoder_substream_get_metadata);
	GET_REQUIRED_FUNCTION(metadata_destroy);
	GET_REQUIRED_FUNCTION(metadata_release_buffer);
	GET_REQUIRED_FUNCTION(metadata_get_album);
	GET_REQUIRED_FUNCTION(metadata_get_track_title);
	GET_REQUIRED_FUNCTION(metadata_get_track_number);
	GET_REQUIRED_FUNCTION(metadata_get_track_artist);
	GET_REQUIRED_FUNCTION(metadata_get_date);
	GET_REQUIRED_FUNCTION(metadata_get_track_gain);
	GET_REQUIRED_FUNCTION(metadata_get_track_peak);
	GET_REQUIRED_FUNCTION(metadata_get_album_gain);
	GET_REQUIRED_FUNCTION(metadata_get_album_peak);
	GET_REQUIRED_FUNCTION(metadata_get_front_cover);
	GET_OPTIONAL_FUNCTION(metadata_get_track_number_int);
	GET_OPTIONAL_FUNCTION(metadata_get_iterator);
	if (this->metadata_get_iterator){
		GET_REQUIRED_FUNCTION(metadata_iterator_release);
		GET_REQUIRED_FUNCTION(metadata_iterator_get_next);
	}else{
		this->metadata_iterator_release = nullptr;
		this->metadata_iterator_get_next = nullptr;
	}
}

std::unique_ptr<GenericMetadata> MetadataModule::get_metadata(DecoderSubstream &substream){
	return std::unique_ptr<ExternalMetadata>(new ExternalMetadata(*this, substream));
}

ExternalMetadata::ExternalMetadata(MetadataModule &parent, DecoderSubstream &substream):
		parent(&parent),
		metadata(nullptr, parent.metadata_destroy){
	this->metadata.reset(parent.decoder_substream_get_metadata(substream.substream_ptr.get()));
}

ExternalMetadata::~ExternalMetadata(){}

#define IMPLEMENT_GETTER(x) \
AbstractMetadataBuffer &ExternalMetadata::x(){ \
	this->emb_##x = ExternalMetadataBuffer( \
		this->parent->metadata_get_##x(this->metadata.get()), \
		this->parent->metadata_release_buffer \
	); \
	return static_cast<AbstractMetadataBuffer &>(this->emb_##x); \
}

IMPLEMENT_GETTER(album)
IMPLEMENT_GETTER(track_title)
IMPLEMENT_GETTER(track_number)
IMPLEMENT_GETTER(track_artist)
IMPLEMENT_GETTER(date)
IMPLEMENT_GETTER(front_cover)

double ExternalMetadata::track_gain(){
	return this->parent->metadata_get_track_gain(this->metadata.get());
}

double ExternalMetadata::track_peak(){
	return this->parent->metadata_get_track_peak(this->metadata.get());
}

double ExternalMetadata::album_gain(){
	return this->parent->metadata_get_album_gain(this->metadata.get());
}
double ExternalMetadata::album_peak(){
	return this->parent->metadata_get_album_peak(this->metadata.get());
}

int ExternalMetadata::track_number_int(){
	auto f = this->parent->metadata_get_track_number_int;
	if (!f)
		return this->GenericMetadata::track_number_int();
	return f(this->metadata.get());
}

std::unique_ptr<AbstractMetadataIterator> ExternalMetadata::get_iterator(){
	auto f = this->parent->metadata_get_iterator;
	if (!f)
		return this->GenericMetadata::get_iterator();
	
	return std::unique_ptr<AbstractMetadataIterator>(new ExternalMetadataIterator(*this->parent, f(this->metadata.get())));
}

ExternalMetadataIterator::ExternalMetadataIterator(MetadataModule &module, MetadataIteratorPtr it):
		module(&module),
		iterator(it, module.metadata_iterator_release){
}

bool ExternalMetadataIterator::next(std::string &key, std::string &value){
	MetadataKeyValue kv;
	auto result = this->module->metadata_iterator_get_next(this->iterator.get(), &kv);
	if (!result)
		return false;
	if (result > 0){
		try{
			key.assign((const char *)kv.key.ptr, kv.key.size);
			value.assign((const char *)kv.value.ptr, kv.value.size);
		}catch (std::exception &){
			this->module->metadata_release_buffer(kv.key);
			this->module->metadata_release_buffer(kv.value);
			throw;
		}
		return true;
	}
	this->module->module->check_error();
	return false;
}
