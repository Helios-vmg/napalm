#pragma once

#include "../common/output.hpp"
#include <mutex>
#include <unordered_map>
#include <thread>
#include <array>
#include <string>
#include <objbase.h>
#include <comdef.h>
#pragma warning(push)
#pragma warning(disable : 4201)
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#pragma warning(pop)
#include <functiondiscoverykeys.h>
#include <thread>
#include <atomic>

template <class T>
void com_release(T *ppT){
	if (!ppT)
		return;
    ppT->Release();
}

template <typename T>
using releaser = void (*)(T *);

template <typename T>
using com_unique = std::unique_ptr<T, releaser<T>>;

template <typename T>
using com_shared = std::shared_ptr<T>;

template <typename T>
com_unique<T> to_unique(T *p, releaser<T> f = com_release<T>){
	return {p, f};
}

template <typename T>
com_shared<T> to_shared(T *p, releaser<T> f = com_release<T>){
	return {p, f};
}

class BaseWasapiOutputDevice;

class WasapiOutput : public Output{
	std::mutex mutex;
	std::unordered_map<std::thread::id, std::string> errors;
	bool initialized = false;
	bool com_initialized = false;
	bool devices_initialized = false;
	std::vector<std::unique_ptr<BaseWasapiOutputDevice>> devices;

	void init_device_list();
public:
	WasapiOutput();
	~WasapiOutput();
	void set_error(const std::string &error);
	const char *get_error();
	void clear_error();
	OutputDeviceList *get_device_list() override;
	AudioFormat *get_supported_formats(size_t index) override;
	OutputDevice *open_device(size_t index, size_t format_index, const AudioCallbackData &callback) override;
};

class BaseWasapiOutputDevice : public OutputDevice{
protected:
	WasapiOutput *parent;
	com_shared<IMMDeviceEnumerator> enumerator;
	std::array<std::uint8_t, 32> hash;
	std::vector<AudioFormat> formats;
	typedef std::unique_ptr<WAVEFORMATEX, void (*)(WAVEFORMATEX *)> exformat_t;
	std::vector<exformat_t> exformats;

	void set_format(WAVEFORMATEX *, size_t size);
public:
	BaseWasapiOutputDevice(WasapiOutput &parent, const com_shared<IMMDeviceEnumerator> &enumerator):
		parent(&parent),
		enumerator(enumerator){}
	virtual const std::string &get_name() = 0;
	virtual void open(size_t format_index, const AudioCallbackData &callback) = 0;
	virtual AudioFormat *get_supported_formats() = 0;
	const std::array<std::uint8_t, 32> &get_hash() const{
		return this->hash;
	}
	WasapiOutput &get_parent(){
		return *this->parent;
	}
	virtual void close() = 0;
};

typedef std::unique_ptr<typename std::remove_pointer<HANDLE>::type, void (*)(HANDLE)> unique_handle_t;
typedef std::shared_ptr<typename std::remove_pointer<HANDLE>::type> shared_handle_t;

unique_handle_t create_event(bool auto_reset);

class SessionNotifier;

class SpecificWasapiOutputDevice : public BaseWasapiOutputDevice{
protected:
	com_unique<IMMDevice> device;
	com_unique<IAudioClient> audio_client;
    com_unique<IAudioRenderClient> render_client;
    com_unique<IAudioSessionControl> session_control;
	com_unique<SessionNotifier> notifier;
	std::string name;
	std::string id;
	size_t buffer_size = 0; // In samples.
	AudioFormat selected_format = {Invalid, 0, 0};
	unique_handle_t samples_event = create_event(true);
	unique_handle_t stop_event = create_event(true);
	shared_handle_t stream_switch_event = create_event(true);
	shared_handle_t stream_switch_complete_event = create_event(false);
	size_t bytes_per_sample = 0;
	std::thread thread;
	std::shared_ptr<std::atomic<bool>> switching_streams = std::make_shared<std::atomic<bool>>(false);
	size_t format_index = 0;
	std::atomic<bool> stopping = false;

	SpecificWasapiOutputDevice(WasapiOutput &parent, const com_shared<IMMDeviceEnumerator> &enumerator);
	void set_vars();
	com_unique<IPropertyStore> get_property_store();
	void set_format(IPropertyStore &);
	void set_client();
	void thread_func();
	void check_events();
	void get_more_samples();
	void initialize_audio_engine();
	virtual std::vector<HANDLE> get_events();
	virtual void initialize_stream_switching(){}
	virtual bool handle_event(DWORD);
	void start_client();
public:
	SpecificWasapiOutputDevice(WasapiOutput &parent, const com_shared<IMMDeviceEnumerator> &enumerator, com_unique<IMMDevice> &&);
	~SpecificWasapiOutputDevice();
	AudioFormat *get_supported_formats(){
		return &this->formats[0];
	}
	virtual const std::string &get_name() override{
		return this->name;
	}
	virtual void open(size_t format_index, const AudioCallbackData &callback) override;
	virtual void close() override;
};

class DefaultWasapiOutputDevice : public SpecificWasapiOutputDevice{
	ERole role;

	bool switch_streams();
	void initialize_stream_switching() override;
	std::vector<HANDLE> get_events() override;
	bool handle_event(DWORD) override;
public:
	DefaultWasapiOutputDevice(WasapiOutput &parent, const com_shared<IMMDeviceEnumerator> &enumerator, ERole role);
};
