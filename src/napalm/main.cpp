#include "Player.h"
#include "../common/decoder_module.h"
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

EXPORT void next(Player *player){
	player->next();
}

EXPORT void previous(Player *player){
	player->previous();
}

EXPORT RationalValue get_current_time(Player *player){
	auto current = player->get_current_position();
	RationalValue ret;
	ret.numerator = current.numerator();
	ret.denominator = current.denominator();
	return ret;
}

EXPORT void set_callbacks(Player *player, const Callbacks *callbacks){
	player->set_callbacks(*callbacks);
}
