#pragma once

#include <Volk/volk.h>
#include <string>
#include <vector>

class Renderer {
public:
#ifdef _DEBUG
	static bool areLayersSupported(const std::vector<const char*>& layers);
#endif
	~Renderer();
	bool init(std::string& out_error_message);
	void destroy();

private:
	bool m_initialized = false;
	VkInstance m_vk_instance = nullptr;
};
