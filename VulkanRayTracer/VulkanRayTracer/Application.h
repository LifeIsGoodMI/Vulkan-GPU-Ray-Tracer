#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <fstream>
#include "VkDeleter.h"

#include "Scene\Planee.h"
#include "Scene\Sphere.h"
#include "Scene\Vector3.h"


/// <summary>
/// This class is the heart of the application.
/// It Initializes & communicates to the Vulkan instance.
/// </summary>


const int WIDTH = 1000;
const int HEIGHT = 1000;

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class Application
{
public:
	void Run();

	static void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, 
														size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
private:
#pragma region Fields
	GLFWwindow* window;
	double lastTime = glfwGetTime();
	int frames = 0;

	VKDeleter<VkInstance> instance{ vkDestroyInstance };
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VKDeleter<VkDevice> logicalDevice{ vkDestroyDevice };
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue computeQueue;

	VKDeleter<VkDebugReportCallbackEXT> callback{ instance, DestroyDebugReportCallbackEXT };
	VKDeleter<VkSurfaceKHR> surface{ instance, vkDestroySurfaceKHR };

	VKDeleter<VkSwapchainKHR> swapChain{ logicalDevice, vkDestroySwapchainKHR };
	std::vector<VkImage> swapChainImages;
	uint32_t curImageIndex;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	std::vector<VKDeleter<VkImageView>> swapChainImageViews;


	VKDeleter<VkPipeline> computePipeline{ logicalDevice, vkDestroyPipeline };
	VKDeleter<VkDescriptorPool> computeDescriptorPool{ logicalDevice, vkDestroyDescriptorPool };
	VKDeleter<VkDescriptorSetLayout> computeDescriptorSetLayout{ logicalDevice, vkDestroyDescriptorSetLayout };
	VKDeleter<VkPipelineLayout> computePipelineLayout{ logicalDevice, vkDestroyPipelineLayout };
	std::vector<VkDescriptorSet> computeDescriptorSets;

	VKDeleter<VkCommandPool> computeCommandPool{ logicalDevice, vkDestroyCommandPool };
	VkCommandBuffer computeCommandBuffer;
	VKDeleter<VkFence> computeFence{ logicalDevice, vkDestroyFence };

	VKDeleter<VkSemaphore> imageAvailableSemaphore{ logicalDevice, vkDestroySemaphore };
	VKDeleter<VkSemaphore> renderFinishedSemaphore{ logicalDevice, vkDestroySemaphore };


	VKDeleter<VkImage> computeImage{ logicalDevice, vkDestroyImage };
	VKDeleter<VkImageView > computeImageView{ logicalDevice, vkDestroyImageView };
	VKDeleter<VkDeviceMemory> computeImgDeviceMemory{ logicalDevice, vkFreeMemory };


	VKDeleter<VkBuffer> sphereBuffer{ logicalDevice, vkDestroyBuffer };
	VKDeleter<VkBuffer> planeBuffer{ logicalDevice, vkDestroyBuffer };

	VKDeleter<VkBuffer> uniformBuffer{ logicalDevice, vkDestroyBuffer };
	VKDeleter<VkDeviceMemory> sphereDeviceMemory{ logicalDevice, vkFreeMemory };
	VKDeleter<VkDeviceMemory> planeDeviceMemory{ logicalDevice, vkFreeMemory };

	VKDeleter<VkDeviceMemory> uniformDeviceMemory{ logicalDevice, vkFreeMemory };
#pragma endregion


#pragma region Methods

	void SetWindow();
	void InitVulkan();
	void Update();
	void Draw();

	void DebugFrameTime();


	void CreateVulkanInstance();
	void CreateSurface();

#pragma region Debug Callbacks
	VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
	void SetupDebugCallback();
#pragma endregion

#pragma region Layers & Extensions
	bool ValidationLayersSupported();

	std::vector<const char*> GetRequiredExtensions();
	void SupportedExtensions();
#pragma endregion

#pragma region Physical Device
	void GetPhysicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
#pragma endregion

	void CreateLogicalDevice();

#pragma region Swap Chains
	void CreateSwapChain();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
#pragma endregion

#pragma region Image Setup
	void CreateImageViews();

	void CreateComputeImage(VKDeleter<VkImage> &img, VKDeleter<VkImageView> &imgView, VKDeleter<VkDeviceMemory> &memory);
#pragma endregion

#pragma region Pipelines
	void CreateShaderModule(const std::vector<char>& code, VKDeleter<VkShaderModule>& shaderModule);
	void CreateComputePipeline();

	void CreateDescriptorPool();
	void PrepareComputeForPipelineCreation();
#pragma endregion

#pragma region Command Buffers
	void CreateComputeCommandPool();
	void CreateComputeCommandBuffer();
	void RecordComputeCommandBuffer();
	void CreateComputeFence();
#pragma endregion

#pragma region Synchronization
	void CreateSemaphores();

	void SetFirstImageBarriers(const VkCommandBuffer buffer, int curImage);

	void CopyImageMemory(const VkCommandBuffer buffer, int curImageIndex);

	void SetSecondImageBarriers(const VkCommandBuffer buffer, int curImage);
#pragma endregion

	std::vector<char> ReadBinaryFile(const std::string& filename);


#pragma region Buffers
	void GetMemoryProperties(int &memoryTypeIndex, VkMemoryPropertyFlags properties);

	void PrepareStorageBuffers();

	void CreateStorageBuffer(const void* data, VkDeviceSize &bufferSize, VKDeleter<VkBuffer> &buffer,
		VkBufferUsageFlags bufferUsageFlags, VKDeleter<VkDeviceMemory> &deviceMemory, uint32_t memTypeIndex);

	void UpdateUniformBuffer();

	void CopyMemory(const void* data, VKDeleter<VkDeviceMemory> &deviceMemory, VkDeviceSize &bufferSize);
#pragma endregion


	void InitGameObjects(std::vector<Planee> &planes, std::vector<Sphere> &spheres);


	struct App
	{
		float time;
	} app;
#pragma endregion
};

