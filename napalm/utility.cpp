#include "utility.h"
#include "../common/audio_format.h"
#include "../common/utf8.cpp"

bool valid_format(const AudioFormat &af){
	return af.format != Invalid && af.channels >= 0 && af.freq;
}
