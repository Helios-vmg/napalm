#pragma once

#include <boost/rational.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <array>

typedef boost::rational<std::int64_t> rational_t;

struct output_device{
	std::string name;
	std::array<std::uint8_t, 32> unique_id;
};

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
	rational_t current_time();
	std::vector<output_device> get_output_devices();
	void open_output_device(const std::array<std::uint8_t, 32> &);
};

std::string format_time(rational_t time);
std::string absolute_format_time(const rational_t &time);
