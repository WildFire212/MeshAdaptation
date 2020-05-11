#include "Mesh.h"
#include"Setup/VulkanSetup.h"
Mesh::Mesh(std::vector<MeshData>* meshVertices, std::vector<uint32_t> * indices , Texture* texture)
{
	createBuffers(VulkanSetup::m_GraphicsQueue, VulkanSetup::m_GraphicsCommandPool, meshVertices, indices); 
	m_Model.model = glm::mat4(1.0); 
	m_Texture = texture;

}

Mesh::~Mesh()
{
	cleanUp(); 
}

VkBuffer Mesh::getVertexBuffer()
{
	return m_VertexBuffer->getBuffer();
}

VkBuffer Mesh::getIndexBuffer()
{
	return m_IndexBuffer->getBuffer();
}

Texture* Mesh::getTexture()
{
	return m_Texture;
}

int Mesh::getVertexCount()
{
	return m_VertexBuffer->getVertexCount();
}

int Mesh::getIndexCount()
{
	return m_IndexBuffer->getIndexCount();
}

UBOModel Mesh::getModel()
{
	return m_Model; 
}

void Mesh::setModel(glm::mat4 model)
{
	m_Model.model = model;
}


void Mesh::createBuffers(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<MeshData>* vertices, std::vector<uint32_t>* indices )
{
	//	-Vertex Buffer 
	VkDeviceSize bufferSize = sizeof(MeshData) * vertices->size(); 
	Buffer stagingBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	
	void* data;
	vkMapMemory(VulkanSetup::m_LogicalDevice,stagingBuffer.getDeviceMemory(), 0,bufferSize, 0, &data);
	memcpy(data, vertices->data(), (size_t)bufferSize);
	vkUnmapMemory(VulkanSetup::m_LogicalDevice, stagingBuffer.getDeviceMemory());

	m_VertexBuffer = new VertexBuffer(vertices, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Copy from staging buffer to GPU access buffer
	Buffer::copyBuffer(transferQueue, transferCommandPool,&stagingBuffer,  m_VertexBuffer, bufferSize);
	
	//staging Buffer goes out of scope and gets deleted (still for avoiding mem leaks free staging Buffer explicitly) 
	//free(stagingBuffer);

	//    -Index Buffer
	bufferSize = sizeof(uint32_t) * indices->size() ;
	//Buffer stagingBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	
	vkMapMemory(VulkanSetup::m_LogicalDevice, stagingBuffer.getDeviceMemory(), 0, bufferSize, 0, &data);
	memcpy(data, indices->data(), (size_t)bufferSize);
	vkUnmapMemory(VulkanSetup::m_LogicalDevice, stagingBuffer.getDeviceMemory());

	m_IndexBuffer = new IndexBuffer(indices, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Copy from staging buffer to GPU access buffer
	Buffer::copyBuffer(transferQueue, transferCommandPool, &stagingBuffer, m_IndexBuffer, bufferSize);
}



void Mesh::cleanUp()
{
	delete m_VertexBuffer; 
	delete m_IndexBuffer; 
	delete m_Texture; 
}
