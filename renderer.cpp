#include "renderer.h"

#ifdef _DEBUG
bool Renderer::areLayersSupported(const std::vector<const char*>& layers)
{
	uint32_t supported_layers_count;
	if (vkEnumerateInstanceLayerProperties(&supported_layers_count, nullptr) != VK_SUCCESS) {
		return false;
	}

	std::vector<VkLayerProperties> supported_layers(supported_layers_count);
	if (vkEnumerateInstanceLayerProperties(&supported_layers_count, supported_layers.data()) != VK_SUCCESS) {
		return false;
	}

	for (const char* layer : layers) {
		bool found = false;

		for (const VkLayerProperties& supported_layer : supported_layers) {
			if (std::string(layer) == std::string(supported_layer.layerName)) {
				found = true;
				break;
			}
		}

		if (!found) {
			return false;
		}
	}

	return true;
}
#endif

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

	uint32_t vk_version = volkGetInstanceVersion();
	if (vk_version == 0) {
		out_error_message = "Failed to get Vulkan version.";
		return false;
	}

	if ((VK_API_VERSION_VARIANT(vk_version) != 0) ||
		(VK_API_VERSION_MAJOR(vk_version) != 1) ||
		(VK_API_VERSION_MINOR(vk_version) > 3)) {
		out_error_message = "Unsupported Vulkan version:" +
			std::to_string(VK_API_VERSION_MAJOR(vk_version)) +
			"." + std::to_string(VK_API_VERSION_MINOR(vk_version)) +
			"." + std::to_string(VK_API_VERSION_PATCH(vk_version)) +
			":" + std::to_string(VK_API_VERSION_VARIANT(vk_version));
		return false;
	}

#ifdef _DEBUG
	std::vector<const char*> layers;
	layers.push_back("VK_LAYER_KHRONOS_validation");

	if (!areLayersSupported(layers)) {
		out_error_message = "The necessary Vulkan layers are not supported.";
		return false;
}
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
#ifdef _DEBUG
	inst_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
	inst_info.ppEnabledLayerNames = layers.data();
#else
	inst_info.enabledLayerCount = 0;
	inst_info.ppEnabledLayerNames = nullptr;
#endif
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