#pragma once

#include <Volk/volk.h>
#include <string>
#include <vector>

namespace Simulator {
	class Renderer {
	public:
		~Renderer();
		bool init(
			std::string& out_error_message
#ifdef DEBUG
			, PFN_vkDebugUtilsMessengerCallbackEXT vulkan_debug_callback, void* vulkan_debug_callback_user_data
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
			, HINSTANCE app_instance, HWND window
#endif
		);

		void destroy();
		bool getSupportedDevices(
			std::vector<VkPhysicalDevice>& out_supported_devices,
			std::string& out_error_message
		);

	private:
#ifdef DEBUG
		static bool areLayersSupported(const std::vector<const char*>& layers);
#endif
		static bool areExtensionsSupported(const std::vector<const char*>& extensions);

		bool m_initialized = false;
		VkInstance m_vk_instance = VK_NULL_HANDLE;
#ifdef DEBUG
		VkDebugUtilsMessengerEXT m_vk_debug_messenger = VK_NULL_HANDLE;
#endif
		VkSurfaceKHR m_vk_surface = VK_NULL_HANDLE;
	};
}
