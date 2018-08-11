#include "WasapiOutput.h"
#include "../common/utf8.hpp"
#include "../common/sha256.hpp"
#include <sstream>
#include <avrt.h>
#include <iostream>

WasapiOutput::WasapiOutput(){}

WasapiOutput::~WasapiOutput(){
	for (size_t i = 0; i < this->devices.size(); i++)
		this->devices[i].reset();
}

void WasapiOutput::set_error(const std::string &error){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	this->errors[id] = error;
}

const char *WasapiOutput::get_error(){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	auto it = this->errors.find(id);
	if (it == this->errors.end())
		return nullptr;
	return it->second.c_str();
}

void WasapiOutput::clear_error(){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	auto it = this->errors.find(id);
	if (it == this->errors.end())
		return;
	this->errors.erase(it);
}

std::string to_string(HRESULT result){
	return string_to_utf8(_com_error(result).ErrorMessage());
}

template <typename T>
void cotaskmemfree(T *p){
	CoTaskMemFree(p);
}

void release_PROPVARIANT(PROPVARIANT *p){
	PropVariantClear(p);
}

void WasapiOutput::init_device_list(){
	if (this->devices_initialized)
		return;
	auto result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (SUCCEEDED(result))
		this->com_initialized = true;
	else{
		if (result != CO_E_ALREADYINITIALIZED)
			throw std::runtime_error("COM initialization failed: " + to_string(result));
	}

    IMMDeviceEnumerator *device_enumerator_ptr = nullptr;
	result = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&device_enumerator_ptr));
	if (!SUCCEEDED(result))
		throw std::runtime_error("Failed to create MMDeviceEnumerator: " + to_string(result));
	auto enumerator = to_shared(device_enumerator_ptr);

    IMMDeviceCollection *device_collection_ptr = nullptr;
	result = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &device_collection_ptr);
	if (!SUCCEEDED(result))
		throw std::runtime_error("Failed to get device enumeration: " + to_string(result));
	auto device_collection = to_unique(device_collection_ptr);

	UINT count;
	result = device_collection->GetCount(&count);
	if (!SUCCEEDED(result))
		throw std::runtime_error("Failed to get device count: " + to_string(result));

	this->devices.reserve(count + 3);
	static const ERole roles[] = {
		eMultimedia,
		eConsole,
		eCommunications,
	};
	for (auto role : roles){
		try{
			this->devices.emplace_back(std::make_unique<DefaultWasapiOutputDevice>(*this, enumerator, role));
		}catch (std::exception &){
		}
	}
	for (UINT i = 0; i < count; i++){
		IMMDevice *device_ptr;
		result = device_collection->Item(i, &device_ptr);
		if (!SUCCEEDED(result))
			continue;
		try{
			this->devices.emplace_back(std::make_unique<SpecificWasapiOutputDevice>(*this, enumerator, to_unique(device_ptr)));
		}catch (std::exception &){
			continue;
		}
	}
	this->devices_initialized = true;
}

class WasapiOutputDeviceList{
	std::vector<std::string> strings;
	std::vector<OutputDeviceListItem> items;
	OutputDeviceList odl;

	static void release(void *This){
		delete (WasapiOutputDeviceList *)This;
	}
public:
	WasapiOutputDeviceList(const std::vector<std::unique_ptr<BaseWasapiOutputDevice>> &list){
		size_t length;
		if (list.size()){
			this->strings.reserve(list.size());
			this->items.resize(list.size());
			for (auto &i : list){
				auto j = this->strings.size();
				this->strings.push_back(i->get_name());
				auto &item = this->items[j];
				item.name = this->strings[j].c_str();
				memcpy(item.unique_id, i->get_hash().data(), sizeof(item.unique_id));
			}
			length = this->items.size();
		}else{
			length = 0;
			//Ensure this->items[0] is not an invalid operation.
			this->items.resize(1);
		}
		
		this->odl.opaque = this;
		this->odl.release_function = release;
		this->odl.length = length;
		this->odl.items = &this->items[0];
	}
	OutputDeviceList *get_list(){
		return &this->odl;
	}
};

OutputDeviceList *WasapiOutput::get_device_list(){
	this->init_device_list();
	auto list = new WasapiOutputDeviceList(this->devices);
	return list->get_list();
}

AudioFormat *WasapiOutput::get_supported_formats(size_t index){
	this->init_device_list();
	if (index >= this->devices.size())
		return nullptr;
	return this->devices[index]->get_supported_formats();
}

unique_handle_t create_event(bool auto_reset){
	return {
		CreateEventExW(nullptr, nullptr, !auto_reset * (CREATE_EVENT_INITIAL_SET | CREATE_EVENT_MANUAL_RESET), EVENT_MODIFY_STATE | SYNCHRONIZE),
		[](HANDLE h){
			CloseHandle(h);
		}
	};
}

void SpecificWasapiOutputDevice::check_events(){
	if (!this->samples_event || !this->stop_event || !this->stream_switch_event)
		throw std::bad_alloc();
}

SpecificWasapiOutputDevice::SpecificWasapiOutputDevice(WasapiOutput &parent, const com_shared<IMMDeviceEnumerator> &enumerator, com_unique<IMMDevice> &&device):
		BaseWasapiOutputDevice(parent, enumerator),
		device(std::move(device)),
		audio_client(nullptr, com_release<decltype(audio_client)::element_type>),
		render_client(nullptr, com_release<decltype(render_client)::element_type>),
		session_control(nullptr, com_release<decltype(session_control)::element_type>),
		notifier(nullptr, com_release<decltype(notifier)::element_type>){
	this->check_events();
	this->set_vars();
}

SpecificWasapiOutputDevice::SpecificWasapiOutputDevice(WasapiOutput &parent, const com_shared<IMMDeviceEnumerator> &enumerator):
		BaseWasapiOutputDevice(parent, enumerator),
		device(nullptr, com_release<decltype(device)::element_type>),
		audio_client(nullptr, com_release<decltype(audio_client)::element_type>),
		render_client(nullptr, com_release<decltype(render_client)::element_type>),
		session_control(nullptr, com_release<decltype(session_control)::element_type>),
		notifier(nullptr, com_release<decltype(notifier)::element_type>){
	
	this->check_events();
}

com_unique<IPropertyStore> SpecificWasapiOutputDevice::get_property_store(){
	IPropertyStore *property_store_ptr;
	auto result = this->device->OpenPropertyStore(STGM_READ, &property_store_ptr);
	if (!SUCCEEDED(result))
		throw std::exception();
	return to_unique(property_store_ptr);
}

void SpecificWasapiOutputDevice::set_vars(){
	auto property_store = this->get_property_store();
	if (!this->name.size()){
		wchar_t *device_id_ptr;
		auto result = this->device->GetId(&device_id_ptr);
		if (!SUCCEEDED(result))
			throw std::exception();
		auto device_id = to_unique(device_id_ptr, cotaskmemfree);
		this->id = string_to_utf8(device_id.get());


		{
			PROPVARIANT variant;
			PropVariantInit(&variant);
			result = property_store->GetValue(PKEY_Device_FriendlyName, &variant);
			if (!SUCCEEDED(result))
				throw std::exception();
			std::unique_ptr<PROPVARIANT, void(*)(PROPVARIANT *)> variant_unique(&variant, release_PROPVARIANT);

			std::wstring friendly_name = variant.vt != VT_LPWSTR ? L"Unknown" : variant.pwszVal;
			this->name = string_to_utf8(friendly_name);
		}
	}
	this->set_format(*property_store);
	auto s = this->name;
	s += '/';
	s += this->id;
	SHA256 hash;
	hash.input(s);
	this->hash = hash.result();
}

#define MY_DEFINE_PROPERTYKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) EXTERN_C const PROPERTYKEY name = { { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }, pid }
MY_DEFINE_PROPERTYKEY(my_PKEY_AudioEngine_DeviceFormat, 0xf19f064d, 0x82c, 0x4e27, 0xbc, 0x73, 0x68, 0x82, 0xa1, 0xbb, 0x8e, 0x4c, 0); 

void SpecificWasapiOutputDevice::set_format(IPropertyStore &property_store){
	this->exformats.clear();
	this->formats.clear();
	PROPVARIANT variant;
	PropVariantInit(&variant);
	auto result = property_store.GetValue(my_PKEY_AudioEngine_DeviceFormat, &variant);
	if (!SUCCEEDED(result))
		throw std::exception();
	std::unique_ptr<PROPVARIANT, void(*)(PROPVARIANT *)> variant_unique(&variant, release_PROPVARIANT);

	if (variant.vt != VT_BLOB)
		throw std::exception();
			
	this->BaseWasapiOutputDevice::set_format((WAVEFORMATEX *)variant.blob.pBlobData, variant.blob.cbSize);
}

const char *default_device_name(ERole role){
	switch (role){
		case eConsole:
			return "Default console device";
		case eMultimedia:
			return "Default multimedia device";
		case eCommunications:
			return "Default communication device";
		default:
			throw std::exception();
	}
}

DefaultWasapiOutputDevice::DefaultWasapiOutputDevice(WasapiOutput &parent, const com_shared<IMMDeviceEnumerator> &enumerator, ERole role):
		SpecificWasapiOutputDevice(parent, enumerator),
		role(role){

    IMMDevice *device;
	auto result = enumerator->GetDefaultAudioEndpoint(eRender, role, &device);
	if (!SUCCEEDED(result))
		throw std::exception();
	this->device = to_unique(device);
	this->name = default_device_name(role);
	this->set_vars();
}

SpecificWasapiOutputDevice::~SpecificWasapiOutputDevice(){
	this->SpecificWasapiOutputDevice::close();
	this->device.reset();
}

void set_pcm(AudioFormat &dst, const WAVEFORMATEX &src){
	switch (src.wBitsPerSample){
		case 8:
			dst.format = IntS8;
			break;
		case 16:
			dst.format = IntS16;
			break;
		case 24:
			dst.format = IntS24;
			break;
		case 32:
			dst.format = IntS32;
			break;
		default:
			{
				std::stringstream stream;
				stream << "Unsupported PCM format: " << src.wBitsPerSample;
				throw std::runtime_error(stream.str());
			}
	}
}

void set_float(AudioFormat &dst, const WAVEFORMATEX &src){
	switch (src.wBitsPerSample){
		case 32:
			dst.format = Float32;
			break;
		case 64:
			dst.format = Float64;
			break;
		default:
			{
				std::stringstream stream;
				stream << "Unsupported float format: " << src.wBitsPerSample;
				throw std::runtime_error(stream.str());
			}
	}
}

void BaseWasapiOutputDevice::set_format(WAVEFORMATEX *exformat, size_t size){
	exformat_t format((WAVEFORMATEX *)malloc(size), [](WAVEFORMATEX *f){ free(f); });
	memcpy(format.get(), exformat, size);

	AudioFormat af;

	if (format->wFormatTag == WAVE_FORMAT_PCM){
		set_pcm(af, *format);
	}else if (format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT){
		set_float(af, *format);
	}else if (format->wFormatTag == WAVE_FORMAT_EXTENSIBLE){
		auto ext = (WAVEFORMATEXTENSIBLE *)format.get();
		if (ext->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
			set_pcm(af, *format);
		else if (ext->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
			set_float(af, *format);
	}

	if (af.format == Invalid)
		throw std::runtime_error("Unsupported output format.");

	af.channels = format->nChannels;
	if (af.channels < 1){
		std::stringstream stream;
		stream << "Unsupported number of channels: " << af.channels;
		throw std::runtime_error(stream.str());
	}

	af.freq = format->nSamplesPerSec;
	if (af.freq < 1){
		std::stringstream stream;
		stream << "Unsupported sampling frequency: " << af.freq;
		throw std::runtime_error(stream.str());
	}

	this->exformats.emplace_back(std::move(format));
	this->formats.push_back(af);

	af.format = Invalid;
	af.channels = 0;
	af.freq = 0;
	this->formats.push_back(af);
}

void SpecificWasapiOutputDevice::open(size_t format_index, const AudioCallbackData &callback){
	if (format_index >= this->exformats.size())
		throw std::runtime_error("Invalid format index.");

	this->render_client.reset();
	this->audio_client.reset();
	
	this->set_client();

	this->format_index = format_index;
	this->initialize_audio_engine();

	this->initialize_stream_switching();

	this->callback = callback;
	this->thread = std::thread([this](){ this->thread_func(); });
	this->start_client();
}

void SpecificWasapiOutputDevice::close(){
	this->stopping = true;
	if (this->stop_event)
		SetEvent(this->stop_event.get());
	if (this->thread.joinable())
		this->thread.join();
	if (this->audio_client)
		this->audio_client->Stop();
	this->render_client.reset();
	this->notifier.reset();
	this->session_control.reset();
	this->enumerator.reset();
	this->audio_client.reset();
	this->stopping = false;
}

void SpecificWasapiOutputDevice::start_client(){
	auto result = this->audio_client->Start();
	if (!SUCCEEDED(result))
		throw std::runtime_error("Failed to start audio client: " + to_string(result));
}

void SpecificWasapiOutputDevice::set_client(){
	IAudioClient *audio_client_ptr;
	auto result = this->device->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, nullptr, (void **)&audio_client_ptr);
    if (!SUCCEEDED(result))
		throw std::runtime_error("Failed to activate audio client: " + to_string(result));
	this->audio_client = to_unique(audio_client_ptr);
}

void SpecificWasapiOutputDevice::initialize_audio_engine(){
	const REFERENCE_TIME one_millisecond = 10000;

	auto &exformat = *this->exformats[this->format_index];

	auto result = this->audio_client->Initialize(
		AUDCLNT_SHAREMODE_SHARED, 
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        one_millisecond,
        0,
        &exformat,
        nullptr
	);
    if (!SUCCEEDED(result))
		throw std::runtime_error("Failed to initialize audio client: " + to_string(result));

	this->selected_format = this->formats[this->format_index];

	UINT32 buffer_size;
	result = this->audio_client->GetBufferSize(&buffer_size);
	if (!SUCCEEDED(result))
		throw std::runtime_error("Failed to get buffer size.");
	this->buffer_size = buffer_size;
	this->bytes_per_sample = exformat.nBlockAlign;

	this->audio_client->SetEventHandle(this->samples_event.get());

    IAudioRenderClient *render_client_ptr;
    result = this->audio_client->GetService(IID_PPV_ARGS(&render_client_ptr));
    if (!SUCCEEDED(result))
		throw std::runtime_error("Failed to get render client: " + to_string(result));
	this->render_client = to_unique(render_client_ptr);
}

OutputDevice *WasapiOutput::open_device(size_t index, size_t format_index, const AudioCallbackData &callback){
	if (index >= this->devices.size())
		throw std::runtime_error("Invalid device index.");

	auto ret = this->devices[index].get();
	ret->open(format_index, callback);
	return ret;
}

template <typename T, size_t N>
constexpr size_t array_length(const T (&)[N]){
	return N;
}

void SpecificWasapiOutputDevice::thread_func(){
    auto wait_array = this->get_events();
	
	auto result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (!SUCCEEDED(result))
		return;

    DWORD mmcss_task_index = 0;
	auto mmcss_handle_ptr = AvSetMmThreadCharacteristicsW(L"Audio", &mmcss_task_index);
	typedef std::remove_pointer<decltype(mmcss_handle_ptr)>::type P;
	std::unique_ptr<P, void (*)(P *handle)> mmcss_handle(mmcss_handle_ptr, [](P *handle){ AvRevertMmThreadCharacteristics(handle); });

	while (!this->stopping){
        DWORD wait_result = WaitForMultipleObjects((DWORD)wait_array.size(), &wait_array[0], false, INFINITE);
		this->handle_event(wait_result);
	}
}

std::vector<HANDLE> SpecificWasapiOutputDevice::get_events(){
	return {
		this->stop_event.get(),
		this->samples_event.get(),
	};
}

bool SpecificWasapiOutputDevice::handle_event(DWORD wait_result){
	const DWORD stop = WAIT_OBJECT_0 + 0;
	const DWORD get_more_samples = WAIT_OBJECT_0 + 1;
	switch (wait_result){
		case stop:
			return true;
		case get_more_samples:
			this->get_more_samples();
			return true;
	}
	return true;
}

std::vector<HANDLE> DefaultWasapiOutputDevice::get_events(){
	return {
		this->stop_event.get(),
		this->stream_switch_event.get(),
		this->samples_event.get(),
	};
}

bool DefaultWasapiOutputDevice::handle_event(DWORD wait_result){
	const DWORD stop = WAIT_OBJECT_0 + 0;
	const DWORD stream_switch = WAIT_OBJECT_0 + 1;
	const DWORD get_more_samples = WAIT_OBJECT_0 + 2;
	switch (wait_result){
		case stop:
			return true;
		case stream_switch:
			return !this->switch_streams();
		case get_more_samples:
			this->get_more_samples();
			return true;
	}
	return true;
}

void SpecificWasapiOutputDevice::get_more_samples(){
	UINT32 samples_queued;
	auto result = this->audio_client->GetCurrentPadding(&samples_queued);
	if (!SUCCEEDED(result))
		return;
	auto samples_to_read = this->buffer_size - samples_queued;
	BYTE *buffer;
	result = this->render_client->GetBuffer((UINT32)samples_to_read, &buffer);
	if (!SUCCEEDED(result))
		return;
	auto samples_read = this->get_audio(buffer, samples_to_read, samples_queued);
	DWORD flags = 0;
	double underflow = 0;
	if (samples_read){
		auto bytes_read = samples_read * this->bytes_per_sample;
		auto zeroes = samples_to_read * this->bytes_per_sample - bytes_read;
		if (zeroes){
			underflow = (double)(samples_to_read - samples_read) / this->selected_format.freq;
			memset(buffer + bytes_read, 0, zeroes);
		}
	}else{
		flags = AUDCLNT_BUFFERFLAGS_SILENT;
		samples_read = samples_to_read;
			underflow = (double)samples_to_read / this->selected_format.freq;
	}
	if (underflow)
		std::cout << "Underflow detected. " << underflow * 1000 << " ms.\n";
	result = this->render_client->ReleaseBuffer(samples_read, flags);
}

class SessionNotifier : public IAudioSessionEvents, public IMMNotificationClient{
	shared_handle_t stream_switch_event,
		stream_switch_complete_event;
	std::shared_ptr<std::atomic<bool>> switching_streams;
    LONG refcount = 0;
public:
	SessionNotifier(
		const shared_handle_t &stream_switch_event,
		const shared_handle_t &stream_switch_complete_event,
		const std::shared_ptr<std::atomic<bool>> &switching_streams
	):
		stream_switch_event(stream_switch_event),
		stream_switch_complete_event(stream_switch_complete_event),
		switching_streams(switching_streams){}
	virtual ~SessionNotifier(){}
	STDMETHOD(OnDisplayNameChanged)(LPCWSTR NewDisplayName, LPCGUID EventContext){
		return S_OK;
	}
    STDMETHOD(OnIconPathChanged)(LPCWSTR NewIconPath, LPCGUID EventContext){
		return S_OK;
	}
    STDMETHOD(OnSimpleVolumeChanged)(float NewSimpleVolume, BOOL NewMute, LPCGUID EventContext){
		return S_OK;
	}
	STDMETHOD(OnChannelVolumeChanged)(DWORD ChannelCount, float NewChannelVolumes[], DWORD ChangedChannel, LPCGUID EventContext){
		return S_OK;
	}
    STDMETHOD(OnGroupingParamChanged)(LPCGUID NewGroupingParam, LPCGUID EventContext){
		return S_OK;
	}
    STDMETHOD(OnStateChanged)(AudioSessionState NewState){
		return S_OK;
	}
    STDMETHOD(OnDeviceStateChanged)(LPCWSTR DeviceId, DWORD NewState){
		return S_OK;
	}
    STDMETHOD(OnDeviceAdded)(LPCWSTR DeviceId){
		return S_OK;
	}
    STDMETHOD(OnDeviceRemoved)(LPCWSTR DeviceId){
		return S_OK;
	}
    STDMETHOD(OnPropertyValueChanged)(LPCWSTR DeviceId, const PROPERTYKEY Key){
		return S_OK;
	}
	STDMETHOD(OnSessionDisconnected) (AudioSessionDisconnectReason DisconnectReason){
		switch (DisconnectReason){
			case DisconnectReasonDeviceRemoval:
				*this->switching_streams = true;
				SetEvent(this->stream_switch_event.get());
				break;
			case DisconnectReasonServerShutdown:
				break;
			case DisconnectReasonFormatChanged:
				*this->switching_streams = true;
				SetEvent(this->stream_switch_event.get());
				SetEvent(this->stream_switch_complete_event.get());
				break;
			case DisconnectReasonSessionLogoff:
				break;
			case DisconnectReasonSessionDisconnected:
				break;
			case DisconnectReasonExclusiveModeOverride:
				break;
			default:
				break;
		}
		return S_OK;
	}
	STDMETHOD(OnDefaultDeviceChanged)(EDataFlow Flow, ERole Role, LPCWSTR NewDefaultDeviceId){
		bool expected = false;
		if (this->switching_streams->compare_exchange_strong(expected, true))
			SetEvent(this->stream_switch_event.get());
		SetEvent(this->stream_switch_complete_event.get());
		return S_OK;
	}

	STDMETHOD(QueryInterface)(REFIID iid, void **object){
		if (!object)
			return E_POINTER;
		*object = nullptr;

		if (iid == IID_IUnknown)
		{
			*object = static_cast<IUnknown *>(static_cast<IAudioSessionEvents *>(this));
			AddRef();
		}
		else if (iid == __uuidof(IMMNotificationClient))
		{
			*object = static_cast<IMMNotificationClient *>(this);
			AddRef();
		}
		else if (iid == __uuidof(IAudioSessionEvents))
		{
			*object = static_cast<IAudioSessionEvents *>(this);
			AddRef();
		}
		else
		{
			return E_NOINTERFACE;
		}
		return S_OK;
	}
    STDMETHOD_(ULONG, AddRef)(){
		return InterlockedIncrement(&this->refcount);
	}
	STDMETHOD_(ULONG, Release)(){
		ULONG ret = InterlockedDecrement(&this->refcount);
		if (!ret)
			delete this;
		return ret;
	}
};

void DefaultWasapiOutputDevice::initialize_stream_switching(){
    IAudioSessionControl *audio_session_control_ptr;
	auto result = this->audio_client->GetService(IID_PPV_ARGS(&audio_session_control_ptr));
	if (!SUCCEEDED(result))
		throw std::runtime_error("Failed to get audio session control: " + to_string(result));
	this->session_control.reset(audio_session_control_ptr);

	this->notifier.reset(new SessionNotifier(
		this->stream_switch_event,
		this->stream_switch_complete_event,
		this->switching_streams
	));

	this->notifier->AddRef();

	result = this->session_control->RegisterAudioSessionNotification(this->notifier.get());
	if (!SUCCEEDED(result))
		throw std::runtime_error("Failed to get register session notification: " + to_string(result));
	
	result = this->enumerator->RegisterEndpointNotificationCallback(this->notifier.get());
	if (!SUCCEEDED(result))
		throw std::runtime_error("Failed to get register session notification: " + to_string(result));
}

bool DefaultWasapiOutputDevice::switch_streams(){
	try{
		auto result = this->audio_client->Stop();
		if (!SUCCEEDED(result))
			return false;
		result = this->session_control->UnregisterAudioSessionNotification(this->notifier.get());
		if (!SUCCEEDED(result))
			return false;
		result = this->enumerator->UnregisterEndpointNotificationCallback(this->notifier.get());
		if (!SUCCEEDED(result))
			return false;
		this->session_control.reset();
		this->render_client.reset();
		this->audio_client.reset();
		this->device.reset();

		DWORD waitResult = WaitForSingleObject(this->stream_switch_complete_event.get(), 500);
		if (waitResult == WAIT_TIMEOUT)
			return false;

		IMMDevice *device;
		result = this->enumerator->GetDefaultAudioEndpoint(eRender, this->role, &device);
		if (!SUCCEEDED(result))
			return false;
		this->device = to_unique(device);

		this->set_client();
		auto property_store = this->get_property_store();
		this->set_format(*property_store);
		this->notify_client(&this->formats[0]);
		this->initialize_audio_engine();
		this->initialize_stream_switching();
		ResetEvent(this->stream_switch_complete_event.get());
		this->start_client();
		*this->switching_streams = false;
		return true;
	}catch (std::exception &){
		return false;
	}
}
