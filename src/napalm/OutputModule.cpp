#include "OutputModule.h"

OutputModule::OutputModule(const std::shared_ptr<Module> &module): module(module){
	GET_REQUIRED_FUNCTION(output_get_device_list);
	GET_REQUIRED_FUNCTION(output_get_supported_formats);
	GET_REQUIRED_FUNCTION(output_open_device);
	GET_REQUIRED_FUNCTION(output_device_close);
	GET_OPTIONAL_FUNCTION(output_device_flush);
	GET_OPTIONAL_FUNCTION(output_device_pause);

	this->init_devices();
}

void OutputModule::init_devices(){
	std::unique_ptr<OutputDeviceList, void(*)(OutputDeviceList *)> list(
		this->output_get_device_list(this->module->get_pointer()),
		[](OutputDeviceList *p){ if (p) p->release_function(p->opaque); }
	);
	for (size_t i = 0; i < list->length; i++){
		auto &item = list->items[i];
		this->devices[item.unique_id] = std::shared_ptr<OutputDevice>(new OutputDevice(*this, i, item));
	}
}

OutputDevice::OutputDevice(OutputModule &parent, size_t index, const OutputDeviceListItem &item):
		parent(parent),
		index(index),
		name(item.name),
		unique_id(item.unique_id),
		device_ptr(
			nullptr,
			this->parent.output_device_close
		){
}

void OutputDevice::open(size_t format_index, audio_callback_t &&ac, notification_callback_t &&nc){
	if (this->device_ptr)
		return;
	this->audio_callback = std::move(ac);
	this->notification_callback = std::move(nc);
	AudioCallbackData acd;
	acd.cb_data = this;
	acd.audio_callback = this->audio_callback ? AudioCallback : nullptr;
	acd.format_changed = this->notification_callback ? AudioFormatChangedCallback : nullptr;
	auto dev = this->parent.output_open_device(this->parent.module->get_pointer(), this->index, format_index, &acd);
	if (!dev)
		this->parent.module->check_error();
	this->device_ptr.reset(dev);
}

void OutputDevice::close(){
	this->device_ptr.reset();
}

std::vector<AudioFormat> OutputDevice::get_preferred_formats(){
	auto list = this->parent.output_get_supported_formats(this->parent.module->get_pointer(), this->index);
	if (!valid_format(*list))
		this->parent.module->check_error();
	std::vector<AudioFormat> ret;
	for (auto i = list; valid_format(*i); i++)
		ret.push_back(*i);
	return ret;
}

void OutputDevice::flush(){
	auto f = this->parent.output_device_flush;
	if (f)
		f(this->device_ptr.get());
}
	
void OutputDevice::pause(bool pause){
	auto f = this->parent.output_device_pause;
	if (f)
		f(this->device_ptr.get(), pause);
}
