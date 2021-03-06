#include "OggDecoder.h"
#include <stdexcept>
#include <boost/integer.hpp>

static const NumberFormat number_format = Float32;

static const char *ogg_code_to_string(int e){
	switch (e){
		case 0:
			return "no error";
		case OV_ENOSEEK:
			return "the stream is not seekable";
		case OV_EREAD:
			return "a read from media returned an error";
		case OV_EIMPL:
			return "feature not implemented";
		case OV_EINVAL:
			return "invalid argument or incompletely initialized argument passed to call";
		case OV_ENOTVORBIS:
			return "bitstream does not contain any Vorbis data";
		case OV_EVERSION:
			return "vorbis version mismatch";
		case OV_EBADHEADER:
			return "invalid Vorbis bitstream header";
		case OV_EFAULT:
			return "internal logic fault; indicates a bug or heap/stack corruption";
		case OV_EBADLINK:
			return "the link exists in the Vorbis data stream, but is indecipherable due to garbage or corruption";
		default:
			return "unknown error";
	}
}

OggDecoder::OggDecoder(const char *path, const ExternalIO &io, Module *module):
		Decoder(module),
		io(io),
		module(module),
		path(path){
	ov_callbacks cb;
	cb.read_func = OggDecoder::read;
	cb.seek_func = OggDecoder::seek;
	cb.tell_func = OggDecoder::tell;
	cb.close_func = nullptr;
	int error = ov_open_callbacks(this, &this->ogg_file, nullptr, 0, cb);
	if (error < 0)
		throw std::runtime_error((std::string)"ov_open_callbacks() failed: " + ogg_code_to_string(error));
	this->stream_count = ov_streams(&this->ogg_file);
	this->seekable = !!ov_seekable(&this->ogg_file);
	this->init_time_resolution();
}

OggDecoder::~OggDecoder(){
	ov_clear(&this->ogg_file);
}

void OggDecoder::init_time_resolution(){
	this->time_resolution = 1;
	std::int64_t sample_accum = 0;
	rational_t time_accum(0, 1);
	this->stream_ranges.reserve(this->stream_count);
	for (int i = 0; i < this->stream_count; i++){
		vorbis_info *info = ov_info(&this->ogg_file, i);
		if (!info)
			continue;
		this->time_resolution = boost::integer::lcm<std::int64_t>(this->time_resolution, info->rate);
		if (!this->seekable)
			return;
		auto length = ov_pcm_total(&this->ogg_file, i);
		StreamRange sr;
		sr.frequency = info->rate;
		sr.time_begin = time_accum;
		sr.sample_begin = sample_accum;
		sr.sample_length = length;
		sr.time_length = rational_t(length, info->rate);
		time_accum += sr.time_length;
		sample_accum += length;
		this->stream_ranges.push_back(sr);
	}
}

OggDecoderSubstream::OggDecoderSubstream(
			OggDecoder &parent,
			int index,
			std::int64_t first_sample,
			std::int64_t length,
			const rational_t &first_moment,
			const rational_t &seconds_length):
		DecoderSubstream(parent, index, first_sample, length, first_moment, seconds_length),
		ogg_parent(parent){

	auto ogg_file = this->ogg_parent.get_ogg_file();

	vorbis_info *info = ov_info(ogg_file, this->index);
	if (!info){
		std::stringstream stream;
		stream << "Failed to read Ogg Vorbis stream info for stream index " << this->index;
		throw std::runtime_error(stream.str());
	}
	this->format.format = number_format;
	this->format.channels = info->channels;
	this->format.freq = info->rate;
	vorbis_comment *comment = ov_comment(ogg_file, this->index);
	if (comment){
		for (auto i = comment->comments; i--;)
			this->metadata.add_vorbis_comment(comment->user_comments[i], comment->comment_lengths[i]);
	}
}

size_t OggDecoder::read(void *buffer, size_t size, size_t nmemb, void *s){
	OggDecoder *This = (OggDecoder *)s;
	return (size_t)This->io.read(buffer, size * nmemb);
}

int OggDecoder::seek(void *s, ogg_int64_t offset, int whence){
	OggDecoder *This = (OggDecoder *)s;
	return This->io.seek(offset, whence) < 0 ? -1 : 0;
}

long OggDecoder::tell(void *s){
	OggDecoder *This = (OggDecoder *)s;
	return (long)This->io.tell();
}

struct OggAudioBuffer{
	AudioBuffer read_result;
};

void release(OggAudioBuffer *rr){
	if (rr)
		rr->read_result.release_function(rr->read_result.opaque);
}

std::unique_ptr<OggAudioBuffer, void (*)(OggAudioBuffer *)> alloc(size_t extra_size, size_t extra_data){
	auto ret = (OggAudioBuffer *)malloc(sizeof(OggAudioBuffer) + extra_size + extra_data);
	if (!ret)
		throw std::bad_alloc();
	ret->read_result.opaque = ret;
	ret->read_result.release_function = free;
	ret->read_result.samples_size = extra_size;
	ret->read_result.extra_data_size = extra_data;
	return {ret, release};
}

int OggDecoder::ov_read(const AudioFormat &af, std::uint8_t *dst, size_t size, size_t &samples, int substream_index){
	int substream;
	float **temp;
	int ret = ::ov_read_float(&this->ogg_file, &temp, (int)samples, &substream);
	if (ret <= 0)
		return ret;
	if (substream != substream_index)
		return 0;
	auto rsamples = ret;
	for (size_t c = 0; c < af.channels; c++){
		auto fdst = (float *)dst + c;
		auto fsrc = temp[c];
		for (size_t s = 0; s < rsamples; s++, fdst += af.channels)
			*fdst = fsrc[s];
	}
	samples -= rsamples;
	return ret * sizeof(float) * af.channels;
}

AudioBuffer *OggDecoder::internal_read(const AudioFormat &af, size_t extra_data, int substream_index){
	size_t bytes_per_sample = af.channels * sizeof_NumberFormat(af.format);
	size_t samples_to_read = 1024;
	size_t bytes_to_read = samples_to_read * bytes_per_sample;

	auto ret = alloc(bytes_to_read, extra_data);

	size_t size = 0;
	while (size < bytes_to_read){
		int r = this->ov_read(af, ret->read_result.data + size, bytes_to_read - size, samples_to_read, substream_index);
		if (r < 0){
			throw std::runtime_error((std::string)"OggDecoder: " + ogg_code_to_string(r));
		}
		if (!r){
			if (!size)
				return nullptr;
			break;
		}

		size += r;
	}
	ret->read_result.sample_count = size / bytes_per_sample;
		
	return &ret.release()->read_result;
}

std::int64_t OggDecoder::seek_to_sample_internal(std::int64_t pos, bool fast){
	if (!fast)
		return !ov_pcm_seek(&this->ogg_file, pos) ? pos : -1;
	
	return !ov_pcm_seek_page(&this->ogg_file, pos) ? ov_pcm_tell(&this->ogg_file) : -1;
}

DecoderSubstream *OggDecoder::get_substream(int index){
	if (index < 0 || index >= this->stream_count)
		return nullptr;
	auto &sr = this->stream_ranges[index];
	auto samples = sr.time_length * sr.frequency;
	return new OggDecoderSubstream(*this, index, sr.sample_begin, samples.numerator() / samples.denominator(), sr.time_begin, sr.time_length);
}
