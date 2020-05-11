#include "IndexBuffer.h"
#include"Setup/VulkanSetup.h"
IndexBuffer::IndexBuffer(std::vector<uint32_t>* indices, VkDeviceSize bufferSize,
	VkBufferUsageFlags additionalUsageFlags, VkMemoryPropertyFlags memPropertyFlags) : Buffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | additionalUsageFlags, memPropertyFlags)
{
	m_IndexCount = indices->size(); 
}

unsigned int IndexBuffer::getIndexCount()
{
	return m_IndexCount;
}

void IndexBuffer::create(std::vector<uint32_t>* indices)
{

}
