#include "Player.h"
#include "BasicTrackInfo.h"
#include <decoder_module.h>
#include <iostream>

#if defined WIN32 || defined _WIN32 || defined _WIN64
#define EXPORT extern "C" __declspec(dllexport)
#elif defined __GNUC__ || defined __clang__
#define EXPORT extern "C" __attribute__((visibility ("default")))
#else
#define EXPORT extern "C"
#endif

EXPORT Player *napalm_create_player(){
	try{
		return new Player;
	}catch (...){
	}
	return nullptr;
}

EXPORT void napalm_destroy_player(Player *player){
	delete player;
}

EXPORT std::int32_t napalm_load_file(Player *player, const char *path){
	try{
		player->load_file(path);
		return 1;
	}catch (...){
	}
	return 0;
}

EXPORT void napalm_play(Player *player){
	player->play();
}

EXPORT void napalm_pause(Player *player){
	player->pause();
}

EXPORT void napalm_stop(Player *player){
	player->stop();
}

EXPORT void napalm_next(Player *player){
	player->next();
}

EXPORT void napalm_previous(Player *player){
	player->previous();
}

EXPORT void napalm_get_current_time(Player *player, RationalValue *time, LevelQueue::Level *level){
	rational_t temp;
	player->get_current_position(temp, *level);
	*time = to_RationalValue(temp);
}

EXPORT void napalm_set_callbacks(Player *player, const Callbacks *callbacks){
	player->set_callbacks(*callbacks);
}

int saturate(size_t n){
	if (n > (size_t)std::numeric_limits<int>::max())
		return std::numeric_limits<int>::max();
	return (int)n;
}

EXPORT void napalm_get_playlist_state(Player *player, std::int32_t *size, std::int32_t *position){
	size_t s, p;
	player->get_playlist_state(s, p);
	*size = saturate(s);
	*position = saturate(p);
}

EXPORT BasicTrackInfo *napalm_get_basic_track_info(Player *player, std::int32_t playlist_position){
	if (playlist_position < 0)
		return nullptr;
	auto ret = player->get_basic_track_info(playlist_position);
	if (ret.helper == nullptr)
		return nullptr;
	return new BasicTrackInfo(std::move(ret));
}

EXPORT void napalm_release_basic_track_info(BasicTrackInfo *info){
	delete info;
}

EXPORT void napalm_seek_to_time(Player *player, RationalValue time){
	player->seek(to_rational(time));
}

EXPORT void *napalm_get_front_cover(Player *player, std::int32_t playlist_position, std::int32_t *data_size){
	if (playlist_position < 0)
		return nullptr;
	size_t size;
	auto ret = player->get_front_cover(playlist_position, size);
	if (size > (size_t)std::numeric_limits<std::int32_t>::max())
		size = std::numeric_limits<std::int32_t>::max();
	*data_size = (int)size;
	return ret;
}

EXPORT void napalm_release_front_cover(Player *player, void *buffer){
	player->release_front_cover(buffer);
}

EXPORT void napalm_get_outputs(Player *player, OutputDeviceList *dst){
	*dst = player->get_outputs();
}

EXPORT void napalm_get_selected_output(Player *player, std::uint8_t *dst){
	auto output = player->get_selected_output();
	memcpy(dst, output.data, sizeof(output.data));
}

EXPORT void napalm_select_output(Player *player, const std::uint8_t *dst){
	player->select_output(dst);
}

EXPORT void napalm_set_volume(Player *player, double volume){
	player->set_volume(volume);
}
