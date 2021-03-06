#pragma once

#include "../common/output.hpp"
#include <unordered_map>
#include <thread_compat.hpp>
#include <array>
#include <string>
#include <atomic>
#include <fstream>

class FileOutput;

class FileDevice : public OutputDevice{
protected:
	static const std::string name;
	FileOutput *parent;
	std::array<std::uint8_t, 32> hash;
	AudioFormat formats[2];
	std::thread thread;
	std::atomic<bool> run;
	std::ofstream file;
	std::uint64_t total_samples_written = 0;

	void thread_func();
public:
	FileDevice(FileOutput &parent);
	~FileDevice();
	const std::string &get_name() const{
		return name;
	}
	void open(size_t format_index, const AudioCallbackData &callback);
	AudioFormat *get_supported_formats();
	const std::array<std::uint8_t, 32> &get_hash() const{
		return this->hash;
	}
	FileOutput &get_parent(){
		return *this->parent;
	}
	void close();
};

class FileOutput : public Output{
	std::mutex mutex;
	std::unordered_map<std::thread::id, std::string> errors;
	bool initialized = false;
	FileDevice device;

public:
	FileOutput();
	~FileOutput();
	void set_error(const std::string &error);
	const char *get_error();
	void clear_error();
	OutputDeviceList *get_device_list() override;
	AudioFormat *get_supported_formats(const UniqueID &unique_id) override;
	OutputDevice *open_device(const UniqueID &unique_id, size_t format_index, const AudioCallbackData &callback) override;
};
