#pragma once

#include "Decoder.h"
#include "OutputModule.h"
#include "Threads.h"
#include "AudioQueue.h"
#include "Playlist.h"
#include <memory>
#include <mutex>

class Player;

enum class Status{
	Stopped,
	Playing,
	Paused,
};

class Player{
	std::vector<std::unique_ptr<DecoderModule>> decoders;
	std::vector<std::unique_ptr<OutputModule>> outputs;
	std::unique_ptr<BufferSource> now_playing;
	std::map<uniqueid_t, std::shared_ptr<OutputDevice>> devices;
	std::shared_ptr<OutputDevice> output_device;
	AudioFormat final_format;
	AudioQueue queue;
	std::mutex mutex;
	Status status = Status::Stopped;
	Playlist playlist;
	std::thread decoding_thread;
	rational_t current_time = {0, 1};
	
	void load_plugins();
	void open_output();
	void audio_format_changed(const AudioFormat &af);
	void decoding_function();
	std::shared_ptr<Decoder> internal_load(const std::string &path);
public:
	Player();
	~Player();
	void load_file(const char *path);
	void play();
	void pause();
	void stop();
	rational_t get_current_position();
};
