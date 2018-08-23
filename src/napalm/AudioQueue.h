#pragma once

#include "BufferSource.h"
#include "Threads.h"
#include "utility.h"
#include <mutex>
#include <limits>

#define NEXT(x) get_extra_data<BufferExtraData>(x).next

struct BufferExtraData;

typedef std::uint64_t AudioQueueFlags;

namespace AudioQueueFlags_values{
static const std::uint64_t track_changed = (std::uint64_t)1 << 0;
static const std::uint64_t seek_complete = (std::uint64_t)1 << 1;
}

class AudioQueue{
	AudioBuffer *head = nullptr,
		*tail = nullptr,
		*exit_queue = nullptr;
	std::mutex mutex;
	Event event;
	rational_t limit = {10, 1};
	rational_t size = {0, 1};
	size_t queue_elements = 0;
	AudioFormat *format;
	AudioFormat expected_format = {Invalid, 0, 0};
	std::uint64_t current_stream_id = std::numeric_limits<std::uint64_t>::max();
	std::uint64_t next_buffer_index = 0;

	void clear_queue(AudioBuffer *&);
public:
	AudioQueue(AudioFormat &format): format(&format){}
	~AudioQueue();
	std::uint64_t push_to_queue(audio_buffer_t &&buffer, AudioFormat format, std::uint64_t buffer_index);
	size_t pop_buffer(rational_t &time, AudioQueueFlags &flags, void *void_dst, size_t size, size_t samples_queued);
	BufferExtraData flush_queue();
	void set_expected_format(const AudioFormat &format);
	std::uint64_t get_next_buffer_index(){
		LOCK_MUTEX(this->mutex, "AudioQueue::get_next_buffer_index()");
		return this->next_buffer_index;
	}
};
