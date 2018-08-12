#include "../common/output_module.h"
#include "../common/output.hpp"
#include "WasapiOutput.h"

#if defined WIN32 || defined _WIN32 || defined _WIN64
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif

EXPORT ModulePtr InitModule(){
	return new WasapiOutput;
}

EXPORT int GetModuleApiVersion(ModulePtr){
	return 1;
}

EXPORT void DestroyModule(ModulePtr instance){
	delete (WasapiOutput *)instance;
}

static const char **get_module_types(ModulePtr){
	static const char *table[] = {
		"output",
		nullptr,
	};
	return table;
}

static const char *get_module_name(ModulePtr){
	return "WasapiOutput";
}

static const char *get_module_version(ModulePtr){
	return "1.0";
}

static const char *get_error_message(ModulePtr instance){
	auto &wasapi = *(WasapiOutput *)instance;
	return wasapi.get_error();
}

OutputDeviceList *output_get_device_list(ModulePtr instance){
	auto &wasapi = *(WasapiOutput *)instance;
	try{
		wasapi.clear_error();
		return wasapi.get_device_list();
	}catch (std::exception &e){
		wasapi.set_error(e.what());
	}catch (...){}
	return nullptr;
}

OutputDevicePtr output_open_device(ModulePtr instance, size_t index, size_t format_index, const AudioCallbackData *callback){
	auto &wasapi = *(WasapiOutput *)instance;
	try{
		wasapi.clear_error();
		return wasapi.open_device(index, format_index, *callback);
	}catch (std::exception &e){
		wasapi.set_error(e.what());
	}catch (...){}
	return nullptr;
}

void output_device_close(OutputDevicePtr instance){
	((BaseWasapiOutputDevice *)instance)->close();
}

AudioFormat *output_get_supported_formats(ModulePtr instance, size_t index){
	auto &wasapi = *(WasapiOutput *)instance;
	try{
		wasapi.clear_error();
		return wasapi.get_supported_formats(index);
	}catch (std::exception &e){
		wasapi.set_error(e.what());
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
