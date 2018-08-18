#pragma once

#include "Decoder.h"
#include "OutputModule.h"
#include "Threads.h"
#include "AudioQueue.h"
#include "Playlist.h"
#include "CircularQueue.h"
#include <memory>
#include <mutex>

class Player;

enum class Status{
	Stopped,
	Playing,
	Paused,
};

struct Callbacks{
	void *cb_data = nullptr;
	void (*on_track_changed)(void *cb_data) = nullptr;
};

class Player{
	enum class Notification{
		Destructing,
		TrackChanged,
	};

	std::vector<std::unique_ptr<DecoderModule>> decoders;
	std::vector<std::unique_ptr<OutputModule>> outputs;
	std::unique_ptr<BufferSource> now_playing;
	std::map<uniqueid_t, std::shared_ptr<OutputDevice>> devices;
	std::shared_ptr<OutputDevice> output_device;
	AudioFormat final_format;
	AudioQueue queue;
	std::recursive_mutex mutex;
	Status status = Status::Stopped;
	Playlist playlist;
	std::thread decoding_thread;
	rational_t current_time = {0, 1};
	std::mutex callbacks_mutex;
	Callbacks callbacks;
	std::thread async_notifications_thread;
	ThreadSafeCircularQueue<Notification> notification_queue;
	
	void load_plugins();
	void open_output();
	void audio_format_changed(const AudioFormat &af);
	void decoding_function();
	void async_notifications_function();
	std::shared_ptr<Decoder> internal_load(const std::string &path);
public:
	Player();
	~Player();
	void load_file(const char *path);
	void play();
	void pause();
	void stop();
	void next();
	void previous();
	rational_t get_current_position();
	void set_callbacks(const Callbacks &);
};
