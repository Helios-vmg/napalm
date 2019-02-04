#pragma once

#include <portaudio.h>
#include "../common/output.hpp"
#include <unordered_map>
#include <thread_compat.hpp>
#include <array>
#include <string>
#include <atomic>

class PortAudioDeviceList;

class PortAudioOutput : public Output{
	std::mutex mutex;
	std::unordered_map<std::thread::id, std::string> errors;
	bool initialized = false;
	std::unique_ptr<PortAudioDeviceList> device_list;

public:
	PortAudioOutput();
	~PortAudioOutput();
	void set_error(const std::string &error);
	const char *get_error();
	void clear_error();
	OutputDeviceList *get_device_list() override;
	AudioFormat *get_supported_formats(const UniqueID &unique_id) override;
	OutputDevice *open_device(const UniqueID &unique_id, size_t format_index, const AudioCallbackData &callback) override;
};

class PortAudioDevice : public OutputDevice{
protected:
	PortAudioOutput *parent;
	PaDeviceIndex index;
	AudioFormat format;
	PaStream *stream = nullptr;
	size_t bytes_per_sample;

	static int audio_callback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);
public:
	PortAudioDevice(PortAudioOutput &parent, const AudioCallbackData &callback, PaDeviceIndex index, const AudioFormat &format);
	~PortAudioDevice();
	PortAudioOutput &get_parent(){
		return *this->parent;
	}
	void close();
};
