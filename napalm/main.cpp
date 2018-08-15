#include "Player.h"
#include <iostream>

#if defined WIN32 || defined _WIN32 || defined _WIN64
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif

EXPORT Player *create_player(){
	try{
		return new Player;
	}catch (...){
	}
	return nullptr;
}

EXPORT void destroy_player(Player *player){
	delete player;
}

EXPORT int load_file(Player *player, const char *path){
	try{
		player->load_file(path);
		return 1;
	}catch (...){
	}
	return 0;
}

EXPORT void play(Player *player){
	player->play();
}

EXPORT void pause(Player *player){
	player->pause();
}

EXPORT void stop(Player *player){
	player->stop();
}

struct Rational{
	std::int64_t numerator, denominator;
};

struct CurrentTime{
	Rational current_time, total_time;
};

EXPORT CurrentTime get_current_time(Player *player){
	auto current = player->get_current_position();
	CurrentTime ret;
	ret.current_time.numerator = current.numerator();
	ret.current_time.denominator = current.denominator();
	ret.total_time.numerator = -1;
	ret.total_time.denominator = 1;
	return ret;
}
