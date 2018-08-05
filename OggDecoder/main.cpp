#include "../common/decoder_module.h"
#include "OggDecoder.h"

#ifdef WIN32
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
		"decoder",
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

static void decoder_release_ReadResult(ReadResult *p){
	release(p);
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

static ReadResult *substream_read(DecoderSubstreamPtr instance){
	auto &substream = *(OggDecoderSubstream *)instance;
	auto &module = substream.get_parent().get_module();
	try{
		return substream.read();
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

#define EXPORT_MODULE_FUNCTION(x) { #x , x }

EXPORT const ModuleExportEntry *GetFunctionTable(ModulePtr){
	static const ModuleExportEntry ret[] = {
		EXPORT_MODULE_FUNCTION(get_module_types),
		EXPORT_MODULE_FUNCTION(get_module_name),
		EXPORT_MODULE_FUNCTION(get_module_version),
		EXPORT_MODULE_FUNCTION(get_error_message),

		EXPORT_MODULE_FUNCTION(decoder_get_supported_extensions),
		EXPORT_MODULE_FUNCTION(decoder_open),
		EXPORT_MODULE_FUNCTION(decoder_close),
		EXPORT_MODULE_FUNCTION(decoder_release_ReadResult),
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

		{ nullptr, nullptr },
	};
	return ret;
}
