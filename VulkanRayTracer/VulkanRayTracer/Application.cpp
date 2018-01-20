#include "Application.h"
#include "VulkanInitializers.h"
#include "QueueFamilyIndices.h"
#include "SwapChainSupportInfo.h"

#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <algorithm>
#include <chrono>


void Application::Run()
{
	SetWindow();

	InitVulkan();

	Update();
}

void Application::SetWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void Application::InitVulkan()
{
	CreateVulkanInstance();
	SetupDebugCallback();
	CreateSurface();

	GetPhysicalDevice();
	CreateLogicalDevice();

	CreateSwapChain();
	CreateImageViews();

	CreateComputeImage(computeImage, computeImageView, computeImgDeviceMemory);
	PrepareStorageBuffers();

	CreateDescriptorPool();
	PrepareComputeForPipelineCreation();
	CreateComputePipeline();


	CreateComputeCommandPool();
	CreateComputeCommandBuffer();
	RecordComputeCommandBuffer();
	CreateComputeFence();

	CreateSemaphores();
}

void Application::Update()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		DebugFrameTime();

		UpdateUniformBuffer();

		Draw();
	}

	// Wait for all queues to finish work.
	vkDeviceWaitIdle(logicalDevice);

	glfwDestroyWindow(window);
}

void Application::Draw()
{
	vkAcquireNextImageKHR(logicalDevice, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &curImageIndex);
	// Ideally, we would check if the swap chain is still valid etc. here.

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	
	VkSubmitInfo computeSubmitInfo = {};
	computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	computeSubmitInfo.commandBufferCount = 1;
	computeSubmitInfo.pCommandBuffers = &computeCommandBuffer;

	computeSubmitInfo.waitSemaphoreCount = 1;
	computeSubmitInfo.pWaitSemaphores = waitSemaphores;
	computeSubmitInfo.signalSemaphoreCount = 1;
	computeSubmitInfo.pSignalSemaphores = signalSemaphores;
	computeSubmitInfo.pWaitDstStageMask = waitStages;


	vkWaitForFences(logicalDevice, 1, &computeFence, VK_TRUE, UINT64_MAX);
	vkResetFences(logicalDevice, 1, &computeFence);

	RecordComputeCommandBuffer();

	auto resultSubmit = vkQueueSubmit(computeQueue, 1, &computeSubmitInfo, computeFence);
	if (resultSubmit != VK_SUCCESS)
		throw std::runtime_error("Failed to submit Compute Command Buffers to Compute Queue !");
	

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &curImageIndex;

	auto result = vkQueuePresentKHR(presentQueue, &presentInfo);

	// ToDo: Recreate Swap Chain
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		;
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present swap chain image !");
}


void Application::DebugFrameTime()
{
	double currentTime = glfwGetTime();
	frames++;

	if (currentTime - lastTime >= 1.0)
	{
		fprintf(stdout, "\rms/frame: %8.2f%", 1000.0 / double(frames));

		frames = 0;
		lastTime += 1.0;
	}
}



void Application::CreateVulkanInstance()
{
	if (enableValidationLayers && !ValidationLayersSupported())
		throw std::runtime_error("validation layers requested, but not available!");

	auto appInfo = Initializers::ApplicationInfo("Vulkan Ray Tracer", VK_MAKE_VERSION(1, 0, 0), VK_MAKE_VERSION(1, 0, 54));
	auto instanceInfo = Initializers::InstanceCreateInfo(appInfo);

	auto extensions = GetRequiredExtensions();
	instanceInfo.enabledExtensionCount = extensions.size();
	instanceInfo.ppEnabledExtensionNames = extensions.data();

	if (enableValidationLayers)
	{
		instanceInfo.enabledLayerCount = validationLayers.size();
		instanceInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
		instanceInfo.enabledLayerCount = 0;

	auto result = vkCreateInstance(&instanceInfo, nullptr, instance.Replace());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan Instance !");

	SupportedExtensions();
}


void Application::CreateSurface()
{
	auto result = glfwCreateWindowSurface(instance, window, nullptr, surface.Replace());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface!");
}


#pragma region DebugCallbacks
void Application::DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
		func(instance, callback, pAllocator);
}

VKAPI_ATTR VkBool32 VKAPI_CALL Application::DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location,
														  int32_t code, const char* layerPrefix, const char* msg, void* userData)
{
	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}

VkResult Application::CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
												   const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pCallback);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Application::SetupDebugCallback()
{
	if (!enableValidationLayers)
		return;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = Application::DebugCallback;

	auto result = CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, callback.Replace());
	if (result != VK_SUCCESS)
		throw std::runtime_error("failed to set up debug callback!");
}
#pragma endregion


#pragma region Layers & Extensions

bool Application::ValidationLayersSupported()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

std::vector<const char*> Application::GetRequiredExtensions()
{
	std::vector<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++)
		extensions.push_back(glfwExtensions[i]);

	if (enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	return extensions;
}

void Application::SupportedExtensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "available extensions:" << std::endl;

	for (const auto& extension : extensions)
		std::cout << "\t" << extension.extensionName << std::endl;
}
#pragma endregion


#pragma region Physical Device
void Application::GetPhysicalDevice()
{
	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
		throw std::runtime_error("Failed to find GPUs with Vulkan support !");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (IsDeviceSuitable(device))
		{
			physicalDevice = device;
			
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Failed to find a suitable GPU !");
}

bool Application::IsDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = FindQueueFamilies(device, surface);
	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) 
	{
		SwapChainSupportInfo swapChainSupport = QuerySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

bool Application::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
}
#pragma endregion


void Application::CreateLogicalDevice()
{
	auto indices = FindQueueFamilies(physicalDevice, surface);

	std::vector<VkDeviceQueueCreateInfo> queueInfos;
	std::set<int> uniqueQueueFamilies = { indices.presentFamily, indices.computeFamily };

	// Create a Queue Create Info for each of our Queue Families (i.e. present & compute)
	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies)
	{
		auto queueCreateInfo = Initializers::DeviceQueueCreateInfo(queueFamily, queuePriority);

		queueInfos.push_back(queueCreateInfo);
	}

	// Either fill this with explicit features, or query all supported features by the physical device.
	// However, secondly might have some heavy performance impact since it could allocate memory for unused features.
	VkPhysicalDeviceFeatures deviceFeatures {};

	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

	auto deviceInfo = Initializers::DeviceCreateInfo();
	deviceInfo.pQueueCreateInfos = queueInfos.data();
	deviceInfo.queueCreateInfoCount = queueInfos.size();
	deviceInfo.pEnabledFeatures = &deviceFeatures;
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceInfo.enabledExtensionCount = deviceExtensions.size();

	if (enableValidationLayers)
	{
		deviceInfo.enabledLayerCount = validationLayers.size();
		deviceInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
		deviceInfo.enabledLayerCount = 0;

	auto result = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, logicalDevice.Replace());
	if (result != VK_SUCCESS)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		std::string s = "Failed to creae a logical device for the physical device: " + std::string(deviceProperties.deviceName);

		throw std::runtime_error(s);
	}

	vkGetDeviceQueue(logicalDevice, indices.presentFamily, 0, &presentQueue);
	vkGetDeviceQueue(logicalDevice, indices.computeFamily, 0, &computeQueue);
}


#pragma region Swap Chain
void Application::CreateSwapChain()
{
	auto swapChainSupport = QuerySwapChainSupport(physicalDevice, surface);
	auto surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	auto presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	auto extent = ChooseSwapExtent(swapChainSupport.capabilities);

	if (!(swapChainSupport.capabilities.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT))
		throw std::runtime_error("Swap chain doesn't support VK_IMAGE_USAGE_STORAGE BIT !");


	// Decide how many images we can work with at the same time (1, double buffering or triple buffering)
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	auto swapchainInfo = Initializers::SwapchainCreateInfoKHR(surface);
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = surfaceFormat.format;
	swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainInfo.imageExtent = extent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// The Transform capabilities of the swap chain define which transformations are supported
	// I.e. portrait, landscape presentation etc.
	swapchainInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	// Ignore image's alpha channel (if there's any)
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// Set the presentation mode. Mailbox is preferred since it works just like immediate but instead
	// of blocking the application when the queue is full, it simply replaces images with new ones.
	swapchainInfo.presentMode = presentMode;
	// Only render images when they're visible.
	swapchainInfo.clipped = VK_TRUE;

	// Use the current swap chain for recycling if there is any.
	VkSwapchainKHR oldSwapChain = swapChain;
	swapchainInfo.oldSwapchain = oldSwapChain;

	VkSwapchainKHR newSwapChain;
	auto result = vkCreateSwapchainKHR(logicalDevice, &swapchainInfo, nullptr, &newSwapChain);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create swap chain !");

	swapChain = newSwapChain;

	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };


	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}

	return availableFormats[0];
}

// Get preferably triple buffering presentation mode if available (triple buffering brings best performance)
// Triple buffering means that we can write to another 2 Images while 1 Image is being already presented.
VkPresentModeKHR Application::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Application::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else
	{
		VkExtent2D actualExtent = { WIDTH, HEIGHT };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}
#pragma endregion


#pragma region Image Setup
void Application::CreateImageViews()
{
	swapChainImageViews.resize(swapChainImages.size(), VKDeleter<VkImageView>{logicalDevice, vkDestroyImageView});
	for (uint32_t i = 0; i < swapChainImages.size(); i++)
	{
		VkComponentMapping components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

		auto createInfo = Initializers::ImageViewCreateInfo(swapChainImages[i], VK_IMAGE_VIEW_TYPE_2D);
		createInfo.format = swapChainImageFormat;
		createInfo.components = components;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		auto imageViewCreated = vkCreateImageView(logicalDevice, &createInfo, nullptr, swapChainImageViews[i].Replace());
		if (imageViewCreated != VK_SUCCESS)
			throw std::runtime_error("failed to create image views!");
	}
}

void Application::CreateComputeImage(VKDeleter<VkImage> &img, VKDeleter<VkImageView> &imgView, VKDeleter<VkDeviceMemory> &memory)
{
	auto swapChainSupport = QuerySwapChainSupport(physicalDevice, surface);
	auto surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);

	auto info = Initializers::ImageCreateInfo(VK_IMAGE_TYPE_2D);
	info.format = surfaceFormat.format;
	info.extent = { WIDTH, HEIGHT, 1 };
	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	info.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	
	auto result = vkCreateImage(logicalDevice, &info, nullptr, img.Replace());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create compute image !");

	int memTypeIndex = 0;
	GetMemoryProperties(memTypeIndex, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(logicalDevice, img, &memReqs);

	auto memInfo = Initializers::MemoryAllocateInfo(memReqs.size, memTypeIndex);

	result = vkAllocateMemory(logicalDevice, &memInfo, nullptr, memory.Replace());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate memory for compute image !");
	result = vkBindImageMemory(logicalDevice, img, memory, 0);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to bind memory to compute image !");

	auto viewInfo = Initializers::ImageViewCreateInfo(img, VK_IMAGE_VIEW_TYPE_2D);
	viewInfo.format = surfaceFormat.format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	result = vkCreateImageView(logicalDevice, &viewInfo, nullptr, imgView.Replace());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create compute image view !");
}
#pragma endregion


#pragma region Pipelines
void Application::CreateDescriptorPool()
{
	auto storageSize = Initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3);
	auto bufferSize = Initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2);
	auto uniformSize = Initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);

	std::vector<VkDescriptorPoolSize> poolSizes = { storageSize , bufferSize, uniformSize };

	auto poolInfo = Initializers::DescriptorPoolCreateInfo();
	poolInfo.maxSets = 1;
	poolInfo.poolSizeCount = poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();

	auto result = vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, computeDescriptorPool.Replace());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create Compute Descriptor Pool !");
}

void Application::PrepareComputeForPipelineCreation()
{
	auto computeBinding = Initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 0);
	auto sphereBinding = Initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1);
	auto planeBinding = Initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2);
	auto uniformBinding = Initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 3);

	std::vector<VkDescriptorSetLayoutBinding> bindings{ computeBinding, sphereBinding, planeBinding, uniformBinding };

	auto layoutInfo = Initializers::DescriptorSetLayoutCreateInfo();
	layoutInfo.bindingCount = bindings.size();
	layoutInfo.pBindings = bindings.data();

	auto result = vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, computeDescriptorSetLayout.Replace());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create Compute DescriptorSet Layout !");


	auto pipelineLayoutInfo = Initializers::PipelineLayoutCreateInfo();
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;

	result = vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, computePipelineLayout.Replace());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create Compute Pipeline Layout !");


	auto allocInfo = Initializers::DescriptorSetAllocateInfo(computeDescriptorPool);
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &computeDescriptorSetLayout;;

	computeDescriptorSets.resize(1);
	result = vkAllocateDescriptorSets(logicalDevice, &allocInfo, computeDescriptorSets.data());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate Compute Descriptor Sets from Compute Descriptor Pool !");


	// Bind resources to the descriptor sets
	auto computeInfo = Initializers::DescriptorImageInfo(computeImageView, VK_IMAGE_LAYOUT_GENERAL);

	auto sphereInfo = Initializers::DescriptorBufferInfo(sphereBuffer);
	auto planeInfo = Initializers::DescriptorBufferInfo(planeBuffer);
	auto uniformInfo = Initializers::DescriptorBufferInfo(uniformBuffer);


	auto computeWrite = Initializers::WriteDescriptorSet(computeDescriptorSets[0], 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &computeInfo);

	auto sphereWrite = Initializers::WriteDescriptorSet(computeDescriptorSets[0], 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &sphereInfo);
	auto planeWrite = Initializers::WriteDescriptorSet(computeDescriptorSets[0], 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &planeInfo);
	auto uniformWrite = Initializers::WriteDescriptorSet(computeDescriptorSets[0], 3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &uniformInfo);

	std::vector<VkWriteDescriptorSet> writeSets = { computeWrite, sphereWrite, planeWrite, uniformWrite };
	vkUpdateDescriptorSets(logicalDevice, writeSets.size(), writeSets.data(), 0, VK_NULL_HANDLE);
}

void Application::CreateComputePipeline()
{
	auto computeShaderCode = ReadBinaryFile("shaders/comp.spv");
	VKDeleter<VkShaderModule> computeShaderModule{ logicalDevice, vkDestroyShaderModule };
	CreateShaderModule(computeShaderCode, computeShaderModule);


	auto computeStageInfo = Initializers::PipelineShaderStageCreateInfo();
	computeStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	computeStageInfo.module = computeShaderModule;
	// Entry point of the shader.
	computeStageInfo.pName = "main";

	auto pipelineInfo = Initializers::ComputePipelineCreateInfo();
	pipelineInfo.stage = computeStageInfo;
	pipelineInfo.layout = computePipelineLayout;

	// ToDo: Create Pipeline Cache to accelerate pipeline creation.
	// See "Accelerating Pipeline Creation" in the Vulkan Programming Guide book.
	auto result = vkCreateComputePipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, computePipeline.Replace());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create Compute Pipelines !");
}


void Application::CreateShaderModule(const std::vector<char>& code, VKDeleter<VkShaderModule>& shaderModule)
{
	auto createInfo = Initializers::ShaderModuleCreateInfo();
	createInfo.codeSize = code.size();
	createInfo.pCode = (uint32_t*)code.data();

	auto shaderModCreated = vkCreateShaderModule(logicalDevice, &createInfo, nullptr, shaderModule.Replace());
	if (shaderModCreated != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader module!");
}
#pragma endregion


#pragma region Command Buffers
void Application::CreateComputeCommandPool()
{
	auto queueFamilyIndices = FindQueueFamilies(physicalDevice, surface);

	auto poolInfo = Initializers::CommandPoolCreateInfo(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily;

	auto result = vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, computeCommandPool.Replace());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create Compute Command Pool !");
}

void Application::CreateComputeCommandBuffer()
{
	curImageIndex = 0;

	auto allocateInfo = Initializers::CommandBufferAllocateInfo(computeCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	auto alloResult = vkAllocateCommandBuffers(logicalDevice, &allocateInfo, &computeCommandBuffer);
	if (alloResult != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate Compute Command Buffers !");
}

void Application::RecordComputeCommandBuffer()
{
	auto beginInfo = Initializers::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

	auto result = vkBeginCommandBuffer(computeCommandBuffer, &beginInfo);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Compute Command Buffer Recording couldn't be started !");

	vkCmdBindPipeline(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

	// Bind current descriptor set for each image in the swap chain.
	vkCmdBindDescriptorSets(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout,
		0, 1, &computeDescriptorSets[0], 0, 0);

	vkCmdDispatch(computeCommandBuffer, swapChainExtent.width, swapChainExtent.height, 1);

	// set a image memory barrier for each image seperatly.
	SetFirstImageBarriers(computeCommandBuffer, curImageIndex);

	CopyImageMemory(computeCommandBuffer, curImageIndex);

	SetSecondImageBarriers(computeCommandBuffer, curImageIndex);

	result = vkEndCommandBuffer(computeCommandBuffer);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Compute Command Buffer Recording couldn't be ended !");
}

void Application::CreateComputeFence()
{
	auto info = Initializers::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

	vkCreateFence(logicalDevice, &info, nullptr, computeFence.Replace());
}
#pragma endregion


#pragma region Synchronization
void Application::CreateSemaphores()
{
	auto semaphoreInfo = Initializers::SemaphoreCreateInfo();

	auto availResult = vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, imageAvailableSemaphore.Replace());
	auto renderFshResult = vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, renderFinishedSemaphore.Replace());
	if (availResult != VK_SUCCESS || renderFshResult != VK_SUCCESS)
		throw std::runtime_error("Failed to create semaphores !");
}

void Application::SetFirstImageBarriers(const VkCommandBuffer buffer, int curImageIndex)
{
	auto compWrite = Initializers::ImageMemoryBarrier(computeImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	compWrite.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	compWrite.srcAccessMask = 0;
	compWrite.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

	auto compTransfer = Initializers::ImageMemoryBarrier(computeImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	compTransfer.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	compTransfer.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	compTransfer.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	auto swapTransfer = Initializers::ImageMemoryBarrier(swapChainImages[curImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	swapTransfer.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	swapTransfer.srcAccessMask = 0;
	swapTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	std::vector<VkImageMemoryBarrier> barriers{ compWrite, compTransfer, swapTransfer };

	vkCmdPipelineBarrier(buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		0, 0, nullptr, 0, nullptr, barriers.size(), barriers.data());
}

void Application::CopyImageMemory(const VkCommandBuffer buffer, int curImageIndex)
{
	VkImageSubresourceLayers source;
	source.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	source.mipLevel = 0;
	source.baseArrayLayer = 0;
	source.layerCount = 1;

	VkImageSubresourceLayers dest;
	dest.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	dest.mipLevel = 0;
	dest.baseArrayLayer = 0;
	dest.layerCount = 1;

	VkImageCopy copy;
	copy.srcSubresource = source;
	copy.dstSubresource = dest;
	copy.extent = { WIDTH, HEIGHT, 1 };
	copy.srcOffset = { 0, 0, 0 };
	copy.dstOffset = { 0, 0, 0 };


	vkCmdCopyImage(buffer, computeImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapChainImages[curImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
}

void Application::SetSecondImageBarriers(const VkCommandBuffer buffer, int curImageIndex)
{
	auto swapPres = Initializers::ImageMemoryBarrier(swapChainImages[curImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	swapPres.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	swapPres.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	swapPres.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

	vkCmdPipelineBarrier(buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0, 0, nullptr, 0, nullptr, 1, &swapPres);
}
#pragma endregion


#pragma region Buffers
void Application::GetMemoryProperties(int &memoryTypeIndex, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

	for (int i = 0; i < memProps.memoryTypeCount; i++)
	{
		if ((memProps.memoryTypes[i].propertyFlags & properties) == properties)
		{
			memoryTypeIndex = i;
			break;
		}
	}
}


void Application::PrepareStorageBuffers()
{
	std::vector<Planee> planes;
	std::vector<Sphere> spheres;
	InitGameObjects(planes, spheres);

	int memTypeIndex = 0;
	GetMemoryProperties(memTypeIndex, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	VkDeviceSize spBufferSize = spheres.size() * sizeof(Sphere);
	VkDeviceSize plBufferSize = planes.size() * sizeof(Planee);
	VkDeviceSize uniformBufferSize = sizeof(app);

	CreateStorageBuffer(spheres.data(), spBufferSize, sphereBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sphereDeviceMemory, memTypeIndex);
	CreateStorageBuffer(planes.data(), plBufferSize, planeBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, planeDeviceMemory, memTypeIndex);

	CreateStorageBuffer(&app, uniformBufferSize, uniformBuffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, uniformDeviceMemory, memTypeIndex);
}


void Application::CreateStorageBuffer(const void* data, VkDeviceSize &bufferSize, VKDeleter<VkBuffer> &buffer,
	VkBufferUsageFlags bufferUsageFlags, VKDeleter<VkDeviceMemory> &deviceMemory, uint32_t memTypeIndex)
{
	VkResult result;

	auto bufferInfo = Initializers::BufferCreateInfo(bufferUsageFlags);
	bufferInfo.size = bufferSize;

	result = vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, buffer.Replace());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create storage buffer !");


	VkMemoryRequirements memoryReqs;
	vkGetBufferMemoryRequirements(logicalDevice, buffer, &memoryReqs);

	auto memoryInfo = Initializers::MemoryAllocateInfo(memoryReqs.size, memTypeIndex);

	result = vkAllocateMemory(logicalDevice, &memoryInfo, nullptr, deviceMemory.Replace());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate device memory for storage buffer !");


	CopyMemory(data, deviceMemory, bufferSize);

	vkBindBufferMemory(logicalDevice, buffer, deviceMemory, 0);
}


void Application::UpdateUniformBuffer()
{
	// Update values here
	app.time = glfwGetTime();
	// 

	VkDeviceSize size = sizeof(app);
	CopyMemory(&app, uniformDeviceMemory, size);
}

void Application::CopyMemory(const void* data, VKDeleter<VkDeviceMemory> &deviceMemory, VkDeviceSize &bufferSize)
{
	// map memory to the range of the data
	void* mapped = nullptr;
	auto result = vkMapMemory(logicalDevice, deviceMemory, 0, VK_WHOLE_SIZE, 0, &mapped);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to map memory for storage buffer !");

	// copy the data to the mapped region
	std::memcpy(mapped, data, bufferSize);

	// memory is copied, mapped region is no longer needed.
	if (mapped)
	{
		vkUnmapMemory(logicalDevice, deviceMemory);
		mapped = nullptr;
	}
}
#pragma endregion




std::vector<char> Application::ReadBinaryFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		throw std::runtime_error("Failed to open binary file !");

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

void Application::InitGameObjects(std::vector<Planee> &planes, std::vector<Sphere> &spheres)
{
	auto AddPlanee = [&planes](Planee* go, Vector3 color, int type)
	{
		go->mat = Material(color, type);
		planes.push_back(*go);
	};

	auto AddSphere = [&spheres](Sphere* go, Vector3 color, int type)
	{
		go->mat = Material(color, type);
		spheres.push_back(*go);
	};

	auto sphere = new Sphere(Vector3(-0.55, -1.55, -4.0), 1.0);
	auto sphere2 = new Sphere(Vector3(1.3, 1.2, -4.2), 0.8);

	auto bottom = new Planee(Vector3(0, 1, 0), 2.5);
	auto back = new Planee(Vector3(0, 0, 1), 5.5);
	auto left = new Planee(Vector3(1, 0, 0), 2.75);
	auto right = new Planee(Vector3(-1, 0, 0), 2.75);
	auto ceiling = new Planee(Vector3(0, -1, 0), 3.0);
	auto front = new Planee(Vector3(0, 0, -1), 0.5);


	AddSphere(sphere, Vector3(0.3, 0.9, 0.76), 2);
	AddSphere(sphere2, Vector3(0.062, 0.917, 0.078), 1);

	AddPlanee(bottom, Vector3(0.8, 0.8, 0.8), 1);
	AddPlanee(back, Vector3(0.8, 0.8, 0.8), 1);
	AddPlanee(left, Vector3(1, 0.250, 0.019), 1);
	AddPlanee(right, Vector3(0.007, 0.580, 0.8), 1);
	AddPlanee(ceiling, Vector3(0.8, 0.8, 0.8), 1);
	AddPlanee(front, Vector3(0.8, 0.8, 0.8), 1);
}