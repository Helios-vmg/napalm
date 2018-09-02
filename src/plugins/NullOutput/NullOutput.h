#pragma once

#include "../common/output.hpp"
#include <mutex>
#include <unordered_map>
#include <thread>
#include <array>
#include <string>
#include <thread>
#include <atomic>

class NullOutput;

class NullDevice : public OutputDevice{
protected:
	static const std::string name;
	NullOutput *parent;
	std::array<std::uint8_t, 32> hash;
	AudioFormat formats[2];
	std::thread thread;
	std::atomic<bool> run;

	void thread_func();
public:
	NullDevice(NullOutput &parent);
	~NullDevice();
	const std::string &get_name() const{
		return name;
	}
	void open(size_t format_index, const AudioCallbackData &callback);
	AudioFormat *get_supported_formats();
	const std::array<std::uint8_t, 32> &get_hash() const{
		return this->hash;
	}
	NullOutput &get_parent(){
		return *this->parent;
	}
	void close();
};

class NullOutput : public Output{
	std::mutex mutex;
	std::unordered_map<std::thread::id, std::string> errors;
	bool initialized = false;
	NullDevice device;

public:
	NullOutput();
	~NullOutput();
	void set_error(const std::string &error);
	const char *get_error();
	void clear_error();
	OutputDeviceList *get_device_list() override;
	AudioFormat *get_supported_formats(const UniqueID &unique_id) override;
	OutputDevice *open_device(const UniqueID &unique_id, size_t format_index, const AudioCallbackData &callback) override;
};
