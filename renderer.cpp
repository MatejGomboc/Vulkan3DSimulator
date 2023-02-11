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
	std::vector<const char*> vk_layers{
		VK_LAYER_KHRONOS_VALIDATION_NAME
	};

	if (!areInstanceLayersSupported(vk_layers, out_error_message)) {
		destroy();
		return false;
	}
#endif

	std::vector<const char*> vk_extensions{
		 VK_KHR_SURFACE_EXTENSION_NAME
#ifdef VK_USE_PLATFORM_WIN32_KHR
		 , VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif
#ifdef DEBUG
		 , VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
	};

	if (!areInstanceExtensionsSupported(vk_extensions, out_error_message)) {
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

	volkLoadInstanceOnly(m_vk_instance);

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
	if (m_vk_logical_device != VK_NULL_HANDLE) {
		vkDestroyDevice(m_vk_logical_device, nullptr);
		m_vk_logical_device = VK_NULL_HANDLE;
	}

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

bool Renderer::getSupportedPhysicalDevices(std::vector<VkPhysicalDevice>& out_supported_devices, std::string& out_error_message)
{
	if (!m_initialized) {
		out_error_message = "Renderer not initialized.";
		return false;
	}

	uint32_t devices_count;
	VkResult vk_error = vkEnumeratePhysicalDevices(m_vk_instance, &devices_count, nullptr);
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to enumerate Vulkan physical devices. VK error:" + std::to_string(vk_error) + ".";
		return false;
	}

	if (devices_count == 0) {
		out_error_message = "No Vulkan physical devices found.";
		return false;
	}

	std::vector<VkPhysicalDevice> devices(devices_count);
	vk_error = vkEnumeratePhysicalDevices(m_vk_instance, &devices_count, devices.data());
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to enumerate Vulkan physical devices. VK error:" + std::to_string(vk_error) + ".";
		return false;
	}

	for (const VkPhysicalDevice& device : devices) {
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(device, &device_properties);

		uint32_t queue_families_count;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count, nullptr);

		if (queue_families_count == 0) {
			out_supported_devices.clear();
			out_error_message = "No Vulkan queue families found for physical device \"" + std::string(device_properties.deviceName) + "\".";
			return false;
		}

		std::vector<VkQueueFamilyProperties> queue_families_props(queue_families_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count, queue_families_props.data());

		bool graphics_queue_family_found = false;
		bool present_queue_family_found = false;

		for (uint32_t i = 0; i < queue_families_props.size(); i++) {
			if (queue_families_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				graphics_queue_family_found = true;
			}

			VkBool32 supported = false;
			vk_error = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_vk_surface, &supported);
			if (vk_error != VK_SUCCESS) {
				out_supported_devices.clear();
				out_error_message = "Failed to query Vulkan physical device \"" + std::string(device_properties.deviceName) +
					"\" for presentation support. VK error:" + std::to_string(vk_error) + ".";
				return false;
			}

			if (supported) {
				present_queue_family_found = true;
			}

			if (graphics_queue_family_found && present_queue_family_found) {
				break;
			}
		}

		if (graphics_queue_family_found && present_queue_family_found) {
			out_supported_devices.push_back(device);
		}
	}

	if (out_supported_devices.empty()) {
		out_error_message = "No supported Vulkan physical devices found.";
		return false;
	}

	return true;
}

bool Renderer::createLogicalDevice(const VkPhysicalDevice& physical_device, std::string& out_error_message)
{
	if (!m_initialized) {
		out_error_message = "Renderer not initialized.";
		return false;
	}

	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(physical_device, &device_properties);

	uint32_t queue_families_count;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, nullptr);

	if (queue_families_count == 0) {
		out_error_message = "No Vulkan queue families found for physical device \"" + std::string(device_properties.deviceName) + "\".";
		return false;
	}

	std::vector<VkQueueFamilyProperties> queue_families_props(queue_families_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, queue_families_props.data());

	bool graphics_queue_family_found = false;
	bool present_queue_family_found = false;

	uint32_t graphics_queue_family_idx = 0;
	uint32_t present_queue_family_idx = 0;

	for (uint32_t i = 0; i < queue_families_props.size(); i++) {
		if (queue_families_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphics_queue_family_idx = i;
			graphics_queue_family_found = true;
		}

		VkBool32 supported = false;
		VkResult vk_error = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, m_vk_surface, &supported);
		if (vk_error != VK_SUCCESS) {
			out_error_message = "Failed to query Vulkan physical device \"" + std::string(device_properties.deviceName) +
				"\" for presentation support. VK error:" + std::to_string(vk_error) + ".";
			return false;
		}

		if (supported) {
			present_queue_family_idx = i;
			present_queue_family_found = true;
		}

		if (graphics_queue_family_found && present_queue_family_found) {
			break;
		}
	}

	if (!(graphics_queue_family_found && present_queue_family_found)) {
		out_error_message = "No Vulkan supported queue families found for physical device \"" + std::string(device_properties.deviceName) + "\".";
		return false;
	}

	float device_queue_priority = 1.0f;
	VkDeviceQueueCreateInfo device_queue_create_info{};
	device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.pNext = nullptr;
	device_queue_create_info.flags = 0;
	device_queue_create_info.queueFamilyIndex = graphics_queue_family_idx;
	device_queue_create_info.queueCount = 1;
	device_queue_create_info.pQueuePriorities = &device_queue_priority;

	std::vector<VkDeviceQueueCreateInfo> device_queue_create_infos{ device_queue_create_info };
	if (present_queue_family_idx != graphics_queue_family_idx) {
		device_queue_create_info.queueFamilyIndex = present_queue_family_idx;
		device_queue_create_infos.push_back(device_queue_create_info);
	}

#ifdef DEBUG
	std::vector<const char*> vk_layers{
		VK_LAYER_KHRONOS_VALIDATION_NAME
	};

	if (!areDeviceLayersSupported(physical_device, vk_layers, out_error_message)) {
		destroy();
		return false;
	}
#endif

	VkPhysicalDeviceFeatures enabled_device_features{};

	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pNext = nullptr;
	device_create_info.flags = 0;
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(device_queue_create_infos.size());
	device_create_info.pQueueCreateInfos = device_queue_create_infos.data();
#ifdef DEBUG
	device_create_info.enabledLayerCount = static_cast<uint32_t>(vk_layers.size());
	device_create_info.ppEnabledLayerNames = vk_layers.data();
#else
	device_create_info.enabledLayerCount = 0;
	device_create_info.ppEnabledLayerNames = nullptr;
#endif
	device_create_info.enabledExtensionCount = 0;
	device_create_info.ppEnabledExtensionNames = nullptr;
	device_create_info.pEnabledFeatures = &enabled_device_features;

	VkResult vk_error = vkCreateDevice(physical_device, &device_create_info, nullptr, &m_vk_logical_device);
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to create Vulkan logical device. VK error:" + std::to_string(vk_error) + ".";
		destroy();
		return false;
	}

	volkLoadDevice(m_vk_logical_device);

	return true;
}

#ifdef DEBUG
bool Renderer::areInstanceLayersSupported(const std::vector<const char*>& layers, std::string& out_error_message)
{
	uint32_t supported_layers_count;
	VkResult vk_error = vkEnumerateInstanceLayerProperties(&supported_layers_count, nullptr);
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to enumerate Vulkan instance layers. VK error:" + std::to_string(vk_error) + ".";
		return false;
	}

	if (supported_layers_count == 0) {
		out_error_message = "No Vulkan instance layers found.";
		return false;
	}

	std::vector<VkLayerProperties> supported_layers(supported_layers_count);
	vk_error = vkEnumerateInstanceLayerProperties(&supported_layers_count, supported_layers.data());
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to enumerate Vulkan instance layers. VK error:" + std::to_string(vk_error) + ".";
		return false;
	}

	for (const char* const layer : layers) {
		bool found = false;

		for (const VkLayerProperties& supported_layer : supported_layers) {
			if (std::string(layer) == std::string(supported_layer.layerName)) {
				found = true;
				break;
			}
		}

		if (!found) {
			out_error_message = "Vulkan instance layer \"" + std::string(layer) + "\" not supported.";
			return false;
		}
	}

	return true;
}

bool Renderer::areDeviceLayersSupported(const VkPhysicalDevice& physical_device, const std::vector<const char*>& layers, std::string& out_error_message)
{
	uint32_t supported_layers_count;
	VkResult vk_error = vkEnumerateDeviceLayerProperties(physical_device, &supported_layers_count, nullptr);
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to enumerate Vulkan device layers. VK error:" + std::to_string(vk_error) + ".";
		return false;
	}

	if (supported_layers_count == 0) {
		out_error_message = "No Vulkan layers found.";
		return false;
	}

	std::vector<VkLayerProperties> supported_layers(supported_layers_count);
	vk_error = vkEnumerateDeviceLayerProperties(physical_device, &supported_layers_count, supported_layers.data());
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to enumerate Vulkan device layers. VK error:" + std::to_string(vk_error) + ".";
		return false;
	}

	for (const char* const layer : layers) {
		bool found = false;

		for (const VkLayerProperties& supported_layer : supported_layers) {
			if (std::string(layer) == std::string(supported_layer.layerName)) {
				found = true;
				break;
			}
		}

		if (!found) {
			out_error_message = "Vulkan device layer \"" + std::string(layer) + "\" not supported.";
			return false;
		}
	}

	return true;
}
#endif

bool Renderer::areInstanceExtensionsSupported(const std::vector<const char*>& extensions, std::string& out_error_message)
{
	uint32_t supported_extensions_count;
	VkResult vk_error = vkEnumerateInstanceExtensionProperties(nullptr, &supported_extensions_count, nullptr);
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to enumerate Vulkan instance extensions. VK error:" + std::to_string(vk_error) + ".";
		return false;
	}

	if (supported_extensions_count == 0) {
		out_error_message = "No Vulkan instance extensions found.";
		return false;
	}

	std::vector<VkExtensionProperties> supported_extensions(supported_extensions_count);
	vk_error = vkEnumerateInstanceExtensionProperties(nullptr, &supported_extensions_count, supported_extensions.data());
	if (vk_error != VK_SUCCESS) {
		out_error_message = "Failed to enumerate Vulkan instance extensions. VK error:" + std::to_string(vk_error) + ".";
		return false;
	}

	for (const char* const extension : extensions) {
		bool found = false;

		for (const VkExtensionProperties& supported_extension : supported_extensions) {
			if (std::string(extension) == std::string(supported_extension.extensionName)) {
				found = true;
				break;
			}
		}

		if (!found) {
			out_error_message = "Vulkan instance extension \"" + std::string(extension) + "\" not supported.";
			return false;
		}
	}

	return true;
}