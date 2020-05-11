#include "Image.h"
#include"Setup/VulkanSetup.h"

Image::Image(uint32_t height, uint32_t width, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags memPropertyFlags)
{
	m_Image = createImage(height, width, format, tiling, useFlags, memPropertyFlags);
	m_ImageView = createImageView(m_Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT); 
}

Image::~Image()
{
	cleanUp(); 
}

const VkImage& Image::getImage()
{
	return m_Image;
}

const VkImageView& Image::getImageView()
{
	return m_ImageView; 
}

const VkDeviceMemory& Image::getDeviceMemory()
{
	return m_DeviceMemory; 
}

const VkDeviceSize Image::getImageSize()
{
	return m_BufferSize;
}

void Image::copyImageBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, Buffer* srcBuffer,
								Image* dstImage,uint32_t height, uint32_t width)
{
	VkCommandBuffer transferCommandBuffer = VulkanSetup::beginCommandBuffer(transferCommandPool);
	
	VkBufferImageCopy bufferImageCopyRegion = {};
	bufferImageCopyRegion.bufferImageHeight = 0; 
	bufferImageCopyRegion.bufferOffset = 0; 
	bufferImageCopyRegion.bufferRowLength= 0; 
	bufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
	bufferImageCopyRegion.imageSubresource.layerCount= 1; 
	bufferImageCopyRegion.imageSubresource.baseArrayLayer = 0; 
	bufferImageCopyRegion.imageSubresource.mipLevel= 0; 
	bufferImageCopyRegion.imageExtent = {width , height, 1};
	bufferImageCopyRegion.imageOffset = { 0,0,0 };


	vkCmdCopyBufferToImage(transferCommandBuffer, srcBuffer->getBuffer(), dstImage->getImage(),VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1, &bufferImageCopyRegion);

	VulkanSetup::endAndSubmitCommandBuffer(transferCommandPool, transferQueue, transferCommandBuffer);
}

void Image::transitionImageLayout(VkQueue queue, VkCommandPool commandPool, Image* image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer imageTransitionCommandBuffer = VulkanSetup::beginCommandBuffer(commandPool); 

	VkImageMemoryBarrier imageMemoryBarrier = {}; 
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; 
	imageMemoryBarrier.image = image->getImage(); 
	imageMemoryBarrier.oldLayout = oldLayout; 
	imageMemoryBarrier.newLayout = newLayout; 
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	VkPipelineStageFlags srcStage; 
	VkPipelineStageFlags dstStage;

	//2 transitions from undefined to transfer 
	//and then transfer to shader read 

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = 0;									// Memory access stage transition must after...
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;		// Memory access stage transition must before...
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; 
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT; 
	}
	
	if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;	// Memory access stage transition must after...
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;		// Memory access stage transition must before...
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; 
	}

	vkCmdPipelineBarrier(imageTransitionCommandBuffer, srcStage, dstStage, 0 ,0
		, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);

	VulkanSetup::endAndSubmitCommandBuffer(commandPool, queue,imageTransitionCommandBuffer);

}

VkImage Image::createImage(uint32_t height, uint32_t width, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags memPropertyFlags)
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
	if (vkCreateImage(VulkanSetup::m_LogicalDevice, &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(VulkanSetup::m_LogicalDevice, image, &memRequirements);

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memRequirements.size;
	memAllocInfo.memoryTypeIndex = Buffer::findMemoryTypeIndex(memRequirements.memoryTypeBits, memPropertyFlags);

	if (vkAllocateMemory(VulkanSetup::m_LogicalDevice, &memAllocInfo, nullptr, &m_DeviceMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Device Memory For Image!");
	}

	vkBindImageMemory(VulkanSetup::m_LogicalDevice, image, m_DeviceMemory, 0);
	return image;
}

VkImageView Image::createImageView(VkImage image, VkFormat format, VkImageAspectFlags imageAspectFlags)
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
	if (vkCreateImageView(VulkanSetup::m_LogicalDevice, &createInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Image view!");
	}
	return imageView;
}

void Image::cleanUp()
{
	vkDestroyImage(VulkanSetup::m_LogicalDevice, m_Image, nullptr);
	vkDestroyImageView(VulkanSetup::m_LogicalDevice, m_ImageView, nullptr);
}

