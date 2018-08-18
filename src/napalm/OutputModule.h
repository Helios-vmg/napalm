#pragma once

#include "Module.h"
#include "Decoder.h"
#include "../common/output_module.h"
#include "utility.h"
#include <cstdint>
#include <map>

typedef basic_uniqueid_t<uintptr_t, sizeof(OutputDeviceListItem().unique_id)> uniqueid_t;

class OutputModule{
	friend class OutputDevice;
	std::shared_ptr<Module> module;
	std::map<uniqueid_t, std::shared_ptr<OutputDevice>> devices;
	DEFINE_FP(output_get_device_list);
	DEFINE_FP(output_get_supported_formats);
	DEFINE_FP(output_open_device);
	DEFINE_FP(output_device_close);
	DEFINE_FP(output_device_flush);
	DEFINE_FP(output_device_pause);

	void init_devices();
public:
	OutputModule(const std::shared_ptr<Module> &module);
	const auto &get_devices() const{
		return this->devices;
	}
};

class OutputDevice{
	friend class OutputModule;
public:
	typedef std::function<size_t (void *, size_t, size_t)> audio_callback_t;
	typedef std::function<void (const AudioFormat &)> notification_callback_t;
private:
	OutputModule &parent;
	size_t index;
	std::string name;
	uniqueid_t unique_id;
	std::unique_ptr<std::remove_pointer<OutputDevicePtr>::type, output_device_close_f> device_ptr;
	audio_callback_t audio_callback;
	notification_callback_t notification_callback;

	OutputDevice(OutputModule &parent, size_t index, const OutputDeviceListItem &);
	static size_t AudioCallback(void *cb_data, void *dst, size_t size, size_t samples_queued){
		return ((OutputDevice *)cb_data)->audio_callback(dst, size, samples_queued);
	}
	static void AudioFormatChangedCallback(void *cb_data, const AudioFormat *af){
		((OutputDevice *)cb_data)->notification_callback(*af);
	}
public:
	OutputDevice(const OutputDevice &) = delete;
	OutputDevice &operator=(const OutputDevice &) = delete;
	OutputDevice(OutputDevice &&) = delete;
	OutputDevice &operator=(OutputDevice &&) = delete;
	void open(size_t format_index, audio_callback_t &&, notification_callback_t &&);
	void close();
	virtual std::vector<AudioFormat> get_preferred_formats();
	void flush();
	void pause(bool pause);
};
