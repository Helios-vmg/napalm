#pragma once

#include "BufferSource.h"
#include "Threads.h"
#include "utility.h"
#include <mutex>

#define NEXT(x) get_extra_data<BufferExtraData>(x).next

struct BufferExtraData;

class AudioQueue{
	AudioBuffer *head = nullptr,
		*tail = nullptr,
		*exit_queue = nullptr;
	std::mutex mutex;
	Event event;
	rational_t limit = {1, 1};
	rational_t size = {0, 1};
	AudioFormat *format;
	AudioFormat expected_format = {Invalid, 0, 0};
	void clear_queue(AudioBuffer *&);
public:
	AudioQueue(AudioFormat &format): format(&format){}
	~AudioQueue();
	void push_to_queue(audio_buffer_t &&buffer, AudioFormat format);
	size_t pop_buffer(void *void_dst, size_t size, size_t samples_queued);
	BufferExtraData flush_queue();
	void set_expected_format(const AudioFormat &format);
};
