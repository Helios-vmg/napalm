#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <Player.h>
#include <RationalValue.h>

#if 0
#include <boost/regex.hpp>

template <typename T>
int to_integer(const T &x){
	int ret = 0;
	for (auto &i : x){
		ret *= 10;
		ret += i - '0';
	}
	return ret;
}

template <typename T>
double to_double(const T &x){
	double ret = 0;
	double multiplier = 1;
	for (auto &i : x){
		multiplier /= 10;
		ret += (i - '0') * multiplier;
	}
	return ret;
}

double parse_time(const std::string &time_string){
	static boost::regex s   ("^(\\d+)$");
	static boost::regex sd  ("^(\\d+)\\.(\\d+)$");
	static boost::regex ms  ("^(\\d+)\\:(\\d{2})$");
	static boost::regex msd ("^(\\d+)\\:(\\d{2})\\.(\\d+)$");
	static boost::regex hms ("^(\\d+)\\:(\\d{2})\\:(\\d{2})$");
	static boost::regex hmsd("^(\\d+)\\:(\\d{2})\\:(\\d{2})\\.(\\d+)$");

	boost::smatch match;
	if (boost::regex_search(time_string, match, s))
		return to_integer(match[1]);
	if (boost::regex_search(time_string, match, sd))
		return to_integer(match[1]) + to_double(match[2]);
	if (boost::regex_search(time_string, match, ms))
		return to_integer(match[1]) * 60 + to_integer(match[2]);
	if (boost::regex_search(time_string, match, msd))
		return to_integer(match[1]) * 60 + to_integer(match[2]) + to_double(match[3]);
	if (boost::regex_search(time_string, match, hms))
		return (to_integer(match[1]) * 60 + to_integer(match[2])) * 60 + to_integer(match[3]);
	if (boost::regex_search(time_string, match, hmsd))
		return (to_integer(match[1]) * 60 + to_integer(match[2])) * 60 + to_integer(match[3]) + to_double(match[4]);
	return std::nan("");
}
#else
double parse_time(const char *time_string){
	const static double bad = std::nan("");

	double ret = 0;
	double multiplier = 1;
	int fragment = 0;
	int colons = 0;
	int fragment_length = 0;
	for (auto p = time_string; *p; p++){
		auto c = *p;
		switch (c){
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (multiplier == 1){
					fragment *= 10;
					fragment += c - '0';
				}else{
					ret += (c - '0') * multiplier;
					multiplier /= 10;
				}
				fragment_length++;
				break;
			case ':':
				if (colons == 2 || multiplier != 1 || colons && fragment_length != 2)
					return bad;
				colons++;
				ret += fragment;
				ret *= 60;
				fragment = 0;
				fragment_length = 0;
				break;
			case '.':
				if (multiplier != 1)
					return bad;
				ret += fragment;
				multiplier /= 10;
				fragment_length = 0;
				break;
		}
	}
	if (!fragment_length)
		return bad;
	if (multiplier == 1){
		if (colons && fragment_length != 2)
			return bad;
		ret += fragment;
	}
	return ret;
}

double parse_time(const std::string &time_string){
	return parse_time(time_string.c_str());
}
#endif

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
					"output\n"
					"load\n"
					"play\n"
					"pause\n"
					"seek\n"
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
			}else if (line == "seek"){
				std::cout << "Time: ";
				std::getline(std::cin, line);
				auto time = parse_time(line);
				if (isnan(time)){
					std::cout << "Invalid time string.\n";
					continue;
				}
				player.seek(to_rational<std::int64_t, (std::int64_t)1 << 32>(time));
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
