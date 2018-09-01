#pragma once

#include "utility.h"
#include "Threads.h"
#include <RationalValue.h>
#include <audio_format.h>
#include <decoder_module.h>
#include <deque>
#include <vector>
#include <mutex>

class LevelQueue{
public:
	struct Level{
		char level_count;
		float levels[16];
	};
	struct LevelInstant{
		rational_t time;
		rational_t duration;
		Level level;
	};
	typedef Level (*level_f)(const std::vector<Level> &);
private:
	std::deque<LevelInstant> queue;
	std::vector<Level> temp_buffer;
	std::mutex mutex;
	typedef smart_c_struct<AudioBuffer> (*converter_f)(const smart_c_struct<AudioBuffer> &, size_t, size_t);
	converter_f converter = nullptr;
	AudioFormat format = {Invalid, 0, 0};
	size_t bytes_per_sample = 0;
	level_f level_function = nullptr;
	rational_t time = {0, 1};
public:
	LevelQueue(level_f f): level_function(f){}
	void set_format(const AudioFormat &format);
	void push_data(const smart_c_struct<AudioBuffer> &buffer);
	Level get_level(const rational_t &);
	void clear(const rational_t &time);
};
