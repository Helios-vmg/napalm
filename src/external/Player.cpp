#include "Player.h"
#include <utf8.cpp>
#include <functional>
#include <boost/dll.hpp>
#include <RationalValue.h>

typedef void *PlayerPtr;

extern "C"{
	typedef PlayerPtr (*create_player_f)();
	typedef void(*destroy_player_f)(PlayerPtr player);
	typedef int (*load_file_f)(PlayerPtr player, const char *path);
	typedef void (*play_f)(PlayerPtr player);
	typedef void (*pause_f)(PlayerPtr player);
	typedef void (*stop_f)(PlayerPtr player);
	typedef RationalValue (*get_current_time_f)(PlayerPtr player);
}

#if defined _WIN32 || defined _WIN64
#if defined _M_IX86 || defined _X86_ || defined __X86__
static const wchar_t * const library_name = L"napalm";
#elif defined _M_X64 || defined _M_AMD64 || defined __amd64__ || defined __amd64 || defined __x86_64__ || defined __x86_64
static const wchar_t * const library_name = L"napalm";
#else
#error
#endif
#else
#error
#endif

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
	DECLARE_FUNC(create_player);
	DECLARE_FUNC(destroy_player);
	DECLARE_FUNC(load_file);
	DECLARE_FUNC(play);
	DECLARE_FUNC(pause);
	DECLARE_FUNC(stop);
	DECLARE_FUNC(get_current_time);
	PlayerPtr player;

	void init(){
		this->module = boost::dll::shared_library(library_name, boost::dll::load_mode::append_decorations);

		const char *function_name = nullptr;
		try{
			GET_FUNCTION(create_player);
			GET_FUNCTION(destroy_player);
			GET_FUNCTION(load_file);
			GET_FUNCTION(play);
			GET_FUNCTION(pause);
			GET_FUNCTION(stop);
			GET_FUNCTION(get_current_time);
		}catch (std::exception &){
			throw std::runtime_error((std::string)"Required function not found: " + function_name);
		}
	}
public:
	Pimpl(){
		this->init();
		this->player = this->fp_create_player();
		if (!this->player)
			throw std::runtime_error("Error initializing player.");
	}
	~Pimpl(){
		this->fp_destroy_player(this->player);
	}
	bool load_file(const std::wstring &path){
		auto utf8 = string_to_utf8(path);
		return !!this->fp_load_file(this->player, utf8.c_str());
	}
	bool load_file(const std::string &path){
		return !!this->fp_load_file(this->player, path.c_str());
	}
	void play(){
		this->fp_play(this->player);
	}
	void pause(){
		this->fp_pause(this->player);
	}
	void stop(){
		this->fp_stop(this->player);
	}
	rational_t current_time(){
		return to_rational(this->fp_get_current_time(this->player));
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
