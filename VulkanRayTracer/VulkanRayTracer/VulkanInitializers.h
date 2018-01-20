#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

/// <summary>
/// Provides initializers for common Vulkan structures.
/// </summary>

namespace Initializers
{
	inline VkApplicationInfo ApplicationInfo(const char* appName, uint32_t appVersion, uint32_t apiVersion)
	{
		VkApplicationInfo result {};
		result.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		result.pApplicationName = appName;
		result.applicationVersion = appVersion;
		result.apiVersion = apiVersion;

		return result;
	}

	inline VkInstanceCreateInfo InstanceCreateInfo(const VkApplicationInfo appInfo)
	{
		VkInstanceCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		result.pApplicationInfo = &appInfo;

		return result;
	}


	inline VkDeviceQueueCreateInfo DeviceQueueCreateInfo(uint32_t queueFamily, const float queuePriority, uint32_t queueCount = 1)
	{
		VkDeviceQueueCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		result.queueFamilyIndex = queueFamily;
		result.queueCount = queueCount;
		result.pQueuePriorities = &queuePriority;

		return result;
	}

	inline VkDeviceCreateInfo DeviceCreateInfo()
	{
		VkDeviceCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		return result;
	}

	inline VkSwapchainCreateInfoKHR SwapchainCreateInfoKHR(VkSurfaceKHR surface)
	{
		VkSwapchainCreateInfoKHR result {};
		result.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		result.surface = surface;

		return result;
	}

	inline VkImageViewCreateInfo ImageViewCreateInfo(VkImage image, VkImageViewType viewType)
	{
		VkImageViewCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		result.image = image;
		result.viewType = viewType;

		return result;
	}

	inline VkImageCreateInfo ImageCreateInfo(VkImageType imageType)
	{
		VkImageCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		result.imageType = imageType;

		return result;
	}

	inline VkMemoryAllocateInfo MemoryAllocateInfo(VkDeviceSize allocationSize, uint32_t memTypeIndex)
	{
		VkMemoryAllocateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		result.allocationSize = allocationSize;
		result.memoryTypeIndex = memTypeIndex;

		return result;
	}


	inline VkDescriptorPoolSize DescriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount)
	{
		VkDescriptorPoolSize result {};
		result.type = type;
		result.descriptorCount = descriptorCount;

		return result;
	}

	inline VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo()
	{
		VkDescriptorPoolCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

		return result;
	}


	inline VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding, uint32_t descriptorCount = 1)
	{
		VkDescriptorSetLayoutBinding result{};
		result.descriptorType = type;
		result.stageFlags = stageFlags;
		result.descriptorCount = descriptorCount;
		result.binding = binding;

		return result;
	}

	inline VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo()
	{
		VkDescriptorSetLayoutCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

		return result;
	}

	inline VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo()
	{
		VkPipelineLayoutCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		return result;
	}

	inline VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(VkDescriptorPool pool)
	{
		VkDescriptorSetAllocateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		result.descriptorPool = pool;

		return result;
	}


	inline VkDescriptorImageInfo DescriptorImageInfo(VkImageView imageView, VkImageLayout imageLayout)
	{
		VkDescriptorImageInfo result {};
		result.imageView = imageView;
		result.imageLayout = imageLayout;

		return result;
	}

	inline VkDescriptorBufferInfo DescriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE)
	{
		VkDescriptorBufferInfo result{};
		result.buffer = buffer;
		result.offset = offset;
		result.range = range;

		return result;
	}

	inline VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet set, uint32_t binding, VkDescriptorType type, VkDescriptorImageInfo* imageInfo, uint32_t descriptorCount = 1)
	{
		VkWriteDescriptorSet result {};
		result.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		result.dstSet = set;
		result.dstBinding = binding;
		result.descriptorType = type;
		result.descriptorCount = descriptorCount;
		result.pImageInfo = imageInfo;

		return result;
	}

	inline VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet set, uint32_t binding, VkDescriptorType type, VkDescriptorBufferInfo* bufferInfo, uint32_t descriptorCount = 1)
	{
		VkWriteDescriptorSet result{};
		result.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		result.dstSet = set;
		result.dstBinding = binding;
		result.descriptorType = type;
		result.descriptorCount = descriptorCount;
		result.pBufferInfo = bufferInfo;

		return result;
	}


	inline VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo()
	{
		VkPipelineShaderStageCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

		return result;
	}

	inline VkComputePipelineCreateInfo ComputePipelineCreateInfo()
	{
		VkComputePipelineCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

		return result;
	}

	inline VkShaderModuleCreateInfo ShaderModuleCreateInfo()
	{
		VkShaderModuleCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

		return result;
	}


	inline VkCommandPoolCreateInfo CommandPoolCreateInfo(VkCommandPoolCreateFlags flags)
	{
		VkCommandPoolCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		result.flags = flags;

		return result;
	}

	inline VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, VkCommandBufferLevel level, uint32_t cmdBufferCount = 1)
	{
		VkCommandBufferAllocateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		result.commandPool = pool;
		result.level = level;
		result.commandBufferCount = cmdBufferCount;

		return result;
	}

	inline VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags)
	{
		VkCommandBufferBeginInfo result {};
		result.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		result.flags = flags;

		return result;
	}

	inline VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags)
	{
		VkFenceCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		result.flags = flags;

		return result;
	}

	inline VkSemaphoreCreateInfo SemaphoreCreateInfo()
	{
		VkSemaphoreCreateInfo result {};
		result.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		return result;
	}

	inline VkImageMemoryBarrier ImageMemoryBarrier(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier result {};
		result.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		result.image = image;
		result.oldLayout = oldLayout;
		result.newLayout = newLayout;

		return result;
	}


	inline VkBufferCreateInfo BufferCreateInfo(VkBufferUsageFlags flags, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
	{
		VkBufferCreateInfo sphereInfo{};
		sphereInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		sphereInfo.usage = flags;
		sphereInfo.sharingMode = sharingMode;

		return sphereInfo;
	}
}