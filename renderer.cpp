#include "renderer.h"
#include <vector>

Renderer::~Renderer()
{
	destroy();
}

bool Renderer::init(std::string& out_error_message)
{
	if (m_initialized) {
		out_error_message = "Renderer already initialized.";
		return false;
	}

	if (volkInitialize() != VK_SUCCESS) {
		out_error_message = "Vulkan not found on this system.";
		return false;
	}

	uint32_t supported_vk_version = volkGetInstanceVersion();
	if (supported_vk_version == 0) {
		out_error_message = "Failed to read supported Vulkan version.";
		return false;
	}

	if (VK_VERSION_MAJOR(supported_vk_version) != 1) {
		out_error_message = "Unsupported Vulkan version.";
		return false;
	}

	std::vector<const char*> layers;
#if defined(_DEBUG)
	layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

	std::vector<const char*> extensions = {
		 VK_KHR_SURFACE_EXTENSION_NAME,
		 VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = nullptr;
	app_info.pApplicationName = "Simulator";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "none";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo inst_info{};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = nullptr;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
	inst_info.ppEnabledLayerNames = layers.data();
	inst_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	inst_info.ppEnabledExtensionNames = extensions.data();

	VkResult vk_error = vkCreateInstance(&inst_info, nullptr, &m_vk_instance);
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to create Vulkan instance. VK error:" + std::to_string(vk_error) + ".";
		return false;
	}

	volkLoadInstance(m_vk_instance);
	m_initialized = true;
	return true;
}

void Renderer::destroy()
{
	if (m_vk_instance != nullptr) {
		vkDestroyInstance(m_vk_instance, nullptr);
		m_vk_instance = nullptr;
	}

	m_initialized = false;
}