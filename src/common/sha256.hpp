#pragma once

#include "sha256.h"
#include <array>
#include <cstdint>
#include <string>

class SHA256{
	SHA256_CTX state;
public:
	SHA256(){
		sha256_init(&this->state);
	}
	void input(const void *buffer, size_t size){
		sha256_update(&this->state, (const std::uint8_t *)buffer, size);
	}
	void input(const std::string &s){
		this->input(&s[0], s.size());
	}
	std::array<std::uint8_t, 32> result(){
		std::array<std::uint8_t, 32> ret;
		sha256_final(&this->state, ret.data());
		return ret;
	}
};
