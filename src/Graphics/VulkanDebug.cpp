#include "VulkanDebug.h"
#include "VulkanRenderer.h"


VkBool32 DebugCallback(
	VkDebugReportFlagsEXT                       flags,
	VkDebugReportObjectTypeEXT                  objectType,
	uint64_t                                    object,
	size_t                                      location,
	int32_t                                     messageCode,
	const char*                                 pLayerPrefix,
	const char*                                 pMessage,
	void*                                       pUserData)
{
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		ERROR(appendToString("[", pLayerPrefix, "] [Vk#", messageCode, "]: ", pMessage));
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		WARNING(appendToString("[", pLayerPrefix, "] [Vk#", messageCode, "]: ", pMessage));
	else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		NOTE(appendToString("[", pLayerPrefix, "] [Vk#", messageCode, "]: ", pMessage));
	else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		WARNING(appendToString("[", pLayerPrefix, "] [Vk#", messageCode, "]: ", pMessage));
	else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		NOTE(appendToString("[", pLayerPrefix, "] [Vk#", messageCode, "]: ", pMessage));
	return VK_FALSE;
}

VulkanDebug::VulkanDebug()
{
	m_loadedFunctions.vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)m_vulkanInstance.getProcAddr("vkCreateDebugReportCallbackEXT");
	if (!m_loadedFunctions.vkCreateDebugReportCallbackEXT)
		THROW_INITIALIZATION_EXCEPTION("Couldn't load vkCreateDebugReportCallbackEXT");

	m_loadedFunctions.vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)m_vulkanInstance.getProcAddr("vkDestroyDebugReportCallbackEXT");
	if (!m_loadedFunctions.vkDestroyDebugReportCallbackEXT)
		THROW_INITIALIZATION_EXCEPTION("Couldn't load vkDestroyDebugReportCallbackEXT");

	vk::DebugReportCallbackCreateInfoEXT callbackInfo;
	callbackInfo.setFlags(vk::DebugReportFlagBitsEXT::eDebug | vk::DebugReportFlagBitsEXT::eError |
		vk::DebugReportFlagBitsEXT::eInformation | vk::DebugReportFlagBitsEXT::ePerformanceWarning | vk::DebugReportFlagBitsEXT::eWarning)
		.setPfnCallback(DebugCallback);


	m_debugCallback = m_vulkanInstance.createDebugReportCallbackEXT(callbackInfo, nullptr, m_loadedFunctions);
	if (!m_debugCallback)
		THROW_INITIALIZATION_EXCEPTION("Couldn't create a debug callback");
}


VulkanDebug::~VulkanDebug()
{
	m_vulkanInstance.destroyDebugReportCallbackEXT(m_debugCallback, nullptr, m_loadedFunctions);
}