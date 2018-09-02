#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
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
					"get_outputs\n"
					"load\n"
					"play\n"
					"pause\n"
					"stop\n"
					"time\n";
			}else if (line == "output"){
				auto outputs = player.get_output_devices();
				if (!outputs.size()){
					std::cout << "There are no output devices available!\n";
					return 0;
				}
				size_t i = 1;
				size_t j = 0;
				do{
					for (auto &o : outputs){
						std::stringstream stream;
						stream << std::setw(2) << i++ << ". " << o.name;
						std::cout << stream.str() << std::endl;
					}
					std::getline(std::cin, line);
					std::stringstream stream(line);
					stream >> j;
				}while (j < 1 || j > outputs.size());
				player.open_output_device(outputs[j - 1].unique_id);
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
