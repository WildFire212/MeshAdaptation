#pragma once
#include"Buffer.h"
class IndexBuffer : public Buffer
{
private: 
	unsigned int m_IndexCount; 
public:
	IndexBuffer(std::vector<uint32_t>* indices, VkDeviceSize bufferSize,
		VkBufferUsageFlags additionalUsageFlags, VkMemoryPropertyFlags memPropertyFlags);
	unsigned int getIndexCount(); 
	~IndexBuffer(){}
private:
	void create(std::vector<uint32_t>* indices);

};

