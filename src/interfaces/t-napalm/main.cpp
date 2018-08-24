#include <iostream>
#include <string>
#include <sstream>
#include <Player.h>

int main(){
	try{
		Player player;
		while (true){
			std::cout << "> ";
			std::string line;
			std::getline(std::cin, line);
			
			if (line == "exit" || line == "quit")
				break;
			if (line == "help"){
				std::cout <<
					"Commands:\n"
					"load\n"
					"play\n"
					"pause\n"
					"stop\n"
					"time\n";
			}else if (line == "load"){
				std::cout << "Path: ";
				std::getline(std::cin, line);
				bool result = player.load_file(line);
				std::cout << (result ? "OK.\n" : "Failed.\n");
			}else if (line == "play"){
				player.play();
			}else if (line == "pause"){
				player.pause();
			}else if (line == "stop"){
				player.stop();
			}else if (line == "time"){
				auto t = player.current_time();
				std::cout << "Current time: " << absolute_format_time(t) << std::endl;
			}
		}
	}catch (std::exception &e){
		std::cerr << "Error: " << e.what() << std::endl;
		return -1;
	}
	return 0;
}
