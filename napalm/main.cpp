#include "Player.h"
#include "../common/decoder_module.h"
#include <iostream>

#if defined WIN32 || defined _WIN32 || defined _WIN64
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif

struct CurrentTime{
	RationalValue current_time, total_time;
};

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

EXPORT void next(Player *player){
	player->next();
}

EXPORT void previous(Player *player){
	player->previous();
}

EXPORT CurrentTime get_current_time(Player *player){
	auto current = player->get_current_position();
	CurrentTime ret;
	ret.current_time.numerator = current.first.numerator();
	ret.current_time.denominator = current.first.denominator();
	ret.total_time.numerator = current.second.numerator();
	ret.total_time.denominator = current.second.denominator();
	return ret;
}

EXPORT void set_callbacks(Player *player, const Callbacks *callbacks){
	player->set_callbacks(*callbacks);
}
