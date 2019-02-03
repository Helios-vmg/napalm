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
static const std::uint64_t unset_current_stream_id = std::numeric_limits<std::uint64_t>::max();

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

class TrackManager;

class ManagedTrack{
	TrackManager *parent;
	std::unique_lock<std::mutex> track_lock;
	std::unique_ptr<BufferSource> track;
	AudioFormat final_format;
	std::shared_ptr<LevelQueue> level_queue;
public:
	ManagedTrack(): parent(nullptr){}
	ManagedTrack(
			TrackManager &parent,
			std::unique_ptr<BufferSource> &&track,
			std::unique_lock<std::mutex> &&track_lock,
			const AudioFormat &final_format,
			const std::shared_ptr<LevelQueue> &level_queue
		):
		parent(&parent),
		track_lock(std::move(track_lock)),
		track(std::move(track)),
		final_format(final_format),
		level_queue(level_queue){}
	~ManagedTrack();
	ManagedTrack(const ManagedTrack &other) = delete;
	ManagedTrack &operator=(const ManagedTrack &other) = delete;
	ManagedTrack(ManagedTrack &&other){
		*this = std::move(other);
	}
	const ManagedTrack &operator=(ManagedTrack &&other){
		this->parent = other.parent;
		other.parent = nullptr;
		this->track_lock = std::move(other.track_lock);
		this->track = std::move(other.track);
		this->final_format = other.final_format;
		return *this;
	}
	operator bool() const{
		return !!this->track;
	}
	BufferSource &operator*(){
		return *this->track;
	}
	BufferSource *operator->(){
		return this->track.get();
	}
	const AudioFormat &get_format() const{
		return this->final_format;
	}
	LevelQueue &get_level_queue(){
		return *this->level_queue;
	}
};

class TrackManager{
	friend class ManagedTrack;
	mutable std::mutex mutex;
	AudioQueue &queue;
	std::mutex track_mutex;
	bool valid = false;
	std::unique_ptr<BufferSource> track;
	std::uint64_t stream_id = unset_current_stream_id;
	AudioFormat final_format;
	std::map<std::uint64_t, std::shared_ptr<LevelQueue>> level_queues;

	void return_track(std::unique_ptr<BufferSource> &&track);
public:
	TrackManager(AudioQueue &queue) : queue(queue){}
	void clear();
	void set(std::unique_ptr<BufferSource> &&track, const AudioFormat &, const std::shared_ptr<LevelQueue> &);
	void change_format(const AudioFormat &);
	ManagedTrack get_track();
	operator bool() const{
		LOCK_MUTEX(this->mutex);
		return this->valid;
	}
	AudioFormat get_final_format() const{
		LOCK_MUTEX(this->mutex);
		return this->final_format;
	}
	std::uint64_t get_stream_id() const{
		LOCK_MUTEX(this->mutex);
		return this->stream_id;
	}
	std::shared_ptr<LevelQueue> get_level_queue(std::uint64_t id){
		LOCK_MUTEX(this->mutex);
		auto it = this->level_queues.find(id);
		if (it == this->level_queues.end())
			return nullptr;
		return it->second;
	}
	bool get_level(std::uint64_t current_stream_id, const rational_t &time, LevelQueue::Level &level){
		LOCK_MUTEX(this->mutex);
		while (this->level_queues.size()){
			auto begin = this->level_queues.begin();
			if (begin->first < current_stream_id){
				this->level_queues.erase(begin);
				continue;
			}
			level = begin->second->get_level(time);
			return true;
		}
		return false;
	}
};

class Player{
	std::vector<std::unique_ptr<DecoderModule>> decoders;
	std::vector<std::unique_ptr<OutputModule>> outputs;
	AudioQueue queue;
	TrackManager now_playing;
	std::shared_ptr<OutputDevice> output_device;
	std::atomic<Status> status;
	Playlist playlist;
	std::thread decoding_thread;
	rational_t current_time = {-1, 1};
	std::uint64_t current_stream_id = unset_current_stream_id;
	std::mutex mutex;
	std::mutex callbacks_mutex;
	Callbacks callbacks = {nullptr, nullptr};
	std::thread async_notifications_thread;
	ThreadSafeCircularQueue<Notification> notification_queue;
	std::atomic<bool> executing_seek;
	
	void load_plugins();
	void audio_format_changed(const AudioFormat &af);
	void decoding_function();
	void async_notifications_function();
	std::shared_ptr<Decoder> internal_load(const std::string &path);
	struct DecoderState{
		bool load = true;
		bool clear_seek = false;
		unsigned buffer_no = 0;
	};
	audio_buffer_t get_buffer(DecoderState &state, AudioFormat &format);
	bool load_track(std::shared_ptr<Decoder> &decoder);
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
