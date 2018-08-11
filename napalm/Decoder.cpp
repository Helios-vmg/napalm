#include "Decoder.h"
#include "Module.h"
#include "../common/decoder_module.h"

std::atomic<std::uint64_t> DecoderSubstream::next_stream_id(0);

void release_buffer(AudioBuffer *rr){
	if (rr)
		rr->release_function(rr->opaque);
}

audio_buffer_t allocate_buffer(size_t byte_size, size_t extra_data){
	auto ret = (AudioBuffer *)malloc(sizeof(AudioBuffer) + byte_size + extra_data);
	if (!ret)
		throw std::bad_alloc();
	ret->opaque = ret;
	ret->release_function = free;
	ret->samples_size = byte_size;
	ret->extra_data_size = extra_data;
	return audio_buffer_t(ret, release_buffer);
}

audio_buffer_t allocate_buffer_by_clone(const audio_buffer_t &src, size_t byte_size){
	auto ret = allocate_buffer(byte_size, src->extra_data_size);
	if (src->extra_data_size)
		memcpy(ret->data + ret->samples_size, src->data + src->samples_size, src->extra_data_size);
	return ret;
}

DecoderModule::DecoderModule(const std::shared_ptr<Module> &module): module(module){
	GET_REQUIRED_FUNCTION(decoder_get_supported_extensions);
	GET_REQUIRED_FUNCTION(decoder_open);
	GET_REQUIRED_FUNCTION(decoder_close);
	GET_REQUIRED_FUNCTION(decoder_get_substreams_count);
	GET_REQUIRED_FUNCTION(decoder_get_substream);
	GET_REQUIRED_FUNCTION(substream_close);
	GET_REQUIRED_FUNCTION(substream_get_audio_format);
	GET_REQUIRED_FUNCTION(substream_set_number_format_hint);
	GET_REQUIRED_FUNCTION(substream_read);
	GET_OPTIONAL_FUNCTION(substream_get_length_in_seconds);
	GET_OPTIONAL_FUNCTION(substream_get_length_in_samples);
	GET_OPTIONAL_FUNCTION(substream_seek_to_sample);
	GET_OPTIONAL_FUNCTION(substream_seek_to_second);

	for (auto table = this->decoder_get_supported_extensions(this->module.get()); *table; table++)
		this->supported_extensions.insert(*table);
}

std::shared_ptr<Decoder> DecoderModule::open(std::unique_ptr<InputStream> &&stream){
	return std::shared_ptr<Decoder>(new Decoder(*this, std::move(stream)));
}

Decoder::Decoder(DecoderModule &module, std::unique_ptr<InputStream> &&stream):
		module(module),
		decoder_ptr(nullptr, module.decoder_close),
		stream(std::move(stream)){
	
	auto ptr = module.module->get_pointer();
	auto path = this->stream->get_path().c_str();
	auto eio = (ExternalIO)*this->stream;
	this->decoder_ptr.reset(module.decoder_open(ptr, path, &eio));
	if (!this->decoder_ptr){
		this->module.module->check_error();
		throw std::runtime_error("Unknown error decoding " + this->stream->get_path());
	}
}

int Decoder::get_substreams_count(){
	return this->module.decoder_get_substreams_count(this->decoder_ptr.get());
}

std::unique_ptr<DecoderSubstream> Decoder::get_substream(int index){
	return std::unique_ptr<DecoderSubstream>(new DecoderSubstream(*this, index));
}

DecoderSubstream::DecoderSubstream(Decoder &decoder, int index):
		module(decoder.module),
		substream_ptr(nullptr, decoder.module.substream_close),
		stream_id(this->next_stream_id++){

	this->substream_ptr.reset(this->module.decoder_get_substream(decoder.decoder_ptr.get(), index));
	if (!this->substream_ptr){
		this->module.module->check_error();
		throw std::runtime_error("Unknown error decoding " + decoder.stream->get_path());
	}
}

AudioFormat DecoderSubstream::get_audio_format(){
	if (!this->format)
		this->format = this->module.substream_get_audio_format(this->substream_ptr.get());
	return *this->format;
}

void DecoderSubstream::set_number_format_hint(NumberFormat nf){
	auto f = this->module.substream_set_number_format_hint;
	if (f)
		f(this->substream_ptr.get(), nf);
}

audio_buffer_t DecoderSubstream::read(){
	auto freq = this->get_audio_format().freq;
	auto buffer = this->module.substream_read(this->substream_ptr.get(), sizeof(BufferExtraData));
	auto ret = audio_buffer_t{buffer, release_buffer};
	if (!buffer)
		return ret;
	if (!buffer->release_function)
		throw std::runtime_error("Invalid buffer.");

	auto &extra_data = get_extra_data<BufferExtraData>(ret);
	extra_data.timestamp.numerator = this->current_time.numerator();
	extra_data.timestamp.denominator = this->current_time.denominator();
	extra_data.next = nullptr;
	extra_data.stream_id = this->stream_id;
	this->current_time += rational_t(ret->sample_count, freq);

	return ret;
}

rational_t to_rational(const RationalValue &r){
	return rational_t(r.numerator, r.denominator);
}

RationalValue to_RationalValue(const rational_t &r){
	return RationalValue{r.numerator(), r.denominator()};
}

rational_t DecoderSubstream::get_length_in_seconds(){
	auto f = this->module.substream_get_length_in_seconds;
	if (!f)
		return rational_t(-1, 1);
	return to_rational(f(this->substream_ptr.get()));
}

std::uint64_t DecoderSubstream::get_length_in_samples(){
	auto f = this->module.substream_get_length_in_samples;
	if (!f)
		return -1;
	return f(this->substream_ptr.get());
}

std::int64_t DecoderSubstream::seek_to_sample(std::int64_t sample, bool fast){
	auto f = this->module.substream_seek_to_sample;
	if (!f)
		return -1;
	auto ret = f(this->substream_ptr.get(), sample, fast);
	auto freq = this->get_audio_format().freq;
	this->current_time = {ret, freq};
	return ret;
}

rational_t DecoderSubstream::seek_to_second(const rational_t &time, bool fast){
	auto f = this->module.substream_seek_to_second;
	if (!f)
		return -1;
	auto ret = to_rational(f(this->substream_ptr.get(), to_RationalValue(time), fast));
	return this->current_time = ret;
}
