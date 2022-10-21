#pragma once

#include <Volk/volk.h>
#include <string>
#include <vector>
#include "logger.h"

namespace Simulator {
	class Renderer {
	public:
		~Renderer();
		bool init(std::string& out_error_message);
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
#ifdef _DEBUG
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
			VkDebugUtilsMessageTypeFlagsEXT message_type,
			const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
			void* user_data
		);
#endif

		bool m_initialized = false;
		Logger m_logger;
		VkInstance m_vk_instance = VK_NULL_HANDLE;
#ifdef _DEBUG
		VkDebugUtilsMessengerEXT m_vk_debug_messenger = VK_NULL_HANDLE;
#endif
	};
}
