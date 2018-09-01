#pragma once

#include <output_module.h>
#include <string>
#include <vector>
#include <cstdint>

class PrivateOutputDeviceList{
	std::vector<std::string> display_names;
	std::vector<OutputDeviceListItem> devices;
	OutputDeviceList list;
public:
	PrivateOutputDeviceList(){
		this->list.opaque = this;
	}
	OutputDeviceList get_list(){
		assert(this->display_names.size() == this->devices.size());
		for (size_t i = 0; i < this->display_names.size(); i++)
			this->devices[i].name = this->display_names[i].c_str();
		this->list.length = (std::uint32_t)this->devices.size();
		this->list.items = &this->devices[0];
		return this->list;
	}
	void add(const std::string &name, const std::uint8_t (&unique_id)[sizeof(OutputDeviceListItem().unique_id)]){
		this->display_names.push_back(name);
		this->devices.push_back({});
		memcpy(this->devices.back().unique_id.unique_id, unique_id, sizeof(unique_id));
	}
};
