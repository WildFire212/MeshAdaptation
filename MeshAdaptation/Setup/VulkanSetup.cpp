#define STB_IMAGE_IMPLEMENTATION
#include "Setup/VulkanSetup.h"
#include"TextureLoaders/Texture.h"
const int MAX_FRAME_DRAWS = 2;
const int MAX_OBJECTS = 6;
VkPhysicalDevice VulkanSetup::m_PhysicalDevice = VK_NULL_HANDLE;
VkDevice VulkanSetup::m_LogicalDevice;
VkQueue VulkanSetup::m_GraphicsQueue;
VkQueue VulkanSetup::m_PresentQueue;
VkCommandPool VulkanSetup::m_GraphicsCommandPool;
VkDescriptorSetLayout VulkanSetup::m_SamplerSetLayout;
VkDescriptorPool VulkanSetup::m_SamplerDescriptorPool;
VkSampler VulkanSetup::m_TextureSampler;


VulkanSetup::VulkanSetup(GLFWwindow* window) : 
	m_MainWindow(window)
{
	init(); 
}

void VulkanSetup::init()
{
	try
	{	
		createInstance();
		setUpDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createDepthBufferImage();
		createFramebuffers();
		createCommandPool();
		createCommandBuffers();
		createTextureSamplers();
		allocateDynamicBufferTransferSpace();
		createUniformBuffers();
		createDescriptorSetPool();
		allocateDescriptorSets();
		createSemaphore();


		m_VP.projection = glm::perspective(glm::radians(45.0f), (float)m_SwapchainExtent.width / (float)m_SwapchainExtent.height, 0.1f, 100.0f);
		m_VP.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		m_VP.projection[1][1] *= -1;

		std::vector<MeshData> meshVertices1 = {
			{ { -0.4, 0.4, 0.0 },{ 0.0f, 0.0f,0.0f },{ 1.0f, 1.0f } ,{1.0,1.0,1.0}	 },	// 0
		{ { -0.4, -0.4, 0.0 },{ 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } , {1.0,1.0,1.0}},	    // 1
		{ { 0.4, -0.4, 0.0 },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f }, {1.0,1.0,1.0}},    // 2
		{ { 0.4, 0.4, 0.0 },{ 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f }, {1.0,1.0,1.0}}   // 3
		};

		std::vector<uint32_t> indices = {
				0, 1, 2,
				2, 3, 0
		};
		Texture* giraffe = new Texture("giraffe.jpg");
		giraffe->createTextureDescriptor(m_SamplerDescriptorPool, m_SamplerSetLayout, m_TextureSampler);
		Mesh* mesh1 = new Mesh(&meshVertices1, &indices, giraffe);
		m_Meshes.push_back(mesh1);

		const int SourceModelControlPoints[36] = {
				1699 ,
				1584 ,
				929	 ,
				283	 ,
				1033 ,
				509	 ,
				1747 ,
				1749 ,
				1808 ,
				1826 ,
				1099 ,
				1093 ,
				1052 ,
				1119 ,
				825	 ,
				55	 ,
				1624 ,
				1456 ,
				772	 ,
				270	 ,
				742	 ,
				2352 ,
				265	 ,
				2279 ,
				1401 ,
				2108 ,
				2182 ,
				1352 ,
				2136 ,
				682	 ,
				644	 ,
				2030 ,
				1993 ,
				1972 ,
				593	 ,
				29


		};

		const int KinectModelControlPoints[36]{
				68853	,
				95226	,
				63054	,
				211158	,
				324466	,
				203469	,

				43249	 ,
				37904	 ,
				274413	 ,
				9516	 ,
				33331	 ,
				241095	 ,
				167775	 ,
				296163	 ,
				15612	 ,
				4497	 ,
				33222	 ,
				174192	 ,
				86805	 ,
				29431	 ,
				33805	 ,
				234		 ,
				115307	 ,
				115311	 ,
				314		 ,
				30555	 ,
				116109	 ,
				235893	 ,
				116297	 ,
				16266	,
				10849	,
				31077	,
				2709	,
				58515	,
				2210	,
				11605
		};


		Model* sourceMeshDeformed = new Model("SourceFaceModel.obj",true);
		Model* sourceMesh= new Model("SourceFaceModel.obj",true);
		Model* dstMesh = new Model("KinectHeadScan.obj",false);
		
		
		sourceMeshDeformed->start();
		sourceMesh->start();
		dstMesh->start();

		m_Models.push_back(sourceMeshDeformed);
		//m_Models.push_back(dstMesh);

		m_MeshAdapter = new MeshAdapter(sourceMesh, sourceMeshDeformed,dstMesh, SourceModelControlPoints, KinectModelControlPoints);

		m_DirectionalLight = new DirectionalLight(glm::vec3(1.0f, 1.0f, 1.0f), 0.6f, 0.7f, glm::vec3(1.0f, 1.0f, 1.0f));

	}
	catch (std::runtime_error& err)
	{
		std::cout << "ERROR : " << err.what();
	}
}



void VulkanSetup::cleanUp()
{
	vkDeviceWaitIdle(m_LogicalDevice); 

	delete m_MeshAdapter; 
	delete m_DirectionalLight;


	for (size_t i = 0; i < m_Meshes.size(); i++)
	{
		delete m_Meshes[i];
	}
	for (size_t i = 0; i < m_Models.size(); i++)
	{
		delete m_Models[i];
	}
	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		vkDestroySemaphore(m_LogicalDevice, m_ImageAvailableSemaphore[i], nullptr);
		vkDestroySemaphore(m_LogicalDevice, m_FinishedRenderingSemaphore[i], nullptr);
		vkDestroyFence(m_LogicalDevice, m_DrawFences[i], nullptr);
	}

	vkDestroyDescriptorPool(m_LogicalDevice, m_DescriptorPool, nullptr); 
	vkDestroyDescriptorPool(m_LogicalDevice, m_SamplerDescriptorPool, nullptr); 
	vkDestroyDescriptorPool(m_LogicalDevice, m_DirectionalLightPool, nullptr); 
	//Uniform Buffers * 
	//Texture Samplers
	vkDestroySampler(m_LogicalDevice, m_TextureSampler, nullptr); 
	vkDestroyCommandPool(m_LogicalDevice, m_GraphicsCommandPool, nullptr);
	//Frame buffers *
	//Destroy Depth Buffer image 
	vkDestroyImageView(m_LogicalDevice, m_DepthBufferImageView, nullptr); 
	vkDestroyImage(m_LogicalDevice, m_DepthBufferImage, nullptr); 
	//Destroy Pipeline Specifics
	vkDestroyRenderPass(m_LogicalDevice, m_RenderPass, nullptr);
	vkDestroyPipeline(m_LogicalDevice, m_GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_LogicalDevice, m_PipelineLayout, nullptr);
	//Descriptor Set Layouts
	vkDestroyDescriptorSetLayout(m_LogicalDevice, m_DirectionalLightDescriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_LogicalDevice, m_SamplerSetLayout, nullptr); 
	vkDestroyDescriptorSetLayout(m_LogicalDevice, m_DescriptorSetLayout, nullptr);
	//IMage view * 
	vkDestroySwapchainKHR(m_LogicalDevice, m_Swapchain, nullptr);
	vkDestroyDevice(m_LogicalDevice, nullptr); 
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	if (m_ValidationLayersEnabled) {
		destroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
	}
	vkDestroyInstance(m_Instance, nullptr); 
}

void VulkanSetup::createInstance()
{
	if (m_ValidationLayersEnabled && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Validation layers requested but not available!");
	}

	VkApplicationInfo appInfo = {};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;			
	appInfo.pApplicationName = "Mesh Adaptation";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	auto instanceExtensions = getRequiredExtensions();						//Extensions Required
	
	//Print all the extensions
	std::cout << "Total Extensions Required : " << std::endl;
	for (const char* extension : instanceExtensions)
	{
		std::cout << extension << std::endl;
	}

	
	auto checkInstanceExtensionSupport = [](std::vector<const char*> instanceExtension) -> bool
	{
		uint32_t supportedInstanceExtensionsCount;
		vkEnumerateInstanceExtensionProperties(nullptr, &supportedInstanceExtensionsCount, nullptr);
		assert(supportedInstanceExtensionsCount != 0);

		std::vector<VkExtensionProperties> supportedInstanceExtensions(supportedInstanceExtensionsCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &supportedInstanceExtensionsCount, supportedInstanceExtensions.data());

		bool allExtensionsSupported = false;
		for (auto requiredExtension : instanceExtension)
		{
			for (auto supportedExtension : supportedInstanceExtensions)
			{
				if (strcmp(supportedExtension.extensionName, requiredExtension) == 0)
				{
					allExtensionsSupported = true;
					break;
				}
				else {
					allExtensionsSupported = false;
				}
			}
			if (!allExtensionsSupported)
			{
				return false;
			}

		}
		return allExtensionsSupported;
	};


	VkInstanceCreateInfo createInfo = {};					
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;					

	//Check if retrieved Extensions Supported
	if (checkInstanceExtensionSupport(instanceExtensions))
	{
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	}
	else
	{
		createInfo.ppEnabledExtensionNames = nullptr;
		createInfo.enabledExtensionCount = 0;
	}

	//check for validation layer support
	if (m_ValidationLayersEnabled && checkValidationLayerSupport())
	{
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		populateDebugMessenger(debugMessengerCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)(&debugMessengerCreateInfo);
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}
	
	if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan Instance!");
	}


}

void VulkanSetup::setUpDebugMessenger()
{
	if (!m_ValidationLayersEnabled)
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfoStruct;
	populateDebugMessenger(createInfoStruct);

	if (createDebugUtilsMessengerEXT(m_Instance, &createInfoStruct, nullptr, &m_DebugMessenger) != VK_SUCCESS)
	{	
		throw std::runtime_error("Failed to setup Debug Messenger!");
	}

}

void VulkanSetup::createSurface()
{
	glfwCreateWindowSurface(m_Instance, m_MainWindow, nullptr, &m_Surface);
}


void VulkanSetup::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("Unable to find GPUs with vulkan support!");
	}
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);

	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, physicalDevices.data());


	for (const auto& device : physicalDevices)
	{
		if (isDeviceSuitable(device))
		{
			m_PhysicalDevice = device;
			std::cout << "Suitable GPU found" << std::endl;
			break;
		}
	}

	if (m_PhysicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Unable to find suitable GPU!");
	}

	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);
	m_MinUniformBufferOffsetAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
}


void VulkanSetup::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);

	//specify the queues
	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsQueueFamily,indices.presentQueueFamily };


	float queuePriority = 1.0f;
	//for each queue family specify queue info and no. Of queues
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		QueueCreateInfos.push_back(queueCreateInfo);
	}

	//no device features at the moment
	VkPhysicalDeviceFeatures deviceFeatures = {};


	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<int32_t>(QueueCreateInfos.size());
	createInfo.pQueueCreateInfos = QueueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = (deviceExtensions.data());
	if (m_ValidationLayersEnabled)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_LogicalDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create Logical Device");
	}

	vkGetDeviceQueue(m_LogicalDevice, indices.graphicsQueueFamily, 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_LogicalDevice, indices.presentQueueFamily, 0, &m_PresentQueue);
}


void VulkanSetup::createSwapChain()
{
	const SwapChainSupportDetail& m_SwapchainSupport = querySwapchainSupport(m_PhysicalDevice);
	//set the surface format 
	VkSurfaceFormatKHR surfaceFormat = chooseSwapchainFormat(m_SwapchainSupport.formats);

	//Choose presentation Modes
	VkPresentModeKHR presentMode = chooseSwapchainPresentMode(m_SwapchainSupport.presentModes);

	//extents 
	VkExtent2D extent = chooseSwapExtent(m_SwapchainSupport.capabilities);

	uint32_t imageCount = m_SwapchainSupport.capabilities.minImageCount + 1;

	if (m_SwapchainSupport.capabilities.maxImageCount > 0 && imageCount > m_SwapchainSupport.capabilities.maxImageCount)
	{
		imageCount = m_SwapchainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_Surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);

	if (indices.graphicsQueueFamily != indices.presentQueueFamily)
	{
		uint32_t queueFamilyIndices[] = { indices.graphicsQueueFamily , indices.presentQueueFamily };
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	createInfo.preTransform = m_SwapchainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_LogicalDevice, &createInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create info Swapchain!");
	}

	m_SwapchainImagesFormat = surfaceFormat.format;
	m_SwapchainExtent = extent;


}


VulkanSetup::~VulkanSetup()
{
	cleanUp(); 
}



bool VulkanSetup::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);

	//check for m_Swapchain extension
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	//check for m_Swapchain Support
	bool m_SwapchainValid = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetail m_SwapchainSupportDetails = querySwapchainSupport(device);
		if (!m_SwapchainSupportDetails.formats.empty() && !m_SwapchainSupportDetails.presentModes.empty())
		{
			m_SwapchainValid = true;
		}
	}
	return indices.isComplete() && extensionsSupported && m_SwapchainValid;
}

SwapChainSupportDetail VulkanSetup::querySwapchainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetail details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}


QueueFamilyIndices VulkanSetup::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{

		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsQueueFamily = i;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indices.presentQueueFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}
		i++;
	}
	return indices;
}


bool VulkanSetup::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}
	return requiredExtensions.empty();

}

bool VulkanSetup::checkValidationLayerSupport()
{
	uint32_t layerCount;

	//get vaildation layer supported count
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
		{
			return false;
		}
	}

	return true;
}

VkResult VulkanSetup::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (function != nullptr)
	{
		return function(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}


void VulkanSetup::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanSetup::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

void VulkanSetup::populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	createInfo.messageType =	 VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
}

std::vector<const char*> VulkanSetup::getRequiredExtensions()
{
	uint32_t extensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionCount);
	if (m_ValidationLayersEnabled)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);	//add debug extension to the extension list
	}
	return extensions;
}


VkSurfaceFormatKHR VulkanSetup::chooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	//check if 2 formats are available ( RGB and BGR) 
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)			//this means all the formats are available and can choose anyone
	{
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };						//so we choose 
	}
	//if not free to choose any format then select from the available list
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
			availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM || availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
		{
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR VulkanSetup::chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	//VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;	
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSetup::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	// If current extent is at numeric limits, then extent can vary. Otherwise, it is the size of the window.
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(m_MainWindow, &width, &height);
		VkExtent2D actualExtent = { static_cast<uint32_t>(width),static_cast<uint32_t>(height) };
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
		return actualExtent;
	}
}






void VulkanSetup::createDepthBufferImage()
{
	VkFormat depthFormat = chooseSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT,VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	m_DepthBufferImage = createImage(m_SwapchainExtent.height, m_SwapchainExtent.width, depthFormat,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_DepthBufferMemory);
	m_DepthBufferImageView = createImageView(m_DepthBufferImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

}

VkFormat VulkanSetup::chooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags)
{
	for (VkFormat format : formats)
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &formatProperties);
		if (tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL & (formatProperties.optimalTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
	}
}



void VulkanSetup::createImageViews()
{
	//store images
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &imageCount, nullptr);
	std::vector<VkImage> images(imageCount);
	//m_m_SwapchainImages.resize(imageCount); 
	vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &imageCount, images.data());
	for (VkImage image : images)
	{
		SwapchainImages m_SwapchainImage = {};
		m_SwapchainImage.m_Image = image;

		//create image view (handles) for each image
		m_SwapchainImage.m_ImageView = createImageView(image, m_SwapchainImagesFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		m_SwapchainImages.push_back(m_SwapchainImage);
	}
}

//TODO : create Graphics Pipeline
void VulkanSetup::createGraphicsPipeline()
{
	//read the code 
	std::vector<char> vertShaderCode = readFile("Shaders/vert.spv");
	std::vector<char> fragShaderCode = readFile("Shaders/frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);		
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	//Vertex Shader Stage Creation
	VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {};
	vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageCreateInfo.module = vertShaderModule;
	vertShaderStageCreateInfo.pName = "main";
	vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

	//Fragment Shader Stage Creation 
	VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {};
	fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageCreateInfo.module = fragShaderModule;
	fragShaderStageCreateInfo.pName = "main";
	fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageCreateInfo , fragShaderStageCreateInfo };

	//Vertex Input Stage

	VkVertexInputBindingDescription vertInputBindingDescription = {};
	vertInputBindingDescription.binding = 0;
	vertInputBindingDescription.stride = sizeof(MeshData);
	vertInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription positionAttributeDesc = {};
	positionAttributeDesc.binding = 0;
	positionAttributeDesc.location = 0;
	positionAttributeDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
	positionAttributeDesc.offset = offsetof(MeshData, MeshData::positions);

	VkVertexInputAttributeDescription colorAttributeDesc = {};
	colorAttributeDesc.binding = 0;
	colorAttributeDesc.location = 1;
	colorAttributeDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
	colorAttributeDesc.offset = offsetof(MeshData, MeshData::colors);

	VkVertexInputAttributeDescription texCoordsAttributeDesc = {};
	texCoordsAttributeDesc.binding = 0;
	texCoordsAttributeDesc.location = 2;
	texCoordsAttributeDesc.format = VK_FORMAT_R32G32_SFLOAT;
	texCoordsAttributeDesc.offset = offsetof(MeshData, MeshData::uvs);

	VkVertexInputAttributeDescription normalAttributeDesc = {};
	normalAttributeDesc.binding = 0;
	normalAttributeDesc.location = 3;
	normalAttributeDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
	normalAttributeDesc.offset = offsetof(MeshData, MeshData::normals);

	std::array<VkVertexInputAttributeDescription, 4> inputAttributeDescriptions = { positionAttributeDesc  , colorAttributeDesc,texCoordsAttributeDesc, normalAttributeDesc };

	//vertInputBindingDescription.
	VkPipelineVertexInputStateCreateInfo inputStateCreateInfo = {};
	inputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	inputStateCreateInfo.vertexBindingDescriptionCount = 1;
	inputStateCreateInfo.pVertexBindingDescriptions = &vertInputBindingDescription;
	inputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(inputAttributeDescriptions.size());
	inputStateCreateInfo.pVertexAttributeDescriptions = inputAttributeDescriptions.data();

	//Input Assembly 
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
	inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; // Allow overriding of "strip" topology to start new primitives i.e changing topologies in b/w


	//VIEWPORT 
	VkViewport viewport;
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = m_SwapchainExtent.width;
	viewport.height = m_SwapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissors;
	//scissors.offset = VkOffset2D(static_cast<uint32_t>(0), static_cast<uint32_t>(0));
	scissors.offset = { 0,0 };
	scissors.extent = m_SwapchainExtent;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissors;


	//Rasterization INfo 
	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
	rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationStateCreateInfo.lineWidth = 1.0f;
	rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;

	//Multisampling 
	VkPipelineMultisampleStateCreateInfo multisamplingStateCreateInfo = {};
	multisamplingStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingStateCreateInfo.sampleShadingEnable = VK_FALSE;
	multisamplingStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//Blending 
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT	// Colours to apply blending to
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState.blendEnable = VK_TRUE;

	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;

	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
	colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
	//colorBlendStateCreateInfo.blendConstants= ; 

	std::array<VkDescriptorSetLayout, 3> descriptorSetLayouts = { m_DescriptorSetLayout ,m_SamplerSetLayout, m_DirectionalLightDescriptorSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(m_LogicalDevice, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Pipeline Layout!");
	}

	createRenderPass();

	//Depth Stencil State 
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = shaderStages;
	graphicsPipelineCreateInfo.pVertexInputState = &inputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisamplingStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = nullptr;
	graphicsPipelineCreateInfo.layout = m_PipelineLayout;
	graphicsPipelineCreateInfo.renderPass = m_RenderPass;
	graphicsPipelineCreateInfo.subpass = 0;		//its the index
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex = -1;


	if (vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Graphics Pipeline!");
	}

	//Destroy the shader modulelayout;s dont need them 
	vkDestroyShaderModule(m_LogicalDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(m_LogicalDevice, fragShaderModule, nullptr);
}

std::vector<char> VulkanSetup::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (!file.is_open())
	{
		std::string err = "Cannot open file " + filename;
		throw std::runtime_error(err.c_str());
	}

	size_t fileSize = (size_t)file.tellg();
	file.seekg(0);

	std::vector<char> fileData(fileSize);

	file.read(fileData.data(), fileSize);


	file.close();

	return fileData;
}

VkShaderModule VulkanSetup::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	createInfo.codeSize = code.size();

	VkShaderModule shaderModule;		//not a struct just a wrapper for bytecode of 
	if (vkCreateShaderModule(m_LogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create Shader Module");
	}
	return shaderModule;
}

VkCommandBuffer VulkanSetup::beginCommandBuffer(VkCommandPool commandPool)
{
	VkCommandBuffer commandBuffer;
	VkCommandBufferAllocateInfo commBufferAllocInfo = {};
	commBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commBufferAllocInfo.commandPool = commandPool;
	commBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commBufferAllocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(VulkanSetup::m_LogicalDevice, &commBufferAllocInfo, &commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Command Buffer");
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;


	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin Command Buffer");
	}
	return commandBuffer;
}

void VulkanSetup::endAndSubmitCommandBuffer(VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer)
{
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate tranfer Command Buffer");
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(VulkanSetup::m_LogicalDevice, commandPool, 1, &commandBuffer);
}

VkImage VulkanSetup::createImage(uint32_t height, uint32_t width, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags memPropertyFlags, VkDeviceMemory* deviceMemory)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.usage = useFlags;
	imageCreateInfo.format = format;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkImage image;
	if (vkCreateImage(m_LogicalDevice, &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_LogicalDevice, image, &memRequirements);

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memRequirements.size;
	memAllocInfo.memoryTypeIndex = Buffer::findMemoryTypeIndex(memRequirements.memoryTypeBits, memPropertyFlags);

	if (vkAllocateMemory(m_LogicalDevice, &memAllocInfo, nullptr, deviceMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Device Memory For Image!");
	}

	vkBindImageMemory(m_LogicalDevice, image, *deviceMemory, 0);
	return image;
}

VkImageView VulkanSetup::createImageView(VkImage image, VkFormat format, VkImageAspectFlags imageAspectFlags)
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	createInfo.format = format;
	createInfo.image = image;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = imageAspectFlags;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseMipLevel = 0;

	VkImageView imageView;
	if (vkCreateImageView(m_LogicalDevice, &createInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Image view!");
	}
	return imageView;
}

VkRenderPass VulkanSetup::createRenderPass()
{
	//Color attachment of renderPass 
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_SwapchainImagesFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;


	//Depth attachment of renderPass 
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = chooseSupportedFormat({ VK_FORMAT_D32_SFLOAT_S8_UINT,VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentReference = {};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;

	std::array<VkSubpassDependency, 2> subpassDependencies;


	// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transition must happen after...
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						// Subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;		// Pipeline stage
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;				// Stage access mask (memory access)
	// But must happen before...
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;


	// Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// Transition must happen after...
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;
	// But must happen before...
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	std::array<VkAttachmentDescription, 2> attachmentDescriptions{ colorAttachment, depthAttachment };
	//renderPass Struct 
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t> (subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	//VkRenderPass renderP; 
	//CreateRender Pass Object (error detection) 
	if (vkCreateRenderPass(m_LogicalDevice, &renderPassCreateInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create RenderPass!");
	}
	//return object
	//return renderP;
}

void VulkanSetup::createFramebuffers()
{
	m_SwapChainFramebuffers.resize(m_SwapchainImages.size());

	for (size_t i = 0; i < m_SwapChainFramebuffers.size(); i++)
	{
		std::array<VkImageView, 2> attachments = { m_SwapchainImages[i].m_ImageView, m_DepthBufferImageView };
		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();
		framebufferCreateInfo.renderPass = m_RenderPass;
		framebufferCreateInfo.width = m_SwapchainExtent.width;
		framebufferCreateInfo.height = m_SwapchainExtent.height;
		framebufferCreateInfo.layers = 1;

		if (vkCreateFramebuffer(m_LogicalDevice, &framebufferCreateInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Framebuffers!");
		}

	}
}

void VulkanSetup::createCommandPool()
{
	QueueFamilyIndices queueFamilies = findQueueFamilies(m_PhysicalDevice);

	VkCommandPoolCreateInfo commandPoolCreateInfo = { };
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilies.graphicsQueueFamily;

	if (vkCreateCommandPool(m_LogicalDevice, &commandPoolCreateInfo, nullptr, &m_GraphicsCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Command Pool for Graphics Queue Family");
	}
}

void VulkanSetup::createCommandBuffers()

{
	m_CommandBuffers.resize(m_SwapChainFramebuffers.size());
	VkCommandBufferAllocateInfo cmdBufferAllocateInfo = {};
	cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocateInfo.commandPool = m_GraphicsCommandPool;
	cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	if (vkAllocateCommandBuffers(m_LogicalDevice, &cmdBufferAllocateInfo, m_CommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Command Buffers!");
	}


}

void VulkanSetup::createSemaphore()
{
	m_ImageAvailableSemaphore.resize(MAX_FRAME_DRAWS);
	m_FinishedRenderingSemaphore.resize(MAX_FRAME_DRAWS);
	m_DrawFences.resize(MAX_FRAME_DRAWS);
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;


	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		if (vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &m_FinishedRenderingSemaphore[i]) != VK_SUCCESS ||
			vkCreateFence(m_LogicalDevice, &fenceCreateInfo, nullptr, &m_DrawFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create semaphores or fences");
		}

	}
}

void VulkanSetup::recordCommandBuffers(int imageIndex)
{

	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_RenderPass;
	renderPassBeginInfo.renderArea.offset = { 0, 0 };						// Start point of render pass in pixels
	renderPassBeginInfo.renderArea.extent = m_SwapchainExtent;				// Size of region to run render pass on (starting at offset)
	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0, 1.0f },
		clearValues[1].depthStencil.depth = 1.0f,

		renderPassBeginInfo.pClearValues = clearValues.data();							// List of clear values (TODO: Depth Attachment Clear Value)
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

	renderPassBeginInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];
	if (vkBeginCommandBuffer(m_CommandBuffers[imageIndex], &commandBufferBeginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to start recording a Command Buffer!");
	}

	vkCmdBeginRenderPass(m_CommandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	// Bind Pipeline to be used in render pass
	vkCmdBindPipeline(m_CommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

	//Draw all the models 
	for (size_t i = 0; i < m_Models.size(); i++)
	{
		uint32_t dynamicOffset = static_cast<uint32_t>(modelUniformAlignment) * (i);

		//Bind Desccriptor Sets 
		for (size_t j = 0; j < m_Models[i]->getMeshList().size(); j++)
		{
			VkBuffer vertexBuffers[] = { m_Models[i]->getMeshList()[j]->getVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };

			vkCmdBindVertexBuffers(m_CommandBuffers[imageIndex], 0, 1, vertexBuffers, offsets);


			vkCmdBindIndexBuffer(m_CommandBuffers[imageIndex], m_Models[i]->getMeshList()[j]->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);


			std::array<VkDescriptorSet, 3> descriptorSetGrp = { m_DescriptorSets[imageIndex],m_Meshes[0]->getTexture()->getTextureDescriptorSet(),m_DirectionalLightDescriptorSets[imageIndex] };

			vkCmdBindDescriptorSets(m_CommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0,
				static_cast<uint32_t>(descriptorSetGrp.size()), descriptorSetGrp.data(), 1, &dynamicOffset);


			vkCmdDrawIndexed(m_CommandBuffers[imageIndex], m_Models[i]->getMeshList()[j]->getIndexCount(), 1, 0, 0, 0);
		}

	}

	vkCmdEndRenderPass(m_CommandBuffers[imageIndex]);

	if (vkEndCommandBuffer(m_CommandBuffers[imageIndex]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to start recording a Command Buffer!");
	}




}

void VulkanSetup::drawFrame()
{
	uint32_t imageIndex;

	vkWaitForFences(m_LogicalDevice, 1, &m_DrawFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

	vkResetFences(m_LogicalDevice, 1, &m_DrawFences[currentFrame]);
	vkAcquireNextImageKHR(m_LogicalDevice, m_Swapchain, std::numeric_limits<uint64_t>::max(), m_ImageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);

	recordCommandBuffers(imageIndex);
	updateUniformBuffers(imageIndex);
	m_MeshAdapter->update(); 

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_ImageAvailableSemaphore[currentFrame];
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_FinishedRenderingSemaphore[currentFrame];

	QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);

	if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_DrawFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit the command buffer to graphics queue");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;	
	presentInfo.pWaitSemaphores = &m_FinishedRenderingSemaphore[currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_Swapchain;
	presentInfo.pImageIndices = &imageIndex;

	if (vkQueuePresentKHR(m_PresentQueue, &presentInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit to presentation Queue");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;

}


void VulkanSetup::updateModel(glm::mat4 newModel)
{
	m_Models[0]->m_UBOModel.model = newModel;
}

void VulkanSetup::createTextureSamplers()
{
	VkSamplerCreateInfo textureSamplerCreateInfo = {};
	textureSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	textureSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	textureSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	textureSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	textureSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	textureSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	textureSamplerCreateInfo.anisotropyEnable = VK_FALSE;
	//textureSamplerCreateInfo.maxAnisotropy = 16; 
	textureSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	textureSamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	textureSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	textureSamplerCreateInfo.minLod = 0.0f;
	textureSamplerCreateInfo.mipLodBias = 0.0f;
	textureSamplerCreateInfo.maxLod = 0.0f;

	if (vkCreateSampler(m_LogicalDevice, &textureSamplerCreateInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Texture Sampler!");
	}

}


void VulkanSetup::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding vpDescriptorSetLayoutBinding = {};
	vpDescriptorSetLayoutBinding.binding = 0;
	vpDescriptorSetLayoutBinding.descriptorCount = 1;
	vpDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vpDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	vpDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;


	VkDescriptorSetLayoutBinding modelDescriptorSetLayoutBinding = {};
	modelDescriptorSetLayoutBinding.binding = 1;
	modelDescriptorSetLayoutBinding.descriptorCount = 1;
	modelDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	modelDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	modelDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings{ vpDescriptorSetLayoutBinding,modelDescriptorSetLayoutBinding };

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t> (descriptorSetLayoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	if (vkCreateDescriptorSetLayout(m_LogicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Descriptor Set Layout!");
	}


	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo = {};
	textureLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	textureLayoutCreateInfo.bindingCount = 1;
	textureLayoutCreateInfo.pBindings = &samplerLayoutBinding;

	if (vkCreateDescriptorSetLayout(m_LogicalDevice, &textureLayoutCreateInfo, nullptr, &m_SamplerSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Texture Descriptor Set Layout!");
	}

	VkDescriptorSetLayoutBinding directionalLightLayoutBinding = {};
	directionalLightLayoutBinding.binding = 0;
	directionalLightLayoutBinding.descriptorCount = 1;
	directionalLightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	directionalLightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	directionalLightLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo directionalLightLayoutCreateInfo = {};
	directionalLightLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	directionalLightLayoutCreateInfo.bindingCount = 1;
	directionalLightLayoutCreateInfo.pBindings = &directionalLightLayoutBinding;

	if (vkCreateDescriptorSetLayout(m_LogicalDevice, &directionalLightLayoutCreateInfo, nullptr, &m_DirectionalLightDescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Directional Light Descriptor Set Layout!");
	}
}

void VulkanSetup::createDescriptorSetPool()
{
	//For viewProjection pool 
	VkDescriptorPoolSize vppoolSize = {};
	vppoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vppoolSize.descriptorCount = static_cast<uint32_t>(m_vpUniformBuffers.size());

	//For model pool 
	VkDescriptorPoolSize modelPoolSize = {};
	modelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	modelPoolSize.descriptorCount = static_cast<uint32_t>(m_ModelDynamicUniformBuffers.size());

	std::vector<VkDescriptorPoolSize> poolSizes = { vppoolSize ,modelPoolSize };

	//create Info for pools	
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(m_SwapchainImages.size());			// Maximum number of Descriptor Sets that can be created from pool
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t> (poolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

	if (vkCreateDescriptorPool(m_LogicalDevice, &descriptorPoolCreateInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Descriptor Pool!");
	}

	//sampler Descriptor Pool 
	VkDescriptorPoolSize samplerPoolSize;
	samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerPoolSize.descriptorCount = MAX_OBJECTS;

	VkDescriptorPoolCreateInfo samplerDescriptorPoolCreateInfo = {};
	samplerDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	samplerDescriptorPoolCreateInfo.maxSets = MAX_OBJECTS;
	samplerDescriptorPoolCreateInfo.poolSizeCount = 1;
	samplerDescriptorPoolCreateInfo.pPoolSizes = &samplerPoolSize;

	if (vkCreateDescriptorPool(m_LogicalDevice, &samplerDescriptorPoolCreateInfo, nullptr, &m_SamplerDescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Descriptor Pool!");
	}

	VkDescriptorPoolSize directionalLightPoolSize = {};
	directionalLightPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	directionalLightPoolSize.descriptorCount = static_cast<uint32_t>(m_DirectionalLightUniformBuffers.size());

	VkDescriptorPoolCreateInfo directionalLightPoolCreateInfo = {};
	directionalLightPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	directionalLightPoolCreateInfo.maxSets = static_cast<uint32_t>(m_DirectionalLightUniformBuffers.size());
	directionalLightPoolCreateInfo.poolSizeCount = 1;
	directionalLightPoolCreateInfo.pPoolSizes = &directionalLightPoolSize;

	if (vkCreateDescriptorPool(m_LogicalDevice, &directionalLightPoolCreateInfo, nullptr, &m_DirectionalLightPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Directional Light Descriptor Pool!");
	}

}

void VulkanSetup::createUniformBuffers()
{
	VkDeviceSize uniformBufferSize = sizeof(ViewProjection);
	VkDeviceSize modelBufferSize = modelUniformAlignment * MAX_OBJECTS;
	VkDeviceSize directionalLightUniformBufferSize = sizeof(DirectionalLightData);

	m_vpUniformBuffers.resize(m_SwapchainImages.size());
	m_ModelDynamicUniformBuffers.resize(m_SwapchainImages.size());
	m_DirectionalLightUniformBuffers.resize(m_SwapchainImages.size());

	//buffer for each frame 
	for (size_t i = 0; i < m_SwapchainImages.size(); i++)
	{
		m_vpUniformBuffers[i] = new Buffer(uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		m_ModelDynamicUniformBuffers[i] = new Buffer(modelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		m_DirectionalLightUniformBuffers[i] = new Buffer(directionalLightUniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	}

}

void VulkanSetup::allocateDescriptorSets()
{
	m_DescriptorSets.resize(m_SwapchainImages.size());
	m_DirectionalLightDescriptorSets.resize(m_SwapchainImages.size());

	std::vector<VkDescriptorSetLayout> setLayouts(m_SwapchainImages.size(), m_DescriptorSetLayout);			//requires setLayouts for all of the buffers in the array

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(m_SwapchainImages.size());
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.pSetLayouts = setLayouts.data();


	if (vkAllocateDescriptorSets(m_LogicalDevice, &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Descriptor Sets!");
	}

	for (size_t i = 0; i < m_SwapchainImages.size(); i++)
	{
		//viewProjection descriptorBufferInfo  Buffer info and data offset info
		VkDescriptorBufferInfo vpDescriptorBufferInfo = {};
		vpDescriptorBufferInfo.buffer = m_vpUniformBuffers[i]->getBuffer();
		vpDescriptorBufferInfo.offset = 0;
		vpDescriptorBufferInfo.range = sizeof(ViewProjection);

		VkWriteDescriptorSet vpWriteDescriptorSet = {};
		vpWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vpWriteDescriptorSet.descriptorCount = 1;
		vpWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vpWriteDescriptorSet.dstBinding = 0;
		vpWriteDescriptorSet.dstSet = m_DescriptorSets[i];
		vpWriteDescriptorSet.dstArrayElement = 0;
		vpWriteDescriptorSet.pBufferInfo = &vpDescriptorBufferInfo;

		//Model Buffer Binding Info
		VkDescriptorBufferInfo modelDescriptorBufferInfo = {};
		modelDescriptorBufferInfo.buffer = m_ModelDynamicUniformBuffers[i]->getBuffer();
		modelDescriptorBufferInfo.offset = 0;
		modelDescriptorBufferInfo.range = sizeof(UBOModel);

		VkWriteDescriptorSet modelWriteDescriptorSet = {};
		modelWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		modelWriteDescriptorSet.descriptorCount = 1;
		modelWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		modelWriteDescriptorSet.dstBinding = 1;
		modelWriteDescriptorSet.dstSet = m_DescriptorSets[i];
		modelWriteDescriptorSet.dstArrayElement = 0;
		modelWriteDescriptorSet.pBufferInfo = &modelDescriptorBufferInfo;

		std::vector<VkWriteDescriptorSet> writeSets = { vpWriteDescriptorSet,modelWriteDescriptorSet };
		vkUpdateDescriptorSets(m_LogicalDevice, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
	}

	std::vector<VkDescriptorSetLayout> directionalLightSetLayouts(m_SwapchainImages.size(), m_DirectionalLightDescriptorSetLayout);

	VkDescriptorSetAllocateInfo directionalLightDescriptorSetAllocInfo = {};
	directionalLightDescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	directionalLightDescriptorSetAllocInfo.descriptorPool = m_DirectionalLightPool;
	directionalLightDescriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(m_SwapchainImages.size());
	directionalLightDescriptorSetAllocInfo.pSetLayouts = directionalLightSetLayouts.data();

	if (vkAllocateDescriptorSets(m_LogicalDevice, &directionalLightDescriptorSetAllocInfo, m_DirectionalLightDescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Directional Light Descriptor Sets!");
	}

	for (size_t i = 0; i < m_DirectionalLightDescriptorSets.size(); i++)
	{
		VkDescriptorBufferInfo dLightBufferInfo = {};
		dLightBufferInfo.buffer = m_DirectionalLightUniformBuffers[i]->getBuffer();
		dLightBufferInfo.offset = 0;
		dLightBufferInfo.range = sizeof(DirectionalLightData);

		VkWriteDescriptorSet dLightWrite = {};
		dLightWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		dLightWrite.descriptorCount = 1;
		dLightWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		dLightWrite.dstBinding = 0;
		dLightWrite.dstSet = m_DirectionalLightDescriptorSets[i];
		dLightWrite.dstArrayElement = 0;
		dLightWrite.pBufferInfo = &dLightBufferInfo;

		std::vector<VkWriteDescriptorSet> writeSets = { dLightWrite };
		vkUpdateDescriptorSets(m_LogicalDevice, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
	}
}

void VulkanSetup::updateUniformBuffers(uint32_t imageIndex)
{
	void* data;
	vkMapMemory(m_LogicalDevice, m_vpUniformBuffers[imageIndex]->getDeviceMemory(), 0, sizeof(ViewProjection), 0, &data);
	memcpy(data, &m_VP, sizeof(ViewProjection));
	vkUnmapMemory(m_LogicalDevice, m_vpUniformBuffers[imageIndex]->getDeviceMemory());

	vkMapMemory(m_LogicalDevice, m_DirectionalLightUniformBuffers[imageIndex]->getDeviceMemory(), 0, sizeof(DirectionalLightData), 0, &data);
	memcpy(data, &m_DirectionalLight->getDirectionalLightData(), sizeof(DirectionalLightData));
	vkUnmapMemory(m_LogicalDevice, m_DirectionalLightUniformBuffers[imageIndex]->getDeviceMemory());


	for (size_t j = 0; j < m_Models.size(); j++)
	{
		UBOModel* thisModel = (UBOModel*)((uint64_t)m_ModelTransferSpace + j  * modelUniformAlignment);
		*thisModel = m_Models[j]->getModel();
	}

	// Map the list of model data
	vkMapMemory(m_LogicalDevice, m_ModelDynamicUniformBuffers[imageIndex]->getDeviceMemory(), 0, modelUniformAlignment * m_Models.size(), 0, &data);
	memcpy(data, m_ModelTransferSpace, modelUniformAlignment * m_Models.size());
	vkUnmapMemory(m_LogicalDevice, m_ModelDynamicUniformBuffers[imageIndex]->getDeviceMemory());
}

void VulkanSetup::allocateDynamicBufferTransferSpace()
{
	modelUniformAlignment = (sizeof(UBOModel) + m_MinUniformBufferOffsetAlignment - 1)
		& ~(m_MinUniformBufferOffsetAlignment - 1);

	m_ModelTransferSpace = (UBOModel*)_aligned_malloc(modelUniformAlignment * MAX_OBJECTS, modelUniformAlignment);
}
