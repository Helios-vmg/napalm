#pragma once

#include "BufferSource.h"
#include "Threads.h"
#include "utility.h"
#include <mutex>
#include <limits>

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
	std::uint64_t current_stream_id = std::numeric_limits<std::uint64_t>::max();
	std::uint64_t next_buffer_index = 0;

	void clear_queue(AudioBuffer *&);
public:
	AudioQueue(AudioFormat &format): format(&format){}
	~AudioQueue();
	std::uint64_t push_to_queue(audio_buffer_t &&buffer, AudioFormat format, std::uint64_t buffer_index);
	size_t pop_buffer(rational_t &time, bool &track_changed, void *void_dst, size_t size, size_t samples_queued);
	BufferExtraData flush_queue();
	void set_expected_format(const AudioFormat &format);
	std::uint64_t get_next_buffer_index(){
		LOCK_MUTEX(this->mutex, "AudioQueue::get_next_buffer_index()");
		return this->next_buffer_index;
	}
};
