#pragma once
#include"stb_image.h"
#include<string>
#include<stdexcept>
#include<vulkan/vulkan.h>
#include"Buffers/Image.h"
class Texture
{
private: 
	Image* m_Image; 
	VkDescriptorSet m_TextureDescriptorSet;
	int m_Height; 
	int m_Width; 
	int m_Channels; 
	VkDeviceSize m_ImageSize; 

public: 
	Texture(std::string fileName);
	~Texture(); 

	VkDescriptorSet getTextureDescriptorSet(); 
	void createTextureDescriptor(VkDescriptorPool descriptorPool, VkDescriptorSetLayout samplerLayout, VkSampler sampler);
private: 
	void createTexture(VkQueue transferQueue, VkCommandPool transferCommandPool,std::string fileName);
	stbi_uc* loadTextureFile(std::string fileName); 
	void cleanUp(); 
};

