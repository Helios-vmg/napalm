#pragma once

#include "output_module.h"

class OutputDevice;

class Output{
public:
	virtual ~Output(){}
	virtual OutputDeviceList *get_device_list() = 0;
	virtual OutputDevice *open_device(size_t index, size_t format_index, const AudioCallbackData &callback) = 0;
	virtual AudioFormat *get_supported_formats(size_t index) = 0;
};

class OutputDevice{
protected:
	AudioCallbackData callback;

	size_t get_audio(void *dst, size_t size, size_t samples){
		return !this->callback.audio_callback ? 0 : this->callback.audio_callback(this->callback.cb_data, dst, size, samples);
	}
	void notify_client(const AudioFormat *af){
		if (this->callback.format_changed)
			this->callback.format_changed(this->callback.cb_data, af);
	}
public:
	OutputDevice():
		callback{nullptr, nullptr, nullptr}{}
	OutputDevice(const AudioCallbackData &callback):
		callback(callback){}
	virtual ~OutputDevice(){}
};
