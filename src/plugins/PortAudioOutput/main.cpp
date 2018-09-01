#include <output_module.h>
#include <output.hpp>
#include "PortAudioOutput.h"

#if defined WIN32 || defined _WIN32 || defined _WIN64
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif

EXPORT ModulePtr InitModule(){
	return new PortAudioOutput;
}

EXPORT int GetModuleApiVersion(ModulePtr){
	return 1;
}

EXPORT void DestroyModule(ModulePtr instance){
	delete (PortAudioOutput *)instance;
}

static const char **get_module_types(ModulePtr){
	static const char *table[] = {
		"output",
		nullptr,
	};
	return table;
}

static const char *get_module_name(ModulePtr){
	return "PortAudioOutput";
}

static const char *get_module_version(ModulePtr){
	return "1.0";
}

static const char *get_error_message(ModulePtr instance){
	auto &output = *(PortAudioOutput *)instance;
	return output.get_error();
}

OutputDeviceList *output_get_device_list(ModulePtr instance){
	auto &output = *(PortAudioOutput *)instance;
	try{
		output.clear_error();
		return output.get_device_list();
	}catch (std::exception &e){
		output.set_error(e.what());
	}catch (...){}
	return nullptr;
}

OutputDevicePtr output_open_device(ModulePtr instance, const UniqueID *unique_id, size_t format_index, const AudioCallbackData *callback){
	auto &output = *(PortAudioOutput *)instance;
	try{
		output.clear_error();
		return output.open_device(*unique_id, format_index, *callback);
	}catch (std::exception &e){
		output.set_error(e.what());
	}catch (...){}
	return nullptr;
}

void output_device_close(OutputDevicePtr instance){
	((PortAudioDevice *)instance)->close();
}

AudioFormat *output_get_supported_formats(ModulePtr instance, const UniqueID *unique_id){
	auto &output = *(PortAudioOutput *)instance;
	try{
		output.clear_error();
		return output.get_supported_formats(*unique_id);
	}catch (std::exception &e){
		output.set_error(e.what());
	}catch (...){}
	return nullptr;
}

#define EXPORT_MODULE_FUNCTION(x) { #x , x }

EXPORT const ModuleExportEntry *GetFunctionTable(ModulePtr){
	static const ModuleExportEntry ret[] = {
		EXPORT_MODULE_FUNCTION(get_module_types),
		EXPORT_MODULE_FUNCTION(get_module_name),
		EXPORT_MODULE_FUNCTION(get_module_version),
		EXPORT_MODULE_FUNCTION(get_error_message),

		EXPORT_MODULE_FUNCTION(output_get_device_list),
		EXPORT_MODULE_FUNCTION(output_open_device),
		EXPORT_MODULE_FUNCTION(output_device_close),
		EXPORT_MODULE_FUNCTION(output_get_supported_formats),

		{ nullptr, nullptr },
	};
	return ret;
}
