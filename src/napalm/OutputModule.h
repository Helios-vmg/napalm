#pragma once

#include "Module.h"
#include "Decoder.h"
#include "../common/output_module.h"
#include "utility.h"
#include <cstdint>
#include <map>

typedef basic_uniqueid_t<uintptr_t, sizeof(OutputDeviceListItem().unique_id)> uniqueid_t;
class OutputDevice;

class OutputModule{
	friend class OutputDevice;
	std::shared_ptr<Module> module;
	DEFINE_FP(output_get_device_list);
	DEFINE_FP(output_get_supported_formats);
	DEFINE_FP(output_open_device);
	DEFINE_FP(output_device_close);
	DEFINE_FP(output_device_flush);
	DEFINE_FP(output_device_pause);
	DEFINE_FP(output_device_set_volume);
public:
	OutputModule(const std::shared_ptr<Module> &module);
	std::vector<std::shared_ptr<OutputDevice>> get_devices();
	Module &get_module(){
		return *this->module;
	}
};

class OutputDevice{
	friend class OutputModule;
public:
	typedef std::function<size_t (void *, size_t, size_t)> audio_callback_t;
	typedef std::function<void (const AudioFormat &)> audio_format_changed_callback_t;
	typedef std::function<void (const rational_t &)> volume_changed_callback_t;
private:
	OutputModule &parent;
	size_t index;
	std::string name;
	uniqueid_t unique_id;
	std::unique_ptr<std::remove_pointer<OutputDevicePtr>::type, output_device_close_f> device_ptr;
	audio_callback_t audio_callback;
	audio_format_changed_callback_t afc_callback;
	volume_changed_callback_t vc_callback;

	OutputDevice(OutputModule &parent, size_t index, const OutputDeviceListItem &);
	static size_t AudioCallback(void *cb_data, void *dst, size_t size, size_t samples_queued){
		auto This = (OutputDevice *)cb_data;
		return !This->audio_callback ? 0 : This->audio_callback(dst, size, samples_queued);
	}
	static void AudioFormatChangedCallback(void *cb_data, const AudioFormat *af){
		auto This = (OutputDevice *)cb_data;
		if (This->afc_callback)
			This->afc_callback(*af);
	}
	static void VolumeChangedCallback(void *cb_data, RationalValue volume){
		auto This = (OutputDevice *)cb_data;
		if (This->vc_callback)
			This->vc_callback(to_rational(volume));
	}
public:
	OutputDevice(const OutputDevice &) = delete;
	OutputDevice &operator=(const OutputDevice &) = delete;
	OutputDevice(OutputDevice &&) = delete;
	OutputDevice &operator=(OutputDevice &&) = delete;
	void open(size_t format_index, audio_callback_t &&, audio_format_changed_callback_t &&, volume_changed_callback_t &&);
	void close();
	virtual std::vector<AudioFormat> get_preferred_formats();
	void flush();
	void pause(bool pause);
	const std::string &get_name() const{
		return this->name;
	}
	uniqueid_t get_unique_id() const{
		return this->unique_id;
	}
	void set_volume(const rational_t &);
};
