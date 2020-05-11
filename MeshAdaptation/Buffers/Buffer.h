#pragma once
#include<vulkan/vulkan.h>
#include<vector>
//#include"Setup/VulkanSetup.h"

//Might change it to Buffer class only 

class Buffer
{
private: 
	VkBuffer m_Buffer; 
	VkDeviceMemory m_DeviceMemory; 
	VkDeviceSize m_BufferSize; 
	//maybe make physical Device and logical device static
	
public : 
	Buffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage,
		VkMemoryPropertyFlags memPropertyFlags);
	~Buffer(); 

	//getters and setters 
	const VkBuffer& getBuffer();
	const VkDeviceMemory& getDeviceMemory();
	const VkDeviceSize getBufferSize(); 

	static void copyBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, 
		Buffer* srcBuffer,Buffer* dstBuffer,  VkDeviceSize bufferSize); 
	static uint32_t findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags propertyFlags);
	
protected: 
	void createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage,
								VkMemoryPropertyFlags memPropertyFlags);



	//copy operations 

};

