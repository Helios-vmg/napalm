#include "AudioQueue.h"
#include "Decoder.h"

AudioQueue::~AudioQueue(){
	clear_queue(this->head);
}

void AudioQueue::clear_queue(AudioBuffer *&head){
	std::lock_guard<std::mutex> lg(this->mutex);
	while (head){
		auto next = NEXT(head);
		release_buffer(head);
		head = next;
	}
}

void AudioQueue::set_expected_format(const AudioFormat &format){
	std::lock_guard<std::mutex> lg(this->mutex);
	this->expected_format = format;
}

void AudioQueue::push_to_queue(audio_buffer_t &&buffer, AudioFormat format){
	while (true){
		{
			LOCK_MUTEX(this->mutex);
			if (format != this->expected_format)
				return;
			if (this->size < this->limit){
				this->size += rational_t(buffer->sample_count, this->format->freq);
				if (this->tail){
					NEXT(this->tail) = buffer.get();
					this->tail = buffer.release();
				}else
					this->tail = this->head = buffer.release();
				get_extra_data<BufferExtraData>(this->tail).sample_offset = 0;
				break;
			}
		}
		this->event.wait();
	}
	clear_queue(this->exit_queue);
}

size_t AudioQueue::pop_buffer(rational_t &time, bool &track_changed, void *void_dst, size_t size, size_t samples_queued){
	size_t ret = 0;
	bool signal = false;
	bool time_set = false;
	{
		LOCK_MUTEX(this->mutex);
		auto dst = (std::uint8_t *)void_dst;
		auto sample_size = sizeof_NumberFormat(this->format->format) * this->format->channels;
		while (size){
			if (!this->head)
				break;
			auto &extra = get_extra_data<BufferExtraData>(this->head);
			if (!time_set){
				time = rational_t(extra.timestamp.numerator, extra.timestamp.denominator);
				time += rational_t(extra.sample_offset, this->format->freq);
				time_set = true;
			}
			if (extra.stream_id != this->current_stream_id){
				this->current_stream_id = extra.stream_id;
				track_changed = true;
			}
			auto copy_size = std::min(size, this->head->sample_count - extra.sample_offset);
			memcpy(dst, this->head->data + extra.sample_offset * sample_size, copy_size * sample_size);
			extra.sample_offset += copy_size;
			ret += copy_size;
			dst += copy_size * sample_size;
			size -= copy_size;
			if (extra.sample_offset >= this->head->sample_count){
				auto old = this->head;
				this->size -= rational_t(old->sample_count, this->format->freq);
				this->head = NEXT(this->head);
				if (!this->head)
					this->tail = nullptr;
				NEXT(old) = this->exit_queue;
				this->exit_queue = old;
				signal = true;
			}
		}
	}
	if (signal || !ret)
		this->event.signal();
	if (!time_set)
		time = rational_t(-1, 1);
	return ret;
}

BufferExtraData AudioQueue::flush_queue(){
	LOCK_MUTEX(this->mutex);
	BufferExtraData ret;
	bool set = false;
	memset(&ret, 0, sizeof(ret));
	while (this->head){
		auto &extra = get_extra_data<BufferExtraData>(this->head);
		auto next = extra.next;
		if (!set){
			set = true;
			ret = extra;
		}
		release_buffer(this->head);
		this->head = next;
	}
	this->tail = nullptr;
	this->size = rational_t(0, 1);
	this->event.signal();
	return ret;
}
