#pragma once
#include"Buffer.h"
struct MeshData; 
class VertexBuffer : public Buffer
{
private: 
	int m_VertexCount; 
public: 
	VertexBuffer(std::vector<MeshData>* vertices, VkDeviceSize bufferSize,
		VkBufferUsageFlags additionalUsageFlags,VkMemoryPropertyFlags memPropertyFlags);
	~VertexBuffer() {}

	unsigned int getVertexCount(); 
private: 
	void create(std::vector<MeshData>* vertices);

};

