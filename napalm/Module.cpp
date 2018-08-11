#include "Module.h"
#include "utility.h"
#include <Windows.h>
#include <functional>
#include <algorithm>

#define DEFINE_STRING(x) static const char *x##_string = #x

#define GET_MODULE_REQUIRED_FUNCTION(x) this->x = (x##_f)this->get_function(#x)
#define GET_MODULE_OPTIONAL_FUNCTION(x) this->x = (x##_f)this->get_function(#x, false)


DEFINE_STRING(InitModule);
DEFINE_STRING(GetModuleApiVersion);
DEFINE_STRING(DestroyModule);
DEFINE_STRING(GetFunctionTable);
DEFINE_STRING(get_module_types);
DEFINE_STRING(get_module_name);
DEFINE_STRING(get_module_version);
DEFINE_STRING(get_error_message);

static const int module_api_version = 1;

std::shared_ptr<Module> Module::load(const char *path){
	return load(utf8_to_string(path));
}

std::shared_ptr<Module> Module::load(const std::wstring &path){
	auto module = LoadLibraryW(path.c_str());
	if (!module)
		return nullptr;

	std::unique_ptr<void, std::function<void(void *)>> p((void *)module, [](void *mod){ FreeLibrary((HMODULE)mod); });

#define GET_FUNCTION(x) auto x = (x##_f)GetProcAddress(module, x##_string); \
	if (!x) \
		return nullptr

	GET_FUNCTION(InitModule);
	GET_FUNCTION(GetModuleApiVersion);
	GET_FUNCTION(DestroyModule);
	GET_FUNCTION(GetFunctionTable);

	auto utf8 = string_to_utf8(path);
	try{
		return std::shared_ptr<Module>(new Module(utf8, std::move(p), InitModule, GetModuleApiVersion, DestroyModule, GetFunctionTable));
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"Error initializing module " + utf8 + ": " + e.what());
	}
}


static char toslash(char c){
	return c != '\\' ? c : '/';
}

Module::Module(const std::string &path, system_module_ptr_t &&module, InitModule_f InitModule, GetModuleApiVersion_f GetModuleApiVersion, DestroyModule_f DestroyModule, GetFunctionTable_f GetFunctionTable):
		path(path),
		system_module_ptr(std::move(module)),
		module(nullptr, DestroyModule){

	std::transform(this->path.begin(), this->path.end(), this->path.begin(), toslash);
	
	this->module.reset(InitModule());

	this->api_version = GetModuleApiVersion(this->module.get());
	if (this->api_version < module_api_version)
		throw std::runtime_error("Unsupposed API version.");

	auto table = GetFunctionTable(this->module.get());
	for (; table->function_name; table++)
		this->functions[table->function_name] = table->function_pointer;
	
	GET_MODULE_REQUIRED_FUNCTION(get_module_types);
	GET_MODULE_OPTIONAL_FUNCTION(get_module_name);
	GET_MODULE_OPTIONAL_FUNCTION(get_module_version);
	GET_MODULE_REQUIRED_FUNCTION(get_error_message);

	if (this->get_module_name)
		this->name = this->get_module_name(this->module.get());
	else{
		auto slash = this->path.rfind('/');
		if (slash == this->path.npos)
			slash = 0;
		else
			slash++;
		this->name = this->path.substr(slash);
	}
	if (this->get_module_version)
		this->version = this->get_module_version(this->module.get());
	else
		this->version = "1.0";

	this->init_module_types();
}

void Module::init_module_types(){
	auto get_module_types = (get_module_types_f)this->get_function(get_module_types_string);
	for (auto table = get_module_types(this->module.get()); *table; table++)
		this->smt.insert(*table);
}

GenericFunctionPtr Module::get_function(const char *name, bool required) const{
	auto it = this->functions.find(name);
	if (it == this->functions.end()){
		if (!required)
			return nullptr;
		throw std::runtime_error((std::string)"Module doesn't define required function " + name);
	}
	return it->second;
}

void Module::check_error(){
	auto error = this->get_error_message(this->module.get());
	if (error)
		throw std::runtime_error(error);
}
