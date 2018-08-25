#include "Playlist.h"

std::pair<size_t, size_t> Playlist::add(const std::shared_ptr<Decoder> &decoder){
	auto tracks = decoder->get_substreams_count();
	auto begin = this->tracks.size();
	for (int i = 0; i < tracks; i++)
		this->tracks.emplace_back(*decoder, i);
	auto end = begin + tracks;
	return {begin, end};
}

Track::Track(Decoder &decoder, int subtrack){
	this->path = decoder.get_stream().get_path();
	this->subtrack = subtrack;
	auto temp = decoder.get_substream(subtrack);
	this->format = temp->get_audio_format();
	this->duration = temp->get_length_in_seconds();
	this->metadata = temp->get_metadata();
}
