#pragma once

#include <string>
#include "../common/audio_format.h"
#include "Decoder.h"
#include "MetadataModule.h"
#include <vector>

class Track{
	std::string path;
	int subtrack;
	AudioFormat format;
	rational_t duration;
	std::shared_ptr<GenericMetadata> metadata;
public:
	Track() = default;
	Track(Decoder &, int subtrack);
	Track(const Track &) = default;
	Track(Track &&) = default;
	Track &operator=(const Track &) = default;
	Track &operator=(Track &&) = default;

	const std::string &get_path() const{
		return this->path;
	}
	int get_subtrack() const{
		return this->subtrack;
	}
	AudioFormat get_format() const{
		return this->format;
	}
	const rational_t &get_duration() const{
		return this->duration;
	}
	bool has_metadata() const{
		return !!this->metadata;
	}
	GenericMetadata &get_metadata() const{
		return *this->metadata;
	}
};

class Playlist{
	std::shared_ptr<Decoder> current_decoder;
	std::vector<Track> tracks;
	size_t current_index = 0;
public:
	std::pair<size_t, size_t> add(const std::shared_ptr<Decoder> &);
	size_t get_current_index() const{
		return this->current_index;
	}
	const Track &get_current() const{
		return this->tracks[this->current_index];
	}
	const Track &operator[](size_t i) const{
		return this->tracks[i];
	}
	void clear(){
		this->current_decoder.reset();
		this->tracks.clear();
		this->current_index = 0;
	}
	void next_track(){
		if (!this->tracks.size())
			return;
		this->current_index++;
		this->current_index %= this->tracks.size();
	}
	void previous_track(){
		if (!this->tracks.size())
			return;
		this->current_index += this->tracks.size() - 1;
		this->current_index %= this->tracks.size();
	}
	size_t size() const{
		return this->tracks.size();
	}
};
