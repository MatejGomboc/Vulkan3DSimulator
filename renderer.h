#pragma once

#include <Volk/volk.h>
#include <string>

class Renderer {
public:
	~Renderer();
	bool init(std::string& out_error_message);
	void destroy();

private:
	bool m_initialized = false;
	VkInstance m_vk_instance = nullptr;
};
