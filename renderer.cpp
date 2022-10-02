#include "renderer.h"

bool Renderer::Init(std::string& out_error_message)
{
	if (volkInitialize() != VK_SUCCESS) {
		out_error_message = "Vulkan not found on this system.";
		return false;
	}

	uint32_t supported_vk_version = volkGetInstanceVersion();
	if (supported_vk_version == 0) {
		out_error_message = "Failed to read supported Vulkan version.";
		return false;
	}

	if ((VK_VERSION_MAJOR(supported_vk_version) != 1) || (VK_VERSION_MINOR(supported_vk_version) > 3)) {
		out_error_message = "Unsupported Vulkan version.";
		return false;
	}

	/*VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.pApplicationName = "Simulator";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "none";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_MAKE_VERSION(1, 3, 0);*/
}