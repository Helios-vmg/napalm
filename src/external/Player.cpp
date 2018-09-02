#include "Player.h"
#include <utf8.cpp>
#include <functional>
#include <boost/dll.hpp>
#include <RationalValue.h>
#include <iostream>

typedef void *PlayerPtr;

typedef struct{
	uint8_t unique_id[32];
} UniqueID;

typedef struct{
	const char *name;
	UniqueID unique_id;
} OutputDeviceListItem;

typedef struct{
	void *opaque;
	void (*release_function)(void *);
	size_t length;
	OutputDeviceListItem *items;
} OutputDeviceList;

struct Level{
	char level_count;
	float levels[16];
};

extern "C"{
	typedef PlayerPtr (*napalm_create_player_f)();
	typedef void(*napalm_destroy_player_f)(PlayerPtr player);
	typedef int (*napalm_load_file_f)(PlayerPtr player, const char *path);
	typedef void (*napalm_play_f)(PlayerPtr player);
	typedef void (*napalm_pause_f)(PlayerPtr player);
	typedef void (*napalm_stop_f)(PlayerPtr player);
	typedef void (*napalm_get_current_time_f)(PlayerPtr player, RationalValue *time, Level *level);
	typedef void (*napalm_get_outputs_f)(PlayerPtr player, OutputDeviceList *dst);
	typedef void (*napalm_select_output_f)(PlayerPtr player, const std::uint8_t *dst);
}

static const wchar_t * const library_name = L"napalm";

#define DECLARE_FUNC(x) x##_f fp_##x
#define GET_FUNCTION(x) function_name = #x; \
	this->fp_##x = this->module.get<std::remove_pointer<x##_f>::type>(function_name)

std::string format_time(rational_t time){
	bool negative = time < 0;
	if (negative)
		time = -time;
	auto milliseconds = time.numerator() % time.denominator() * 1000 / time.denominator();
	auto total = time.numerator() / time.denominator();
	auto seconds = total % 60;
	total /= 60;
	auto minutes = total % 60;
	total /= 60;
	auto hours = total % 24;
	total /= 24;
	auto days = total % 7;
	auto weeks = total / 7;

	std::stringstream stream;
	
	bool force = false;

	if (negative)
		stream << "-(";

	if (weeks){
		stream << weeks << " wk ";
		force = true;
	}
	
	if (force || days){
		stream << days << " d ";
		force = true;
	}

	if (force || hours)
		stream << std::setw(2) << std::setfill('0') << hours << ':';
	stream
		<< std::setw(2) << std::setfill('0') << minutes << ':'
		<< std::setw(2) << std::setfill('0') << seconds;
	
	if (milliseconds)
		stream << '.' << std::setw(3) << std::setfill('0') << milliseconds;

	if (negative)
		stream << ')';
	
	return stream.str();
}

std::string absolute_format_time(const rational_t &time){
	if (time < 0)
		return "undefined";
	return format_time(time);
}

class Player::Pimpl{
	boost::dll::shared_library module;
	DECLARE_FUNC(napalm_create_player);
	DECLARE_FUNC(napalm_destroy_player);
	DECLARE_FUNC(napalm_load_file);
	DECLARE_FUNC(napalm_play);
	DECLARE_FUNC(napalm_pause);
	DECLARE_FUNC(napalm_stop);
	DECLARE_FUNC(napalm_get_current_time);
	DECLARE_FUNC(napalm_get_outputs);
	DECLARE_FUNC(napalm_select_output);
	PlayerPtr player;

	void init(){
		this->module = boost::dll::shared_library(library_name, boost::dll::load_mode::append_decorations);
		std::cout << "Loaded.\n";

		const char *function_name = nullptr;
		try{
			GET_FUNCTION(napalm_create_player);
			GET_FUNCTION(napalm_destroy_player);
			GET_FUNCTION(napalm_load_file);
			GET_FUNCTION(napalm_play);
			GET_FUNCTION(napalm_pause);
			GET_FUNCTION(napalm_stop);
			GET_FUNCTION(napalm_get_current_time);
			GET_FUNCTION(napalm_get_outputs);
			GET_FUNCTION(napalm_select_output);
		}catch (std::exception &){
			throw std::runtime_error((std::string)"Required function not found: " + function_name);
		}
	}
public:
	Pimpl(){
		this->init();
		this->player = this->fp_napalm_create_player();
		if (!this->player)
			throw std::runtime_error("Error initializing player.");
	}
	~Pimpl(){
		this->fp_napalm_destroy_player(this->player);
	}
	bool load_file(const std::wstring &path){
		auto utf8 = string_to_utf8(path);
		return !!this->fp_napalm_load_file(this->player, utf8.c_str());
	}
	bool load_file(const std::string &path){
		return !!this->fp_napalm_load_file(this->player, path.c_str());
	}
	void play(){
		this->fp_napalm_play(this->player);
	}
	void pause(){
		this->fp_napalm_pause(this->player);
	}
	void stop(){
		this->fp_napalm_stop(this->player);
	}
	rational_t current_time(){
		RationalValue rv;
		Level l;
		this->fp_napalm_get_current_time(this->player, &rv, &l);
		return to_rational(rv);
	}
	std::vector<output_device> get_output_devices(){
		std::vector<output_device> ret;
		OutputDeviceList outputs;
		this->fp_napalm_get_outputs(this->player, &outputs);
		if (!outputs.release_function)
			return ret;
		std::unique_ptr<void, void(*)(void *)> p(outputs.opaque, outputs.release_function);
		ret.reserve(outputs.length);
		for (size_t i = 0; i < outputs.length; i++){
			output_device od;
			od.name = outputs.items[i].name;
			memcpy(od.unique_id.data(), outputs.items[i].unique_id.unique_id, 32);
			ret.push_back(od);
		}
		return ret;
	}
	void open_output_device(const std::uint8_t *unique_id){
		this->fp_napalm_select_output(this->player, unique_id);
	}
};

Player::Player(){
	this->pimpl.reset(new Pimpl);
}

Player::~Player(){}

bool Player::load_file(const std::wstring &path){
	return this->pimpl->load_file(path);
}

bool Player::load_file(const std::string &path){
	return this->pimpl->load_file(path);
}

void Player::play(){
	this->pimpl->play();
}

void Player::pause(){
	this->pimpl->pause();
}

void Player::stop(){
	this->pimpl->stop();
}

rational_t Player::current_time(){
	return this->pimpl->current_time();
}

std::vector<output_device> Player::get_output_devices(){
	return this->pimpl->get_output_devices();
}

void Player::open_output_device(const std::array<std::uint8_t, 32> &id){
	this->pimpl->open_output_device(id.data());
}
