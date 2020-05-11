#include "VertexBuffer.h"
#include"MeshLoader/Mesh.h"
#include"Setup/VulkanSetup.h"

VertexBuffer::VertexBuffer(std::vector<MeshData>* vertices, VkDeviceSize bufferSize,
	VkBufferUsageFlags additionalUsageFlags, VkMemoryPropertyFlags memPropertyFlags) : Buffer(bufferSize,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|additionalUsageFlags,memPropertyFlags)	
{
	m_VertexCount = vertices->size(); 
}

unsigned int VertexBuffer::getVertexCount()
{
	return m_VertexCount;
}

void VertexBuffer::create(std::vector<MeshData>* vertices)
{
}
