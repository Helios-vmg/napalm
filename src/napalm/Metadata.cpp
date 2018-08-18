#include "Metadata.h"
#include <sstream>

int GenericMetadata::track_number_int(){
	std::stringstream stream(this->track_number());
	int ret;
	if (!(stream >> ret))
		ret = -1;
	return ret;
}

std::string GenericMetadata::track_id(){
	std::string ret = this->album();
	ret += " (";
	ret += this->date();
	ret += ") - ";
	ret += this->track_number();
	ret += " - ";
	ret += this->track_artist();
	ret += " - ";
	ret += this->track_title();
	return ret;
}
