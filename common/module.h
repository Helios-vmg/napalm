#pragma once

typedef void *ModulePtr;

typedef void *GenericFunctionPtr;

typedef struct{
	const char *function_name;
	GenericFunctionPtr function_pointer;
} ModuleExportEntry;



//Export name: InitModule
typedef ModulePtr (*InitModule_f)();

//Export name: GetModuleApiVersion
typedef int (*GetModuleApiVersion_f)(ModulePtr);

//Export name: DestroyModule
typedef void (*DestroyModule_f)(ModulePtr);

//Export name: GetFunctionTable
typedef const ModuleExportEntry *(*GetFunctionTable_f)(ModulePtr);



//get_module_types (required)
typedef const char **(*get_module_types_f)(ModulePtr);

//get_module_name (optional)
typedef const char *(*get_module_name_f)(ModulePtr);

//get_module_version (optional)
typedef const char *(*get_module_version_f)(ModulePtr);

//get_error_message (optional)
typedef const char *(*get_error_message_f)(ModulePtr);
