#include "Player.h"
#include "BasicTrackInfo.h"
#include <decoder_module.h>
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
	return to_RationalValue(player->get_current_position());
}

EXPORT void set_callbacks(Player *player, const Callbacks *callbacks){
	player->set_callbacks(*callbacks);
}

int saturate(size_t n){
	if (n > (size_t)std::numeric_limits<int>::max())
		return std::numeric_limits<int>::max();
	return (int)n;
}

EXPORT void get_playlist_state(Player *player, int *size, int *position){
	size_t s, p;
	player->get_playlist_state(s, p);
	*size = saturate(s);
	*position = saturate(p);
}

EXPORT BasicTrackInfo *get_basic_track_info(Player *player, int playlist_position){
	if (playlist_position < 0)
		return nullptr;
	auto ret = player->get_basic_track_info(playlist_position);
	if (ret.helper == nullptr)
		return nullptr;
	return new BasicTrackInfo(std::move(ret));
}

EXPORT void release_basic_track_info(BasicTrackInfo *info){
	delete info;
}

EXPORT void seek_to_time(Player *player, RationalValue time){
	player->seek(to_rational(time));
}
