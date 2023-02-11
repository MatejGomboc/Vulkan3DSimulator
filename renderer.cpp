#include "renderer.h"

using namespace Simulator;

Renderer::~Renderer()
{
	destroy();
}

bool Renderer::init(
	std::string& out_error_message
#ifdef DEBUG
	, PFN_vkDebugUtilsMessengerCallbackEXT vulkan_debug_callback, void* vulkan_debug_callback_user_data
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
	, HINSTANCE app_instance, HWND window
#endif
)
{
	if (m_initialized) {
		out_error_message = "Renderer already initialized.";
		destroy();
		return false;
	}

	if (volkInitialize() != VK_SUCCESS) {
		out_error_message = "Vulkan not found on this system.";
		destroy();
		return false;
	}

	uint32_t vk_version = volkGetInstanceVersion();
	if (vk_version == 0) {
		out_error_message = "Failed to get Vulkan version.";
		destroy();
		return false;
	}

	if ((VK_API_VERSION_VARIANT(vk_version) != 0) ||
		(VK_API_VERSION_MAJOR(vk_version) != 1) ||
		(VK_API_VERSION_MINOR(vk_version) < 3)) {
		out_error_message = "Unsupported Vulkan version:" +
			std::to_string(VK_API_VERSION_MAJOR(vk_version)) +
			"." + std::to_string(VK_API_VERSION_MINOR(vk_version)) +
			"." + std::to_string(VK_API_VERSION_PATCH(vk_version)) +
			":" + std::to_string(VK_API_VERSION_VARIANT(vk_version));
		destroy();
		return false;
	}

#ifdef DEBUG
	std::vector<const char*> vk_layers;
	vk_layers.push_back("VK_LAYER_KHRONOS_validation");

	if (!areLayersSupported(vk_layers)) {
		out_error_message = "The necessary Vulkan layers are not supported.";
		destroy();
		return false;
	}
#endif

	std::vector<const char*> vk_extensions = {
		 VK_KHR_SURFACE_EXTENSION_NAME
#ifdef VK_USE_PLATFORM_WIN32_KHR
		 , VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
#ifdef DEBUG
		 , VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
	};

	if (!areExtensionsSupported(vk_extensions)) {
		out_error_message = "The necessary Vulkan extensions are not supported.";
		destroy();
		return false;
	}

	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = nullptr;
	app_info.pApplicationName = "Simulator";
	app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
	app_info.pEngineName = "none";
	app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
	app_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

#ifdef DEBUG
	VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info{};
	debug_messenger_info.sType =
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debug_messenger_info.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debug_messenger_info.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debug_messenger_info.pfnUserCallback = vulkan_debug_callback;
	debug_messenger_info.pUserData = vulkan_debug_callback_user_data;
#endif

	VkInstanceCreateInfo inst_info{};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifdef DEBUG
	inst_info.pNext = &debug_messenger_info;
#else
	inst_info.pNext = nullptr;
#endif
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
#ifdef DEBUG
	inst_info.enabledLayerCount = static_cast<uint32_t>(vk_layers.size());
	inst_info.ppEnabledLayerNames = vk_layers.data();
#else
	inst_info.enabledLayerCount = 0;
	inst_info.ppEnabledLayerNames = nullptr;
#endif
	inst_info.enabledExtensionCount = static_cast<uint32_t>(vk_extensions.size());
	inst_info.ppEnabledExtensionNames = vk_extensions.data();

	VkResult vk_error = vkCreateInstance(&inst_info, nullptr, &m_vk_instance);
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to create Vulkan instance. VK error:" + std::to_string(vk_error) + ".";
		destroy();
		return false;
	}

	volkLoadInstance(m_vk_instance);

#ifdef DEBUG
	vk_error = vkCreateDebugUtilsMessengerEXT(m_vk_instance, &debug_messenger_info, nullptr, &m_vk_debug_messenger);
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to create Vulkan debug messenger. VK error:" + std::to_string(vk_error) + ".";
		destroy();
		return false;
	}
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.hinstance = app_instance;
	createInfo.hwnd = window;

	vk_error = vkCreateWin32SurfaceKHR(m_vk_instance, &createInfo, nullptr, &m_vk_surface);
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to create Vulkan rendering surface. VK error:" + std::to_string(vk_error) + ".";
		destroy();
		return false;
	}
#endif

	m_initialized = true;
	return true;
}

void Renderer::destroy()
{
	if ((m_vk_instance != VK_NULL_HANDLE) && (m_vk_surface != VK_NULL_HANDLE)) {
		vkDestroySurfaceKHR(m_vk_instance, m_vk_surface, nullptr);
		m_vk_surface = VK_NULL_HANDLE;
	}

#ifdef DEBUG
	if ((m_vk_instance != VK_NULL_HANDLE) && (m_vk_debug_messenger != VK_NULL_HANDLE)) {
		vkDestroyDebugUtilsMessengerEXT(m_vk_instance, m_vk_debug_messenger, nullptr);
		m_vk_debug_messenger = VK_NULL_HANDLE;
	}
#endif

	if (m_vk_instance != VK_NULL_HANDLE) {
		vkDestroyInstance(m_vk_instance, nullptr);
		m_vk_instance = VK_NULL_HANDLE;
	}

	m_initialized = false;
}

bool Renderer::getSupportedDevices(std::vector<VkPhysicalDevice>& out_supported_devices, std::string& out_error_message)
{
	if (!m_initialized) {
		out_error_message = "Renderer not initialized.";
		return false;
	}

	uint32_t devices_count;
	VkResult vk_error = vkEnumeratePhysicalDevices(m_vk_instance, &devices_count, nullptr);
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to enumerate physical devices. VK error:" + std::to_string(vk_error) + ".";
		return false;
	}

	if (devices_count == 0) {
		return true;
	}

	std::vector<VkPhysicalDevice> devices(devices_count);
	vk_error = vkEnumeratePhysicalDevices(m_vk_instance, &devices_count, devices.data());
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to enumerate physical devices. VK error:" + std::to_string(vk_error) + ".";
		return false;
	}

	for (const VkPhysicalDevice& device : devices) {
		//VkPhysicalDeviceProperties device_properties;
		//vkGetPhysicalDeviceProperties(device, &device_properties);

		//VkPhysicalDeviceFeatures device_features;
		//vkGetPhysicalDeviceFeatures(device, &device_features);

		uint32_t queue_families_count;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families_props(queue_families_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count, queue_families_props.data());

		for (const VkQueueFamilyProperties& queue_family_props : queue_families_props) {
			if (queue_family_props.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				out_supported_devices.push_back(device);
				break;
			}
		}
	}

	return true;
}

#ifdef DEBUG
bool Renderer::areLayersSupported(const std::vector<const char*>& layers)
{
	uint32_t supported_layers_count;
	VkResult error = vkEnumerateInstanceLayerProperties(&supported_layers_count, nullptr);
	if (error != VK_SUCCESS) {
		return false;
	}

	std::vector<VkLayerProperties> supported_layers(supported_layers_count);
	error = vkEnumerateInstanceLayerProperties(&supported_layers_count, supported_layers.data());
	if (error != VK_SUCCESS) {
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

bool Renderer::areExtensionsSupported(const std::vector<const char*>& extensions)
{
	uint32_t supported_extensions_count;
	VkResult error = vkEnumerateInstanceExtensionProperties(nullptr, &supported_extensions_count, nullptr);
	if (error != VK_SUCCESS) {
		return false;
	}

	std::vector<VkExtensionProperties> supported_extensions(supported_extensions_count);
	error = vkEnumerateInstanceExtensionProperties(nullptr, &supported_extensions_count, supported_extensions.data());
	if (error != VK_SUCCESS) {
		return false;
	}

	for (const char* extension : extensions) {
		bool found = false;

		for (const VkExtensionProperties& supported_extension : supported_extensions) {
			if (std::string(extension) == std::string(supported_extension.extensionName)) {
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