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
	this->notification_queue.push(NotificationType::Destructing);
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
	LOCK_MUTEX2(this->external_mutex, "Player::play()");
	switch (this->status){
		case Status::Stopped:
			if (this->decoding_thread.joinable())
				this->decoding_thread.join();
			this->decoding_thread = std::thread([this](){ this->decoding_function(); });
			this->status = Status::Playing;
			break;
		case Status::Playing:
			this->seek(rational_t(0, 1));
			break;
		case Status::Paused:
			this->status = Status::Playing;
			this->output_device->pause(false);
			break;
	}
}

void Player::seek(const rational_t &time){
	LOCK_MUTEX2(this->external_mutex, "Player::seek()");
	if (!this->now_playing){
		this->notification_queue.push(NotificationType::SeekComplete);
		return;
	}
	auto &substream = this->now_playing->get_first_source();
	if (substream.seek_to_second(time, false) < 0){
		this->notification_queue.push(NotificationType::SeekComplete);
		return;
	}

	this->now_playing->flush();
	this->queue.flush_queue();
	this->output_device->flush();
	this->executing_seek = true;
}

void Player::pause(){
	LOCK_MUTEX2(this->external_mutex, "Player::pause()");
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
		LOCK_MUTEX2(this->external_mutex, "Player::stop()");
		switch (this->status){
			case Status::Stopped:
				return;
			case Status::Playing:
			case Status::Paused:
				this->status = Status::Stopped;
				break;
		}
		this->now_playing->flush();
		this->queue.flush_queue();
		this->output_device->flush();
	}
	if (this->decoding_thread.joinable())
		this->decoding_thread.join();
	this->decoding_thread = std::thread();
}

void Player::next(){
	LOCK_MUTEX2(this->external_mutex, "Player::next()");
	this->playlist.next_track();
	this->now_playing.reset();
	this->queue.flush_queue();
	this->output_device->flush();
}

void Player::previous(){
	LOCK_MUTEX2(this->external_mutex, "Player::previous()");
	this->playlist.previous_track();
	this->now_playing.reset();
	this->queue.flush_queue();
	this->output_device->flush();
}

rational_t Player::get_current_position(){
	LOCK_MUTEX2(this->external_mutex, "Player::get_current_position()");
	return this->current_time;
}

void Player::set_callbacks(const Callbacks &callbacks){
	LOCK_MUTEX(this->callbacks_mutex, "Player::set_callbacks()");
	this->callbacks = callbacks;
}

void *Player::get_front_cover(size_t playlist_position, size_t &size){
	LOCK_MUTEX2(this->external_mutex, "Player::get_front_cover()");
	if (playlist_position >= this->playlist.size()){
		size = 0;
		return nullptr;
	}
	auto &cover = this->playlist[playlist_position].get_metadata().front_cover();
	size = cover.size();
	if (!size)
		return nullptr;
	std::uint8_t *ret = new (std::nothrow) std::uint8_t[size];
	if (!ret){
		size = 0;
		return nullptr;
	}
	memcpy(ret, cover.data(), size);
	return ret;
}

void Player::release_front_cover(void *buffer){
	delete (std::uint8_t *)buffer;
}

void Player::get_playlist_state(size_t &size, size_t &position){
	LOCK_MUTEX2(this->external_mutex, "Player::get_playlist_state()");
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
		helper.album = metadata.album().to_string();
		helper.date = metadata.date().to_string();
		helper.track_artist = metadata.track_artist().to_string();
		helper.track_id = metadata.track_id();
		helper.track_number = metadata.track_number().to_string();
		helper.track_title = metadata.track_title().to_string();

		numeric_track_info.track_number_int = metadata.track_number_int();
		numeric_track_info.album_gain = metadata.album_gain();
		numeric_track_info.album_peak = metadata.album_peak();
		numeric_track_info.track_gain = metadata.track_gain();
		numeric_track_info.track_peak = metadata.track_peak();
	}

	helper.path = track.get_path();
	numeric_track_info.duration = to_RationalValue(track.get_duration());
	return BasicTrackInfo(numeric_track_info, std::move(helper));
}

void Player::decoding_function(){
	std::shared_ptr<Decoder> decoder;
	unsigned buffer_no = 0;
	//std::ofstream output("output.raw", std::ios::binary);
	auto buffer_index = this->queue.get_next_buffer_index();
	try{
		AudioFormat src_format = {Invalid, 0, 0};
		AudioFormat dst_format = {Invalid, 0, 0};
		std::unique_ptr<BufferSource> last_stream;

		enum class State{
			Initial,
			Decode,
			Done,
		};

		State state = State::Initial;
		typedef std::unique_lock<decltype(this->external_mutex)> L;
		L external_lock;
		decltype(this->now_playing) np;
		std::string current_track_path;
		int current_track_subtrack;
		audio_buffer_t buffer(nullptr, release_buffer);
		while (state != State::Done){
			switch (state){
				case State::Initial:
					{
						external_lock = L(this->external_mutex);
						LOCK_MUTEX2(this->internal_mutex, "Player::decoding_function(), State::Initial");
						if (this->status == Status::Stopped){
							state = State::Done;
							continue;
						}
						dst_format = this->final_format;
						if (this->now_playing){
							np = std::move(this->now_playing);
							state = State::Decode;
							continue;
						}

						if (!this->playlist.size()){
							this->status = Status::Stopped;
							state = State::Done;
							continue;
						}
						auto &track = this->playlist.get_current();
						current_track_path = track.get_path();
						current_track_subtrack = track.get_subtrack();
						external_lock = L();
					}
					{
						if (!decoder || decoder->get_stream().get_path() != current_track_path)
							decoder = this->internal_load(current_track_path);
						auto stream = decoder->get_substream(current_track_subtrack);
						stream->seek_to_sample(0, false);
						auto new_src_format = stream->get_audio_format();
						if (!last_stream || new_src_format != src_format)
							np = build_filter_chain(std::move(stream), new_src_format, this->final_format);
						else
							np = rebuild_filter_chain(std::move(stream), src_format, new_src_format, this->final_format);
						last_stream.reset();
						src_format = new_src_format;
					}
					external_lock = L(this->external_mutex);
					state = State::Decode;
				case State::Decode:
					buffer = np->read();
					if (!buffer || !buffer->sample_count){
						last_stream = std::move(np);
						{
							LOCK_MUTEX2(this->internal_mutex, "Player::decoding_function(), State::Decode");
							this->playlist.next_track();
						}
						external_lock = L();
						state = State::Initial;
						continue;
					}
					{
						auto &extra_data = get_extra_data<BufferExtraData>(buffer);
						extra_data.flags = 0;
						extra_data.buffer_no = buffer_no++;
						bool clear_seek = false;
						if (this->executing_seek){
							extra_data.flags |= BufferExtraData_flags::seek_complete;
							clear_seek = true;
						}
						this->now_playing = std::move(np);
						external_lock = L();
						bool pushed = this->queue.push_to_queue(std::move(buffer), dst_format, buffer_index);
						if (pushed && clear_seek)
							this->executing_seek = false;
						state = State::Initial;
					}
					continue;
			}
		}
	}catch (...){
	}
	this->now_playing.reset();
}

#define CALL_BACK(x) \
	{ \
		LOCK_MUTEX(this->callbacks_mutex, "Player::async_notifications_function()"); \
		if (this->callbacks.on_notification) \
			this->callbacks.on_notification(this->callbacks.cb_data, &notification); \
	}

void Player::async_notifications_function(){
	bool stop = false;
	while (!stop){
		auto notification = this->notification_queue.pop();
		switch (notification.type){
			case NotificationType::Destructing:
				stop = true;
				break;
			case NotificationType::TrackChanged:
				CALL_BACK(on_track_changed);
				break;
			case NotificationType::SeekComplete:
				CALL_BACK(on_seek_complete);
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
						if (this->status == Status::Paused)
							return 0;
						AudioQueueFlags flags = 0;
						rational_t current_time;
						auto ret = this->queue.pop_buffer(current_time, flags, dst, size, samples_queued);
						LOCK_MUTEX2(this->internal_mutex, "Player::open_output()");
						this->current_time = current_time;
						if (flags&AudioQueueFlags_values::track_changed)
							this->notification_queue.push(NotificationType::TrackChanged);
						if (flags&AudioQueueFlags_values::seek_complete)
							this->notification_queue.push(NotificationType::SeekComplete);

						return ret;
					},
					[this](const AudioFormat &af){
						if (af.format == Invalid || !af.channels || !af.freq)
							return;
						this->audio_format_changed(af);
					}
				);
				this->output_device = kv.second;
				this->final_format = formats.front();
				this->queue.set_expected_format(this->final_format);
				return;
			}catch (std::exception &){}
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

	LOCK_MUTEX2(this->internal_mutex, "Player::audio_format_changed()");
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
