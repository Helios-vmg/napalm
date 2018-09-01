#pragma once

#include "Decoder.h"
#include "OutputModule.h"
#include "Threads.h"
#include "AudioQueue.h"
#include "Playlist.h"
#include "CircularQueue.h"
#include "BasicTrackInfo.h"
#include "OutputDeviceList.h"
#include "LevelQueue.h"
#include <memory>
#include <mutex>
#include <cstdint>
#include <limits>

class Player;

enum class Status{
	Stopped,
	Playing,
	Paused,
};

enum class NotificationType : std::int32_t{
	Nothing = 0,
	Destructing,
	TrackChanged,
	SeekComplete,
	PlaylistUpdated,
	VolumeChangedBySystem,
};

struct Notification{
	NotificationType type;
	std::int32_t param1;
	std::int64_t param2;
	std::int64_t param3;
	void *param4;
	Notification():
		type(NotificationType::Nothing),
		param1(0),
		param2(0),
		param3(0),
		param4(nullptr){}
	Notification(NotificationType type):
		type(type),
		param1(0),
		param2(0),
		param3(0),
		param4(nullptr){}
};

struct Callbacks{
	void *cb_data;
	void (*on_notification)(void *, const Notification *);
};

class Player{
	std::vector<std::unique_ptr<DecoderModule>> decoders;
	std::vector<std::unique_ptr<OutputModule>> outputs;
	std::unique_ptr<BufferSource> now_playing;
	std::shared_ptr<OutputDevice> output_device;
	AudioFormat final_format;
	AudioQueue queue;
	mutex_wrapper<std::recursive_mutex> internal_mutex;
	mutex_wrapper<std::recursive_mutex> external_mutex;
	std::atomic<Status> status = Status::Stopped;
	Playlist playlist;
	std::thread decoding_thread;
	rational_t current_time = {-1, 1};
	static const std::uint64_t unset_current_stream_id = std::numeric_limits<std::uint64_t>::max();
	std::uint64_t current_stream_id = unset_current_stream_id;
	std::mutex callbacks_mutex;
	Callbacks callbacks = {nullptr, nullptr};
	std::thread async_notifications_thread;
	ThreadSafeCircularQueue<Notification> notification_queue;
	std::atomic<bool> executing_seek = false;
	std::map<std::uint64_t, std::shared_ptr<LevelQueue>> level_queues;
	
	void load_plugins();
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
	void get_current_position(rational_t &time, LevelQueue::Level &level);
	void set_callbacks(const Callbacks &);
	void get_playlist_state(size_t &size, size_t &position);
	BasicTrackInfo get_basic_track_info(size_t playlist_position);
	void seek(const rational_t &);
	void *get_front_cover(size_t playlist_position, size_t &size);
	void release_front_cover(void *);
	OutputDeviceList get_outputs();
	uniqueid_t get_selected_output();
	void select_output(const uniqueid_t &dst);
	void set_volume(double);
};
