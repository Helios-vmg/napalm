#include "FileOutput.h"
#include <utf8.hpp>
#include <sha256.hpp>
#include <sstream>
#include <vector>
#include <iostream>

const std::string FileDevice::name = "File Output Device";

FileOutput::FileOutput(): device(*this){}

FileOutput::~FileOutput(){}

void FileOutput::set_error(const std::string &error){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	this->errors[id] = error;
}

const char *FileOutput::get_error(){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	auto it = this->errors.find(id);
	if (it == this->errors.end())
		return nullptr;
	return it->second.c_str();
}

void FileOutput::clear_error(){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	auto it = this->errors.find(id);
	if (it == this->errors.end())
		return;
	this->errors.erase(it);
}


class FileOutputDeviceList{
	std::vector<std::string> strings;
	std::vector<OutputDeviceListItem> items;
	OutputDeviceList odl;

	static void release(void *This){
		delete (FileOutputDeviceList *)This;
	}
public:
	FileOutputDeviceList(const FileDevice &device){
		this->strings.push_back(device.get_name());
		this->items.resize(1);
		auto &back = this->items.back();
		back.name = this->strings.back().c_str();
		memcpy(back.unique_id.unique_id, device.get_hash().data(), sizeof(back.unique_id.unique_id));
		
		this->odl.opaque = this;
		this->odl.release_function = release;
		this->odl.length = 1;
		this->odl.items = &this->items[0];
	}
	OutputDeviceList *get_list(){
		return &this->odl;
	}
};

OutputDeviceList *FileOutput::get_device_list(){
	auto list = new FileOutputDeviceList(this->device);
	return list->get_list();
}

AudioFormat *FileOutput::get_supported_formats(const UniqueID &unique_id){
	return this->device.get_supported_formats();
}

OutputDevice *FileOutput::open_device(const UniqueID &unique_id, size_t format_index, const AudioCallbackData &callback){
	this->device.open(format_index, callback);
	return &this->device;
}

FileDevice::FileDevice(FileOutput &parent): parent(&parent), run(false){
	SHA256 hash;
	hash.input(this->name);
	this->hash = hash.result();
	this->formats[0] = {IntS16, 2, 44100};
	this->formats[1] = {Invalid, 0, 0};
	this->file.open("output.raw", std::ios::trunc | std::ios::binary);
	if (!this->file)
		throw std::runtime_error("Couldn't open output file");
}

FileDevice::~FileDevice(){
	this->close();
}

void FileDevice::close(){
	if (!this->thread.joinable())
		return;
	this->run = false;
	this->thread.join();
	this->thread = std::thread();
	this->file.close();
}

void FileDevice::open(size_t format_index, const AudioCallbackData &callback){
	if (format_index > 0)
		throw std::runtime_error("Invalid format index.");

	this->callback = callback;
	this->run = true;
	this->thread = std::thread([this](){ this->thread_func(); });
}

void FileDevice::thread_func(){
	auto initial = std::chrono::high_resolution_clock::now();
	auto bytes_per_sample = sizeof_NumberFormat(this->formats[0].format) * this->formats[0].channels;
	std::vector<char> bitbucket;
	while (this->run){
		const double milliseconds = 50.0;
		const auto samples = (std::uint64_t)(milliseconds * (44100.0 / 1000.0) + 0.5);
		auto bytes_to_read = samples * bytes_per_sample;
		if (bytes_to_read){
			if (bitbucket.size() < bytes_to_read)
				bitbucket.resize(bytes_to_read);
			auto samples_read = this->callback.audio_callback(this->callback.cb_data, &bitbucket[0], samples, 0);
			this->total_samples_written += samples_read;
			this->file.write(&bitbucket[0], samples_read * bytes_per_sample);
		}
	}
}

AudioFormat *FileDevice::get_supported_formats(){
	return this->formats;
}
