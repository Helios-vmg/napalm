#include "Module.h"
#include "../common/decoder.hpp"

#include <mpg123.h>
#include <unordered_map>

const std::unordered_map<int, const char *> error_strings = {
	{ MPG123_ERR,               "Unspecified error" },
	{ MPG123_OK,                "No error" },
	{ MPG123_BAD_OUTFORMAT,     "Bad output format" },
	{ MPG123_BAD_CHANNEL,       "Bad channel count" },
	{ MPG123_BAD_RATE,          "Bad sample rate" },
	{ MPG123_ERR_16TO8TABLE,    "Error 16-to-8 table" },
	{ MPG123_BAD_PARAM,         "Bad parameter" },
	{ MPG123_BAD_BUFFER,        "Bad buffer" },
	{ MPG123_OUT_OF_MEM,        "Out of memory" },
	{ MPG123_NOT_INITIALIZED,   "Not initialized" },
	{ MPG123_BAD_DECODER,       "Bad decoder" },
	{ MPG123_BAD_HANDLE,        "Bad handle" },
	{ MPG123_NO_BUFFERS,        "No buffers" },
	{ MPG123_BAD_RVA,           "Bad RVA" },
	{ MPG123_NO_GAPLESS,        "No gapless" },
	{ MPG123_NO_SPACE,          "No space" },
	{ MPG123_BAD_TYPES,         "Bad types" },
	{ MPG123_BAD_BAND,          "Bad band" },
	{ MPG123_ERR_NULL,          "Error null" },
	{ MPG123_ERR_READER,        "Error reader" },
	{ MPG123_NO_SEEK_FROM_END,  "No seek from end" },
	{ MPG123_BAD_WHENCE,        "Bad whence" },
	{ MPG123_NO_TIMEOUT,        "No timeout" },
	{ MPG123_BAD_FILE,          "Bad file" },
	{ MPG123_NO_SEEK,           "No seek" },
	{ MPG123_NO_READER,         "No reader" },
	{ MPG123_BAD_PARS,          "Bad pars" },
	{ MPG123_BAD_INDEX_PAR,     "Bad index par" },
	{ MPG123_OUT_OF_SYNC,       "Out of sync" },
	{ MPG123_RESYNC_FAIL,       "Resync fail" },
	{ MPG123_NO_8BIT,           "No 8-bit" },
	{ MPG123_BAD_ALIGN,         "Bad align" },
	{ MPG123_NULL_BUFFER,       "Null buffer" },
	{ MPG123_NO_RELSEEK,        "No relative seek" },
	{ MPG123_NULL_POINTER,      "Null pointer" },
	{ MPG123_BAD_KEY,           "Bad key" },
	{ MPG123_NO_INDEX,          "No index" },
	{ MPG123_INDEX_FAIL,        "Index fail" },
	{ MPG123_BAD_DECODER_SETUP, "Bad decoder setup" },
	{ MPG123_MISSING_FEATURE,   "Missing feature" },
	{ MPG123_BAD_VALUE,         "Bad value" },
	{ MPG123_LSEEK_FAILED,      "lseek failed" },
	{ MPG123_BAD_CUSTOM_IO,     "Bad custom I/O" },
	{ MPG123_LFS_OVERFLOW,      "LFS overflow" },
	{ MPG123_INT_OVERFLOW,      "Integer overflow" },
};

const char *mpg123_error_to_string(int error){
	auto it = error_strings.find(error);
	return it == error_strings.end() ? "Unknown error" : it->second;
}

Module::Module(){
	auto error = mpg123_init();
	if (error != MPG123_OK){
		this->set_error(mpg123_error_to_string(error));
		this->initialized = false;
	}else
		this->initialized = true;
}

Module::~Module(){
	if (this->initialized)
		mpg123_exit();
}

void Module::set_error(const std::string &error){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	this->errors[id] = error;
}

const char *Module::get_error(){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	auto it = this->errors.find(id);
	if (it == this->errors.end())
		return nullptr;
	return it->second.c_str();
}

void Module::clear_error(){
	auto id = std::this_thread::get_id();
	std::lock_guard<std::mutex> lg(this->mutex);
	auto it = this->errors.find(id);
	if (it == this->errors.end())
		return;
	this->errors.erase(it);
}
