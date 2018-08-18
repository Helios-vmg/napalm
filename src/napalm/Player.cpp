#include "Player.h"
#include "Module.h"
#include "utility.h"
#include "Filter.h"
#include <samplerate.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include <type_traits>
#include <chrono>

Player::Player(): queue(this->final_format), notification_queue(1024){
	this->async_notifications_thread = std::thread([this](){ this->async_notifications_function(); });
	this->load_plugins();
	this->open_output();
}

Player::~Player(){
	this->stop();
	this->notification_queue.push(Notification::Destructing);
	this->async_notifications_thread.join();
}

void Player::load_file(const char *path){
	if (!decoders.size())
		throw std::runtime_error((std::string)"No decoders to load file " + path);
	auto decoder = this->internal_load(path);
	if (!decoder)
		throw std::runtime_error("Load failed.");
	this->playlist.add(decoder);
}

std::shared_ptr<Decoder> Player::internal_load(const std::string &path){
	if (!decoders.size())
		throw std::runtime_error((std::string)"No decoders to load file " + path);
	auto dot = path.rfind('.');
	if (dot == path.npos)
		return nullptr;
	auto ext = path.substr(dot + 1);
	std::string last_error;
	for (auto &d : this->decoders){
		auto &exts = d->get_supported_extensions();
		if (exts.find(ext) == exts.end())
			continue;
		try{
			return d->open(std::make_unique<StdInputStream>(path.c_str()));
		}catch (std::exception &e){
			last_error = e.what();
		}
	}
	if (last_error.size())
		throw std::runtime_error(last_error);
	return nullptr;
}

void Player::play(){
	LOCK_MUTEX(this->mutex);
	switch (this->status){
		case Status::Stopped:
			if (this->decoding_thread.joinable())
				this->decoding_thread.join();
			this->decoding_thread = std::thread([this](){ this->decoding_function(); });
			this->status = Status::Playing;
			break;
		case Status::Playing:
			//TODO
			break;
		case Status::Paused:
			this->status = Status::Playing;
			this->output_device->pause(false);
			break;
	}
}

void Player::pause(){
	LOCK_MUTEX(this->mutex);
	switch (this->status){
		case Status::Stopped:
			break;
		case Status::Playing:
			this->status = Status::Paused;
			this->output_device->pause(true);
			break;
		case Status::Paused:
			this->status = Status::Playing;
			this->output_device->pause(false);
			break;
	}
}

void Player::stop(){
	{
		LOCK_MUTEX(this->mutex);
		switch (this->status){
			case Status::Stopped:
				return;
			case Status::Playing:
			case Status::Paused:
				this->status = Status::Stopped;
				break;
		}
	}
	this->queue.flush_queue();
	this->output_device->flush();
	if (this->decoding_thread.joinable())
		this->decoding_thread.join();
	this->decoding_thread = std::thread();
}

void Player::next(){
	{
		LOCK_MUTEX(this->mutex);
		this->playlist.next_track();
		this->now_playing.reset();
	}
	this->queue.flush_queue();
	this->output_device->flush();
}

void Player::previous(){
	{
		LOCK_MUTEX(this->mutex);
		this->playlist.previous_track();
		this->now_playing.reset();
	}
	this->queue.flush_queue();
	this->output_device->flush();
}

rational_t Player::get_current_position(){
	LOCK_MUTEX(this->mutex);
	return this->current_time;
}

void Player::set_callbacks(const Callbacks &callbacks){
	LOCK_MUTEX(this->callbacks_mutex);
	this->callbacks = callbacks;
}

void Player::get_playlist_state(size_t &size, size_t &position){
	LOCK_MUTEX(this->mutex);
	size = this->playlist.size();
	position = this->playlist.get_current_index();
}

BasicTrackInfo Player::get_basic_track_info(size_t playlist_position){
	if (playlist_position >= this->playlist.size())
		return BasicTrackInfo();
	auto &track = this->playlist[playlist_position];
	BasicTrackInfoHelper helper;
	NumericTrackInfo numeric_track_info;
	if (track.has_metadata()){
		auto &metadata = track.get_metadata();
		helper.album = metadata.album();
		helper.date = metadata.date();
		helper.track_artist = metadata.track_artist();
		helper.track_id = metadata.track_id();
		helper.track_number = metadata.track_number();
		helper.track_title = metadata.track_title();

		numeric_track_info.track_number_int = metadata.track_number_int();
		numeric_track_info.album_gain = metadata.album_gain();
		numeric_track_info.album_peak = metadata.album_peak();
		numeric_track_info.track_gain = metadata.track_gain();
		numeric_track_info.track_peak = metadata.track_peak();
	}else
		//Test string
		helper.track_title = "\xE9\xA6\xAC\xE3\x80\x8C\x45\x71\x75\x75\x73\x20\x63"
		                     "\x61\x62\x61\x6C\x6C\x75\x73\xE3\x80\x8D";
	helper.path = track.get_path();
	numeric_track_info.duration = to_RationalValue(track.get_duration());
	return BasicTrackInfo(numeric_track_info, std::move(helper));
}

void Player::decoding_function(){
	std::shared_ptr<Decoder> decoder;
	//std::ofstream output("output.raw", std::ios::binary);
	try{
		AudioFormat src_format = {Invalid, 0, 0};
		AudioFormat dst_format = {Invalid, 0, 0};
		std::unique_ptr<BufferSource> temp;
		while (true){
			audio_buffer_t buffer(nullptr, release_buffer);
			{
				LOCK_MUTEX(this->mutex);
				if (this->status == Status::Stopped)
					break;
				if (!this->now_playing){
					if (!this->playlist.size()){
						this->status = Status::Stopped;
						break;
					}
					auto &current = this->playlist.get_current();
					if (!decoder || decoder->get_stream().get_path() != current.get_path())
						decoder = this->internal_load(current.get_path());
					auto stream = decoder->get_substream(current.get_subtrack());
					stream->seek_to_sample(0, false);
					auto new_src_format = stream->get_audio_format();
					if (!temp || new_src_format != src_format)
						this->now_playing = build_filter_chain(std::move(stream), new_src_format, this->final_format);
					else
						this->now_playing = rebuild_filter_chain(std::move(stream), src_format, new_src_format, this->final_format);
					src_format = new_src_format;
				}
				dst_format = this->final_format;
				buffer = this->now_playing->read();
				if (!buffer || !buffer->sample_count){
					temp = std::move(this->now_playing);
					this->playlist.next_track();
					continue;
				}
			}
			//output.write((const char *)buffer->data, buffer->sample_count * dst_format.channels * sizeof_NumberFormat(dst_format.format));
			this->queue.push_to_queue(std::move(buffer), dst_format);
		}
	}catch (...){
	}
	this->now_playing.reset();
}

void Player::async_notifications_function(){
	bool stop = false;
	while (!stop){
		auto type = this->notification_queue.pop();
		switch (type){
			case Notification::Destructing:
				stop = true;
				break;
			case Notification::TrackChanged:
				{
					LOCK_MUTEX(this->callbacks_mutex);
					if (this->callbacks.on_track_changed)
						this->callbacks.on_track_changed(this->callbacks.cb_data);
				}
				break;
			default:
				break;
		}
	}
}

void Player::load_plugins(){
	typedef boost::filesystem::directory_iterator D;
	for (D i("./plugins"), end; i != end; ++i){
		auto path = i->path().wstring();
		if (!glob(L"*.dll", path.c_str(), tolower))
			continue;
		auto module = Module::load(path);
		if (!module)
			continue;
		if (module->module_supports(DECODER_MODULE_TYPE))
			this->decoders.emplace_back(std::make_unique<DecoderModule>(module));
		if (module->module_supports(OUTPUT_MODULE_TYPE))
			this->outputs.emplace_back(std::make_unique<OutputModule>(module));
	}
}

void Player::open_output(){
	if (!this->outputs.size())
		throw std::runtime_error("No outputs available.");
	for (auto &output : this->outputs)
		for (auto &kv : output->get_devices())
			this->devices[kv.first] = kv.second;
	if (!this->devices.size())
		throw std::runtime_error("No devices available.");

	//Test code:
	for (auto &output : this->outputs){
		for (auto &kv : output->get_devices()){
			try{
				auto formats = kv.second->get_preferred_formats();
				if (!formats.size())
					continue;
				kv.second->open(
					0,
					[this](void *dst, size_t size, size_t samples_queued) -> size_t{
						LOCK_MUTEX(this->mutex);
						if (this->status == Status::Paused)
							return 0;
						bool track_changed = false;
						auto ret = this->queue.pop_buffer(this->current_time, track_changed, dst, size, samples_queued);
						if (track_changed)
							this->notification_queue.push(Notification::TrackChanged);
						return ret;
					},
					[this](const AudioFormat &af){
						this->audio_format_changed(af);
					}
				);
				this->output_device = kv.second;
				this->final_format = formats.front();
				this->queue.set_expected_format(this->final_format);
				return;
			}catch (std::exception &e){}
		}
	}
}

void Player::audio_format_changed(const AudioFormat &af){
	auto old_format = this->final_format;
	if (af == old_format){
		//std::cout << "Default device changed. Nothing special to do.\n";
		return;
	}

	this->queue.set_expected_format(af);

	LOCK_MUTEX(this->mutex);
	auto temp = this->now_playing->unroll_chain();
	if (temp)
		this->now_playing = std::move(temp);
	auto extra = this->queue.flush_queue();
	auto substream = static_cast<DecoderSubstream *>(this->now_playing.get());
	if (substream->get_stream_id() == extra.stream_id){
		rational_t time(extra.timestamp.numerator, extra.timestamp.denominator);
		time += rational_t(extra.sample_offset, old_format.freq);
		substream->seek_to_second(time, false);
	}
	auto format = substream->get_audio_format();
	this->final_format = af;
	//std::cout << "Default device changed. Format was changed.\n"
	//	"Old format: " << old_format << "\n"
	//	"New format: " << af << std::endl;
	this->now_playing = build_filter_chain(std::move(this->now_playing), format, this->final_format);
}
