#include "OggFlacDecoder.h"

static const long ogg_read_size = 4096;

bool OggFlacDecoder::read_from_input(long read_size = ogg_read_size){
	auto buffer = ogg_sync_buffer(&this->os, read_size);
	if (!buffer)
		throw std::runtime_error("Failed to obtain buffer from libogg.");
	auto bytes = this->io.read(buffer, read_size);
	if (!bytes)
		return false;
	if (ogg_sync_wrote(&this->os, (long)bytes) < 0)
		throw std::runtime_error("Ogg data error.");
	return true;
}

OggFlacDecoder::OggFlacDecoder(const char *path, const ExternalIO &io, Module *module):
		Decoder(module),
		io(io),
		path(path){
	if (!this->io.can_read())
		throw std::runtime_error("Invalid lower IO.");
	this->seekable = this->io.can_seek();
	ogg_sync_init(&this->os);
	if (!this->read_from_input())
		throw std::runtime_error("Input is empty.");

	this->find_all_streams();
}

OggFlacDecoder::~OggFlacDecoder(){
	ogg_sync_clear(&this->os);
}

void OggFlacDecoder::find_all_streams(){
	if (!this->can_seek())
		return;
	this->io.seek(0, SEEK_END);
	this->length = this->io.tell();
	this->io.seek(0, SEEK_SET);
	ogg_sync_reset(&this->os);
	auto page = this->read_page(0, true);
	if (!page)
		return;
	auto last_serial = ogg_page_serialno(&*page);
	std::int64_t last_offset = 0;
	this->streams.emplace_back(last_offset, last_serial);
	while (true){
		int next_serial;
		auto next_offset = this->find_next_stream(last_serial, last_offset, this->length, next_serial);
		if (next_offset < 0)
			break;
		this->streams.emplace_back(next_offset, next_serial);
		last_serial = next_serial;
		last_offset = next_offset;
	}
}

std::int64_t OggFlacDecoder::find_next_stream(int last_serial, std::int64_t lo, std::int64_t hi, int &serial){
	auto delta = hi - lo;
	while (delta > 1){
		auto mid = lo + delta / 2;
		mid = this->get_page_boundary(mid, serial);
		if (mid < 0)
			return -1;

		if (mid == lo || mid == hi)
			return this->final_search(last_serial, lo, hi, serial);

		if (serial < 0){
			auto page = this->read_page(0, true);
			if (!page){
				if (mid - lo > delta)
					return this->final_search(last_serial, lo, mid, serial);
				hi = mid;
			}else
				serial = ogg_page_serialno(&*page);
		}
		if (serial >= 0){
			if (serial > last_serial)
				hi = mid;
			else
				lo = mid;
		}
		delta = hi - lo;
	}
	return hi;
}

std::int64_t OggFlacDecoder::final_search(int last_serial, std::int64_t lo, std::int64_t hi, int &serial){
	while (true){
		auto i = this->get_page_boundary(lo, serial);
		if (i < 0)
			return -1;
		if (i > hi)
			return -1;
		if (serial < 0){
			auto page = this->read_page(0, true);
			if (!page)
				return -1;
			serial = ogg_page_serialno(&*page);
		}else
			i++;
		if (serial > last_serial)
			return i;
		lo = i;
	}
}

std::int64_t OggFlacDecoder::get_page_boundary(std::int64_t offset){
	int serial;
	return this->get_page_boundary(offset, serial);
}

std::int64_t OggFlacDecoder::get_page_boundary(std::int64_t offset, int &serial){
	this->io.seek(offset, SEEK_SET);
	ogg_sync_reset(&this->os);
	if (!this->read_from_input())
		return -1;
	while (true){
		ogg_page page;
		auto result = ogg_sync_pageseek(&this->os, &page);
		if (!result){
			offset = this->io.tell();
			if (!this->read_from_input())
				return -1;
			continue;
		}
		if (result < 0){
			offset -= result;
			serial = -1;
		}else
			serial = ogg_page_serialno(&page);
		break;
	}
	return offset;
}

boost::optional<ogg_page> OggFlacDecoder::read_page(int serial, bool any){
	if (!any && this->returned_page){
		if (this->returned_page->first == serial){
			ogg_page ret = this->returned_page->second;
			this->returned_page.reset();
			return ret;
		}
		this->returned_page.reset();
	}
	ogg_page ret;
	auto result = ogg_sync_pageout(&this->os, &ret);
	
	do{
		while (result < 0){
			if (!this->read_from_input())
				return {};
			result = ogg_sync_pageout(&this->os, &ret);
		}

		while (!result){
			if (!this->read_from_input())
				return {};
			result = ogg_sync_pageout(&this->os, &ret);
		}

	}while (result < 0);

	return ret;
}

void OggFlacDecoder::return_page(const ogg_page &page, int serial){
	this->returned_page.reset({serial, page});
}

DecoderSubstream *OggFlacDecoder::get_substream(int i){
	if (i < 0 || i >= this->streams.size())
		return nullptr;
	if (this->io.can_seek())
		this->io.seek(this->streams[i].first, SEEK_SET);
	return new OggFlacDecoderSubstream(*this, i);
}

OggFlacDecoderSubstream::OggFlacDecoderSubstream(OggFlacDecoder &parent, int stream_index):
		AbstractFlacDecoderSubstream(parent, stream_index),
		flac_parent(parent),
		flac(parent.path.c_str(), get_io(parent, stream_index), parent.module){
	this->substream.reset(static_cast<FlacDecoderSubstream *>(this->flac.get_substream(0)));
	this->length = this->substream->get_length_in_samples();
	this->seconds_length = to_rational(this->substream->get_length_in_seconds());
	this->format = this->substream->get_audio_format();
}

SlicedIO OggFlacDecoderSubstream::get_io(OggFlacDecoder &decoder, int serial){
	if (!decoder.streams.size())
		return SlicedIO(decoder.io);
	std::int64_t beggining = decoder.streams[serial].first,
		length = decoder.length - beggining;
	if (decoder.streams.size() > serial + 1)
		length = decoder.streams[serial + 1].first - beggining;
	return SlicedIO(decoder.io.get_io(), beggining, length);
}
