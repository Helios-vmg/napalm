#include "Player.h"
#include <iostream>

int main(){
	try{
		Player player;
		player.load_file("test.ogg");
		player.play();
	}catch (std::exception &e){
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
