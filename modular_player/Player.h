#pragma once

#include "AudioBuffer.h"
#include "Decoder.h"
#include <memory>

class Player;

class Player{
	std::unique_ptr<DecoderSubstream> now_playing;
	std::vector<std::unique_ptr<DecoderModule>> decoders;
	std::shared_ptr<Decoder> decoder;

	void load_plugins();
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
