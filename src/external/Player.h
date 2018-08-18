#pragma once

#include <boost/rational.hpp>
#include <cstdint>
#include <memory>
#include <string>

typedef boost::rational<std::int64_t> rational_t;

class Player{
	class Pimpl;
	std::unique_ptr<Pimpl> pimpl;
public:
	Player();
	~Player();
	bool load_file(const std::wstring &path);
	bool load_file(const std::string &path);
	void play();
	void pause();
	void stop();
	std::pair<rational_t, rational_t> current_time();
};

std::string format_time(rational_t time);
std::string absolute_format_time(const rational_t &time);
