#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

/// <summary>
/// This structure finds & holds the required queue families.
/// </summary>

struct QueueFamilyIndices
{
	// Create an index for each family type we need
	// Those are: 
	//	-> Present: For Queues supporting image presentation to a window
	//	-> Compute: For Queues supporting computing operations

	int presentFamily = -1;
	int computeFamily = -1;

	bool IsComplete();
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);