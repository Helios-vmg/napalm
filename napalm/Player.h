#pragma once

#include "Decoder.h"
#include "OutputModule.h"
#include "Threads.h"
#include "AudioQueue.h"
#include <memory>
#include <mutex>

class Player;

class Player{
	std::unique_ptr<BufferSource> now_playing;
	std::vector<std::unique_ptr<DecoderModule>> decoders;
	std::vector<std::unique_ptr<OutputModule>> outputs;
	std::map<uniqueid_t, std::shared_ptr<OutputDevice>> devices;
	std::shared_ptr<Decoder> decoder;
	std::shared_ptr<OutputDevice> output_device;
	AudioFormat final_format;
	AudioQueue queue;
	std::mutex mutex;

	void load_plugins();
	void open_output();
	void audio_format_changed(const AudioFormat &af);
public:
	Player();
	void load_file(const char *path);
	void load_playlist(const char *path);
	void play();
	void pause();
	void stop();
	void process_more();
	rational_t get_current_position();
};
