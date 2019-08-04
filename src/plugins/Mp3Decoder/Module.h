#pragma once

#include <unordered_map>
#include <thread_compat.hpp>
#include <string>

class mp3_static_data{
	bool initialized;
public:
	mp3_static_data(): initialized(0){}
	~mp3_static_data();
	bool init();
};

class Module{
	std::mutex mutex;
	std::unordered_map<std::thread::id, std::string> errors;
	bool initialized;
public:
	Module();
	~Module();
	void set_error(const std::string &error);
	const char *get_error();
	void clear_error();
};

const char *mpg123_error_to_string(int error);
