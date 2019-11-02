#pragma once


#include "Oblivion.h"
#include "Interfaces/IGraphicsObject.h"


class VulkanDebug : public ISingletone<VulkanDebug>, public IVulkanInstanceObject
{
public:
	VulkanDebug();
	~VulkanDebug();

private:
	vk::DebugReportCallbackEXT m_debugCallback;
	struct
	{
		PFN_vkCreateDebugReportCallbackEXT  vkCreateDebugReportCallbackEXT;
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
	} m_loadedFunctions;

};