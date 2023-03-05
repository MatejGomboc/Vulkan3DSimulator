#pragma once

#include <Volk/volk.h>
#include <string>
#include <vector>

namespace Simulator {
	class Renderer {
	public:
		~Renderer();
		bool init(
			std::string& out_error_message, HINSTANCE app_instance, HWND window
#ifdef DEBUG
			, PFN_vkDebugUtilsMessengerCallbackEXT vulkan_debug_callback, void* vulkan_debug_callback_user_data
#endif
		);
		void destroy();
		bool getSupportedPhysicalDevices(std::vector<VkPhysicalDevice>& out_supported_devices, std::string& out_error_message);
		bool createLogicalDevice(const VkPhysicalDevice& physical_device, std::string& out_error_message);

	private:
#ifdef DEBUG
		static bool areDeviceLayersSupported(const VkPhysicalDevice& physical_device, const std::vector<const char*>& layers, std::string& out_error_message);
#endif
		static bool areInstanceExtensionsSupported(const std::vector<const char*>& extensions, std::string& out_error_message);
		static bool areDeviceExtensionsSupported(const VkPhysicalDevice& physical_device, const std::vector<const char*>& extensions, std::string& out_error_message);

#ifdef DEBUG
		static constexpr const char* const VK_LAYER_KHRONOS_VALIDATION_NAME = "VK_LAYER_KHRONOS_validation";
#endif

		bool m_initialized = false;
		VkInstance m_vk_instance = VK_NULL_HANDLE;
#ifdef DEBUG
		VkDebugUtilsMessengerEXT m_vk_debug_messenger = VK_NULL_HANDLE;
#endif
		VkSurfaceKHR m_vk_surface = VK_NULL_HANDLE;
		VkDevice m_vk_logical_device = VK_NULL_HANDLE;
	};
}
