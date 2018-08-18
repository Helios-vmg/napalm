#include "Module.h"

void Module::set_error(const std::string &error){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	this->errors[id] = error;
}

const char *Module::get_error(){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	auto it = this->errors.find(id);
	if (it == this->errors.end())
		return nullptr;
	return it->second.c_str();
}

void Module::clear_error(){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	auto it = this->errors.find(id);
	if (it == this->errors.end())
		return;
	this->errors.erase(it);
}
