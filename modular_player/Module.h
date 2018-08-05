#pragma once

#include "../common/module.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#define DEFINE_FP(x) x##_f x

class Module{
	std::string path;
	typedef std::unique_ptr<void, std::function<void(void *)>> system_module_ptr_t;
	system_module_ptr_t system_module_ptr;
	std::unique_ptr<std::remove_pointer<ModulePtr>::type, DestroyModule_f> module;
	std::unordered_map<std::string, GenericFunctionPtr> functions;
	std::unordered_set<std::string> smt;
	int api_version;
	std::string name;
	std::string version;

	DEFINE_FP(get_module_types);
	DEFINE_FP(get_module_name);
	DEFINE_FP(get_module_version);
	DEFINE_FP(get_error_message);

	Module(const std::string &path, system_module_ptr_t &&, InitModule_f, GetModuleApiVersion_f, DestroyModule_f, GetFunctionTable_f);
	void init_module_types();
public:
	static std::shared_ptr<Module> load(const char *path);
	static std::shared_ptr<Module> load(const std::wstring &path);
	const std::unordered_set<std::string> &supported_module_types(){
		return this->smt;
	}
	bool module_supports(const std::string &type) const{
		auto it = this->smt.find(type);
		return it != this->smt.end();
	}
	const std::string &get_name() const{
		return this->name;
	}
	const std::string &get_version() const{
		return this->version;
	}
	void check_error();
	GenericFunctionPtr get_function(const char *name, bool required = true) const;
	ModulePtr get_pointer(){
		return this->module.get();
	}
};
