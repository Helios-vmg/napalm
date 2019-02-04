#pragma once

#include <unordered_map>
#include <thread_compat.hpp>
#include <string>

class Module{
	std::mutex mutex;
	std::unordered_map<std::thread::id, std::string> errors;
public:
	void set_error(const std::string &error);
	const char *get_error();
	void clear_error();
};
