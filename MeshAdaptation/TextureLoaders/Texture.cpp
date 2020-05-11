#include "Texture.h"
#include"Buffers/Buffer.h"
#include"Setup/VulkanSetup.h"
Texture::Texture(std::string fileName)
{
	createTexture(VulkanSetup::m_GraphicsQueue, VulkanSetup::m_GraphicsCommandPool, fileName); 
}


void Texture::createTexture(VkQueue transferQueue, VkCommandPool transferCommandPool, std::string fileName)
{
	stbi_uc* texData  = loadTextureFile(fileName);

	VkDeviceSize bufferSize = m_ImageSize;
	Buffer stagingBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(VulkanSetup::m_LogicalDevice, stagingBuffer.getDeviceMemory(), 0, bufferSize, 0, &data);
	memcpy(data, texData, (size_t)bufferSize);
	vkUnmapMemory(VulkanSetup::m_LogicalDevice, stagingBuffer.getDeviceMemory());

	stbi_image_free(texData);
	m_Image = new Image(static_cast<uint32_t>(m_Height), static_cast<uint32_t>(m_Width),
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	
	Image::transitionImageLayout(transferQueue, transferCommandPool, m_Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	Image::copyImageBuffer(transferQueue, transferCommandPool, &stagingBuffer, m_Image, m_Height, m_Width);

	Image::transitionImageLayout(transferQueue, transferCommandPool, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

}

stbi_uc* Texture::loadTextureFile(std::string fileName)
{
	std::string fileLoc = "Textures/" + fileName;
	stbi_uc* texData = stbi_load(fileLoc.c_str(), &m_Width, &m_Height, &m_Channels, STBI_rgb_alpha);

	if (!texData)
	{
		throw std::runtime_error("ERROR : Failed to load Texture File! : (" + fileName + ")");
	}

	m_ImageSize = m_Width * m_Height * STBI_rgb_alpha; 
	return texData; 
}

VkDescriptorSet Texture::getTextureDescriptorSet()
{
	return m_TextureDescriptorSet;
}

void Texture::createTextureDescriptor(VkDescriptorPool descriptorPool,VkDescriptorSetLayout samplerLayout,VkSampler sampler)
{
	VkDescriptorSetAllocateInfo textureDescriptorAllocInfo = {};
	textureDescriptorAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO; 
	textureDescriptorAllocInfo.descriptorPool = descriptorPool; 
	textureDescriptorAllocInfo.descriptorSetCount = 1; 
	textureDescriptorAllocInfo.pSetLayouts = &samplerLayout; 

	if (vkAllocateDescriptorSets(VulkanSetup::m_LogicalDevice, &textureDescriptorAllocInfo, &m_TextureDescriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("ERROR : Failed to allocate Texture Descriptor Set!");
	}
	VkDescriptorImageInfo descriptorImageInfo = {}; 
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorImageInfo.sampler = sampler;
	descriptorImageInfo.imageView = m_Image->getImageView(); 

	VkWriteDescriptorSet textureWrite= {}; 
	textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; 
	textureWrite.descriptorCount = 1; 
	textureWrite.pImageInfo = &descriptorImageInfo; 
	textureWrite.dstArrayElement = 0; 
	textureWrite.dstBinding = 0; 
	textureWrite.dstSet = m_TextureDescriptorSet;
	
	vkUpdateDescriptorSets(VulkanSetup::m_LogicalDevice, 1, &textureWrite, 0, nullptr);

}

void Texture::cleanUp()
{
	m_Height = 0;
	m_Width = 0;
	m_Channels = 0 ;
	delete m_Image; 
}


Texture::~Texture()
{
	cleanUp();
}
