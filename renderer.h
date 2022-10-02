#pragma once

#include <Volk/volk.h>
#include <string>

class Renderer {
public:
	bool Init(std::string& out_error_message);
};
