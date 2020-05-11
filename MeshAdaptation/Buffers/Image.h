#pragma once
#include<vulkan/vulkan.h>
class Buffer; 
class Image
{
private:
	VkImage m_Image;
	VkImageView m_ImageView;
	VkDeviceMemory m_DeviceMemory;
	VkDeviceSize m_BufferSize;
	//maybe make physical Device and logical device static

public:
	Image(uint32_t height, uint32_t width, VkFormat format, VkImageTiling tiling, 
				VkImageUsageFlags useFlags, VkMemoryPropertyFlags memPropertyFlags);
	~Image();

	//getters and setters 
	const VkImage& getImage();
	const VkImageView& getImageView();
	const VkDeviceMemory& getDeviceMemory();
	const VkDeviceSize getImageSize();

	static void copyImageBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool,
		Buffer* srcBuffer, Image* dstImage,uint32_t height, uint32_t width);
	static void transitionImageLayout(VkQueue queue, VkCommandPool commandPool, Image* image, VkImageLayout oldLayout, VkImageLayout newLayout);
private: 

	VkImage createImage(uint32_t height, uint32_t width, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags useFlags, VkMemoryPropertyFlags memPropertyFlags); 
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags imageAspectFlags);
	void cleanUp(); 
};

