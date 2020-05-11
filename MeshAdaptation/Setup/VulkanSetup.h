#pragma once
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include<vulkan/vulkan.h>	
#include<stdexcept>
#include<iostream>
#include<vector>
#include<assert.h>
#include<set>
#include<algorithm>
#include<array>
#include<fstream>
#include<glm.hpp>
#include<gtc/matrix_transform.hpp>

#include"Buffers/Buffer.h"
#include"MeshLoader/Mesh.h"
#include"MeshLoader/Model.h"
#include"Lights/DirectionalLight.h"
#include"Deformation/MeshAdapter.h"

const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
#ifdef _DEBUG
const bool m_ValidationLayersEnabled = true;
#else 
const bool m_ValidationLayersEnabled = false;
#endif	

struct SwapChainSupportDetail
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
	uint32_t graphicsQueueFamily = 0;
	uint32_t presentQueueFamily = 0;
	bool graphicsFamilyFound = true;
	bool presentFamilyFound = true;
	bool isComplete() {
		return graphicsFamilyFound && presentFamilyFound;
	}
};

class VulkanSetup
{
private: 
	MeshAdapter* m_MeshAdapter; 
	DirectionalLight* m_DirectionalLight;
	std::vector<Mesh*> m_Meshes;						//not used anymore 
	std::vector<Model*> m_Models;
	int currentFrame = 0;

	struct ViewProjection
	{
		glm::mat4 view;
		glm::mat4 projection;
	}m_VP;

public: 
	static VkPhysicalDevice m_PhysicalDevice; 
	static VkDevice m_LogicalDevice;
	static VkQueue m_GraphicsQueue;
	static VkQueue m_PresentQueue;
	static VkCommandPool m_GraphicsCommandPool;	
	static VkDescriptorSetLayout m_SamplerSetLayout;
	static VkDescriptorPool m_SamplerDescriptorPool;
	static VkSampler m_TextureSampler;
private: 
	GLFWwindow* m_MainWindow;

	//Vulkan Objects
	VkInstance m_Instance; 
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	VkSurfaceKHR m_Surface; 
	VkSwapchainKHR m_Swapchain;

	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkDescriptorSetLayout m_DirectionalLightDescriptorSetLayout;
	
	VkRenderPass m_RenderPass; 
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeline;
	
	std::vector<VkCommandBuffer> m_CommandBuffers;
	std::vector<VkSemaphore> m_ImageAvailableSemaphore;
	std::vector<VkSemaphore> m_FinishedRenderingSemaphore;
	std::vector<VkFence> m_DrawFences;

	//------------SUPPORT FOR VK Objects----------------------

	//Dynamic Uniform Buffer Support 
	VkDeviceSize m_MinUniformBufferOffsetAlignment;
	VkDeviceSize modelUniformAlignment;
	UBOModel* m_ModelTransferSpace; 
	
	//Descriptor Set Pool 
	VkDescriptorPool m_DescriptorPool;
	VkDescriptorPool m_DirectionalLightPool;

	//Uniform Buffers 
	std::vector<Buffer*> m_vpUniformBuffers;									//one for each img in swapchain.
	std::vector<Buffer*> m_ModelDynamicUniformBuffers;							//one for each img in swapchain.
	std::vector<Buffer*> m_DirectionalLightUniformBuffers;						//one for each img in swapChain) 

	
	//Depth Buffer
	VkImage m_DepthBufferImage;
	VkImageView m_DepthBufferImageView;
	VkDeviceMemory m_DepthBufferMemory;

	//Descriptor Sets 
	std::vector<VkDescriptorSet> m_DescriptorSets;							//one for each img in swapchain.
	std::vector<VkDescriptorSet> m_DirectionalLightDescriptorSets;

	//SwapChain Frame Buffers 
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;

	//SwapChains 
	struct SwapchainImages
	{
		VkImage m_Image;
		VkImageView m_ImageView;
	};

	std::vector<SwapchainImages> m_SwapchainImages;
	VkFormat m_SwapchainImagesFormat;
	VkExtent2D m_SwapchainExtent;

public: 
	VulkanSetup(GLFWwindow* window); 
	~VulkanSetup();

	void drawFrame();
	void updateModel(glm::mat4 newModel);
	static VkCommandBuffer beginCommandBuffer(VkCommandPool commandPool);
	static void endAndSubmitCommandBuffer(VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer);


private: 
	void init(); 
	void createInstance(); 
	void setUpDebugMessenger(); 
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createDepthBufferImage();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createTextureSamplers();
	void allocateDynamicBufferTransferSpace();
	void createUniformBuffers();
	void createDescriptorSetPool();
	void allocateDescriptorSets();
	void createSemaphore();
	void cleanUp(); 

	//Semi Important 
	VkRenderPass createRenderPass();
	void recordCommandBuffers(int imageIndex);
	void updateUniformBuffers(uint32_t imageIndex);


	//support Functions 
	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport(); 
	void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo); 
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetail querySwapchainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> swapChainPresent);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkFormat chooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);
	//Image 
	VkImage createImage(uint32_t height, uint32_t width, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags memPropertyFlags, VkDeviceMemory* deviceMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags imageAspectFlags);
	//Shaders Modules 
	VkShaderModule createShaderModule(const std::vector<char>& code);
	std::vector<char> readFile(const std::string& filename);
	
	//Debug Callback Function
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);


	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
											const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	
};

