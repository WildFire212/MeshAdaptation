#include "Buffer.h"
#include "Setup/VulkanSetup.h"
Buffer::Buffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage,
	VkMemoryPropertyFlags memPropertyFlags)
{
	m_BufferSize = bufferSize; 
	createBuffer( bufferSize,  bufferUsage, memPropertyFlags);

	//m_BufferCount
	//createBuffer(physicalDe)
}

void Buffer::createBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage,
					VkMemoryPropertyFlags memPropertyFlags)
{
	VkBufferCreateInfo vertBufferCreateInfo = {};
	vertBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertBufferCreateInfo.usage = bufferUsage;
	vertBufferCreateInfo.size = bufferSize; 
	vertBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;				// Similar to Swap Chain images, can share vertex buffers

	if (vkCreateBuffer(VulkanSetup::m_LogicalDevice, &vertBufferCreateInfo, nullptr, &m_Buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("ERROR : Failed to create Vertex Buffer!");
	}

	//alright this is my vertexbuffer i have created what would be the memory requirements for my vert buffer
	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(VulkanSetup::m_LogicalDevice, m_Buffer, &memRequirements);

	//alright these are the memory requirements for my buffer i want u to allocate a memory for my buffer
	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memRequirements.size;
	//also these are the additional memory property flags give me the memory type index
	memAllocInfo.memoryTypeIndex = findMemoryTypeIndex(memRequirements.memoryTypeBits, memPropertyFlags);

	if (vkAllocateMemory(VulkanSetup::m_LogicalDevice, &memAllocInfo, nullptr, &m_DeviceMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("ERROR : Failed to allocate Device Memory for Vertex Buffer!");
	}

	//allocate memory to given vertex buffer
	vkBindBufferMemory(VulkanSetup::m_LogicalDevice, m_Buffer, m_DeviceMemory, 0);

}

uint32_t Buffer::findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags propertyFlags)
{
	VkPhysicalDeviceMemoryProperties availableMemProperties;
	vkGetPhysicalDeviceMemoryProperties(VulkanSetup::m_PhysicalDevice, &availableMemProperties);


	for (uint32_t i = 0; i < availableMemProperties.memoryTypeCount; i++)
	{
		if ((allowedTypes & (1 << i)) &&
			(availableMemProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
		{
			return i;
		}
	}
}


const VkBuffer& Buffer::getBuffer()
{
	return m_Buffer;
}

const VkDeviceMemory& Buffer::getDeviceMemory()
{
	return m_DeviceMemory;
}

const VkDeviceSize Buffer::getBufferSize()
{
	return m_BufferSize; 
}

void Buffer::copyBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, Buffer* srcBuffer,Buffer* dstBuffer,  VkDeviceSize bufferSize)
{

		VkCommandBuffer transferCommandBuffer= VulkanSetup::beginCommandBuffer(transferCommandPool); 

		VkBufferCopy bufferCopyRegion = {}; 
		bufferCopyRegion.srcOffset = 0;
		bufferCopyRegion.dstOffset = 0; 
		bufferCopyRegion.size = bufferSize; 
		vkCmdCopyBuffer(transferCommandBuffer, srcBuffer->getBuffer(), dstBuffer->getBuffer(), 1, &bufferCopyRegion);

		VulkanSetup::endAndSubmitCommandBuffer(transferCommandPool, transferQueue, transferCommandBuffer);

}

Buffer::~Buffer()
{
	vkDestroyBuffer(VulkanSetup::m_LogicalDevice, m_Buffer, nullptr);
	vkFreeMemory(VulkanSetup::m_LogicalDevice, m_DeviceMemory, nullptr);
}