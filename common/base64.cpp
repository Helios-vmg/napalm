#include "base64.hpp"

static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

static void b64_decode_block(unsigned char in[4], unsigned char out[3]){
	out[0] = (in[0] << 2 | in[1] >> 4);
	out[1] = (in[1] << 4 | in[2] >> 2);
	out[2] = (((in[2] << 6) & 0xc0) | in[3]);
}

void b64_decode(std::vector<unsigned char> &dst, const char *src, size_t slen){
	auto size = (slen / 4) * 3;

	dst.resize(size);
	auto d = &dst[0];

	unsigned char in[4] = {0},
		out[3];
	size_t i,
		len,
		pos = 0;

	while (pos <= slen){
		for (len = 0, i = 0; i < 4 && pos <= slen; i++){
			unsigned char v = 0;

			while (pos <= slen && v == 0){
				v = src[pos++];
				v = ((v < 43 || v > 122) ? 0 : cd64[v - 43]);
				if (v)
					v = v == '$' ? 0 : v - 61;
			}

			if (pos <= slen){
				len++;
				if (v)
					in[i] = v - 1;
			}else
				in[i] = 0;
		}

		if (len){
			b64_decode_block(in, out);
			for (i = 0; i < len - 1; i++)
				 *(d++) = out[i];
		}
	}
}
