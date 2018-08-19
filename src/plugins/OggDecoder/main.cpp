#include <decoder_module.h>
#include <metadata_module.h>
#include "OggDecoder.h"

#if defined WIN32 || defined _WIN32 || defined _WIN64
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif

EXPORT ModulePtr InitModule(){
	return new Module;
}

EXPORT int GetModuleApiVersion(ModulePtr){
	return 1;
}

EXPORT void DestroyModule(ModulePtr instance){
	delete (Module *)instance;
}

static const char **get_module_types(ModulePtr){
	static const char *table[] = {
		DECODER_MODULE_TYPE,
		METADATA_MODULE_TYPE,
		nullptr,
	};
	return table;
}

static const char *get_module_name(ModulePtr){
	return "OggDecoder";
}

static const char *get_module_version(ModulePtr){
	return "1.0";
}

static const char *get_error_message(ModulePtr instance){
	auto &eh = *(Module *)instance;
	return eh.get_error();
}

const char **decoder_get_supported_extensions(ModulePtr){
	static const char *ret[] = {
		"ogg",
		nullptr,
	};
	return ret;
}

static DecoderPtr decoder_open(ModulePtr instance, const char *path, const ExternalIO *io){
	auto &module = *(Module *)instance;
	try{
		module.clear_error();
		return new OggDecoder(path, *io, &module);
	}catch (std::exception &e){
		module.set_error(e.what());
	}catch (...){}
	return nullptr;
}

static void decoder_close(DecoderPtr instance){
	delete (OggDecoder *)instance;
}

static int decoder_get_substreams_count(DecoderPtr instance){
	auto &decoder = *(OggDecoder *)instance;
	auto &module = decoder.get_module();
	try{
		return decoder.get_substream_count();
	}catch (std::exception &e){
		module.set_error(e.what());
	}
	return -1;
}

static DecoderSubstreamPtr decoder_get_substream(DecoderPtr instance, int substream_index){
	auto &decoder = *(OggDecoder *)instance;
	auto &module = decoder.get_module();
	try{
		return decoder.get_substream(substream_index);
	}catch (std::exception &e){
		module.set_error(e.what());
	}
	return nullptr;
}

static void substream_close(DecoderSubstreamPtr instance){
	delete (OggDecoderSubstream *)instance;
}

AudioFormat substream_get_audio_format(DecoderSubstreamPtr instance){
	auto &substream = *(OggDecoderSubstream *)instance;
	auto &module = substream.get_parent().get_module();
	try{
		return substream.get_audio_format();
	}catch (std::exception &e){
		module.set_error(e.what());
	}
	AudioFormat ret;
	ret.format = Invalid;
	ret.channels = 0;
	ret.freq = 0;
	return ret;
}

void substream_set_number_format_hint(DecoderSubstreamPtr instance, NumberFormat nf){
	auto &substream = *(OggDecoderSubstream *)instance;
	try{
		substream.set_number_format_hint(nf);
	}catch (std::exception &e){
	}
}

static AudioBuffer *substream_read(DecoderSubstreamPtr instance, size_t extra_data){
	auto &substream = *(OggDecoderSubstream *)instance;
	auto &module = substream.get_parent().get_module();
	try{
		return substream.read(extra_data);
	}catch (std::exception &e){
		module.set_error(e.what());
	}
	return nullptr;
}

static RationalValue substream_get_length_in_seconds(DecoderSubstreamPtr instance){
	auto &substream = *(OggDecoderSubstream *)instance;
	auto &module = substream.get_parent().get_module();
	try{
		return substream.get_length_in_seconds();
	}catch (std::exception &e){
		module.set_error(e.what());
	}
	return {-1, 1};
}

static std::uint64_t substream_get_length_in_samples(DecoderSubstreamPtr instance){
	auto &substream = *(OggDecoderSubstream *)instance;
	auto &module = substream.get_parent().get_module();
	try{
		return substream.get_length_in_samples();
	}catch (std::exception &e){
		module.set_error(e.what());
	}
	return -1;
}

static std::int64_t substream_seek_to_sample(DecoderSubstreamPtr instance, int64_t sample, int fast){
	auto &substream = *(OggDecoderSubstream *)instance;
	auto &module = substream.get_parent().get_module();
	try{
		return substream.seek_to_sample(sample, !!fast);
	}catch (std::exception &e){
		module.set_error(e.what());
	}
	return -1;
}

static RationalValue substream_seek_to_second(DecoderSubstreamPtr instance, RationalValue seconds, int fast){
	auto &substream = *(OggDecoderSubstream *)instance;
	auto &module = substream.get_parent().get_module();
	try{
		return substream.seek_to_second(seconds, !!fast);
	}catch (std::exception &e){
		module.set_error(e.what());
	}
	return {-1, 1};
}

MetadataPtr decoder_substream_get_metadata(DecoderSubstreamPtr instance){
	auto &substream = *(OggDecoderSubstream *)instance;
	auto &module = substream.get_parent().get_module();
	try{
		return substream.get_metadata();
	}catch (std::exception &e){
		module.set_error(e.what());
	}
	return nullptr;
}

void metadata_destroy(MetadataPtr instance){
	delete (OggMetadata *)instance;
}

void metadata_release_buffer(MetadataBuffer buffer){
	delete[] buffer.ptr;
}

MetadataBuffer to_MetadataBuffer(const std::string &s){
	MetadataBuffer ret;
	ret.size = s.size();
	size_t size = ret.size ? ret.size : 1;
	ret.ptr = new std::uint8_t[size];
	if (ret.size)
		memcpy(ret.ptr, &s[0], ret.size);
	return ret;
}

MetadataBuffer to_MetadataBuffer(const std::vector<std::uint8_t> &v){
	MetadataBuffer ret;
	ret.size = v.size();
	size_t size = ret.size ? ret.size : 1;
	ret.ptr = new std::uint8_t[size];
	if (ret.size)
		memcpy(ret.ptr, &v[0], ret.size);
	return ret;
}

#define DEFINE_METADATA_BUFFER_GETTER(x) \
	MetadataBuffer metadata_get_##x(MetadataPtr instance){ \
		auto &metadata = *(OggMetadata *)instance; \
		auto &module = metadata.get_decoder().get_module(); \
		try{ \
			return to_MetadataBuffer(metadata.x()); \
		}catch (std::exception &e){ \
			module.set_error(e.what()); \
		} \
		return { nullptr, 0 }; \
	}

#define DEFINE_METADATA_DOUBLE_GETTER(x) \
	double metadata_get_##x(MetadataPtr instance){ \
		auto &metadata = *(OggMetadata *)instance; \
		auto &module = metadata.get_decoder().get_module(); \
		try{ \
			return metadata.x(); \
		}catch (std::exception &e){ \
			module.set_error(e.what()); \
		} \
		return std::nan(""); \
	}

DEFINE_METADATA_BUFFER_GETTER(album)
DEFINE_METADATA_BUFFER_GETTER(track_title)
DEFINE_METADATA_BUFFER_GETTER(track_number)
DEFINE_METADATA_BUFFER_GETTER(track_artist)
DEFINE_METADATA_BUFFER_GETTER(date)
DEFINE_METADATA_BUFFER_GETTER(front_cover)
DEFINE_METADATA_DOUBLE_GETTER(track_gain)
DEFINE_METADATA_DOUBLE_GETTER(track_peak)
DEFINE_METADATA_DOUBLE_GETTER(album_gain)
DEFINE_METADATA_DOUBLE_GETTER(album_peak)

MetadataIteratorPtr metadata_get_iterator(MetadataPtr instance){
	auto &metadata = *(OggMetadata *)instance;
	auto &module = metadata.get_decoder().get_module();
	try{
		return metadata.get_iterator().release();
	}catch (std::exception &e){
		module.set_error(e.what());
	}
	return nullptr;
}

void metadata_iterator_release(MetadataIteratorPtr instance){
	delete (OggMetadataIterator *)instance;
}

int metadata_iterator_get_next(MetadataIteratorPtr instance, MetadataKeyValue *kv){
	auto &iterator = *(OggMetadataIterator *)instance;
	auto &module = iterator.get_decoder().get_module();
	MetadataBuffer mb_key{ nullptr, 0 },
		mb_value{ nullptr, 0 };
	try{
		std::string key, value;
		if (!iterator.next(key, value))
			return 0;
		mb_key = to_MetadataBuffer(key);
		mb_value = to_MetadataBuffer(value);
		kv->key = mb_key;
		kv->value = mb_value;
		return 1;
	}catch (std::exception &e){
		module.set_error(e.what());
		metadata_release_buffer(mb_key);
		metadata_release_buffer(mb_value);
	}
	return -1;
}

#define EXPORT_MODULE_FUNCTION(x) { #x , x }
#define EXPORT_METADATA_GETTER(x) EXPORT_MODULE_FUNCTION(metadata_get_##x)

EXPORT const ModuleExportEntry *GetFunctionTable(ModulePtr){
	static const ModuleExportEntry ret[] = {
		EXPORT_MODULE_FUNCTION(get_module_types),
		EXPORT_MODULE_FUNCTION(get_module_name),
		EXPORT_MODULE_FUNCTION(get_module_version),
		EXPORT_MODULE_FUNCTION(get_error_message),

		EXPORT_MODULE_FUNCTION(decoder_get_supported_extensions),
		EXPORT_MODULE_FUNCTION(decoder_open),
		EXPORT_MODULE_FUNCTION(decoder_close),
		EXPORT_MODULE_FUNCTION(decoder_get_substreams_count),
		EXPORT_MODULE_FUNCTION(decoder_get_substream),
		EXPORT_MODULE_FUNCTION(substream_close),
		EXPORT_MODULE_FUNCTION(substream_get_audio_format),
		EXPORT_MODULE_FUNCTION(substream_set_number_format_hint),
		EXPORT_MODULE_FUNCTION(substream_read),
		EXPORT_MODULE_FUNCTION(substream_get_length_in_seconds),
		EXPORT_MODULE_FUNCTION(substream_get_length_in_samples),
		EXPORT_MODULE_FUNCTION(substream_seek_to_sample),
		EXPORT_MODULE_FUNCTION(substream_seek_to_second),

		EXPORT_MODULE_FUNCTION(decoder_substream_get_metadata),
		EXPORT_MODULE_FUNCTION(metadata_destroy),
		EXPORT_MODULE_FUNCTION(metadata_release_buffer),
		EXPORT_METADATA_GETTER(album),
		EXPORT_METADATA_GETTER(track_title),
		EXPORT_METADATA_GETTER(track_number),
		EXPORT_METADATA_GETTER(track_artist),
		EXPORT_METADATA_GETTER(date),
		EXPORT_METADATA_GETTER(track_gain),
		EXPORT_METADATA_GETTER(track_peak),
		EXPORT_METADATA_GETTER(album_gain),
		EXPORT_METADATA_GETTER(album_peak),
		EXPORT_METADATA_GETTER(front_cover),
		EXPORT_MODULE_FUNCTION(metadata_get_iterator),
		EXPORT_MODULE_FUNCTION(metadata_iterator_release),
		EXPORT_MODULE_FUNCTION(metadata_iterator_get_next),

		{ nullptr, nullptr },
	};
	return ret;
}
