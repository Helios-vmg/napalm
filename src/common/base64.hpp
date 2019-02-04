#pragma once

#include <vector>
#include <cstddef>

void b64_decode(std::vector<unsigned char> &dst, const char *src, size_t slen);
