#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

/// <summary>
/// This structure provides information to create a swap chain.
/// </summary>

struct SwapChainSupportInfo
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


SwapChainSupportInfo QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);