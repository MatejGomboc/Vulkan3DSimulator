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
#ifdef _DEBUG
			, PFN_vkDebugUtilsMessengerCallbackEXT debug_callback,
			void* debug_callback_user_data
#endif
		);

		void destroy();
		bool getSupportedDevices(
			std::vector<VkPhysicalDevice>& out_supported_devices,
			std::string& out_error_message
		);

	private:
#ifdef _DEBUG
		static bool areLayersSupported(const std::vector<const char*>& layers);
#endif
		static bool areExtensionsSupported(const std::vector<const char*>& extensions);

		bool m_initialized = false;
		VkInstance m_vk_instance = VK_NULL_HANDLE;
#ifdef _DEBUG
		VkDebugUtilsMessengerEXT m_vk_debug_messenger = VK_NULL_HANDLE;
#endif
	};
}
