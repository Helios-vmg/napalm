#include "LevelQueue.h"

typedef smart_c_struct<AudioBuffer> (*converter_f)(const smart_c_struct<AudioBuffer> &, size_t, size_t);
converter_f get_copy_converter(NumberFormat src, NumberFormat dst);

void LevelQueue::set_format(const AudioFormat &format){
	if (format.channels > array_length(Level().levels)){
		this->format = {Invalid, 0, 0};
		return;
	}
	this->temp_buffer = std::vector<Level>();
	temp_buffer.reserve(format.freq * 50 / 1000);
	this->format = format;
	this->converter = get_copy_converter(format.format, Float32);
	this->bytes_per_sample = sizeof_NumberFormat(Float32) * format.channels;
}

void LevelQueue::push_data(const smart_c_struct<AudioBuffer> &buffer){
	if (!buffer)
		return;
	auto temp = this->converter(buffer, this->bytes_per_sample, this->format.channels);
	auto samples = (float *)temp->data;
	auto channels = this->format.channels;
	for (size_t i = 0; i < temp->sample_count; i++){
		auto sample = samples + i * channels;
		Level level;
		level.level_count = (char)channels;
		for (size_t j = 0; j < channels; j++)
			level.levels[j] = sample[j];
		this->temp_buffer.push_back(level);
		if (this->temp_buffer.size() == this->temp_buffer.capacity()){
			LevelInstant li;
			li.time = this->time;
			li.duration = {this->temp_buffer.size(), this->format.freq};
			li.level = this->level_function(this->temp_buffer);
			this->time += li.duration;
			this->temp_buffer.clear();
			LOCK_MUTEX(this->mutex);
			this->queue.push_back(li);
		}
	}
}

LevelQueue::Level LevelQueue::get_level(const rational_t &time){
	LOCK_MUTEX(this->mutex);
	Level ret;
	while (this->queue.size()){
		auto &front = this->queue.front();
		if (time < front.time)
			break;
		if (time >= front.time + front.duration){
			this->queue.pop_front();
			continue;
		}
		ret = front.level;
		return ret;
	}
	memset(&ret, 0, sizeof(ret));
	return ret;
}

void LevelQueue::clear(const rational_t &time){
	LOCK_MUTEX(this->mutex);
	this->queue.clear();
	this->temp_buffer.clear();
	this->time = time;
}
