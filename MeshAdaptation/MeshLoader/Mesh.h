#pragma once
#include<GLFW/glfw3.h>
#define GLFW_INCLUDE_VULKAN
#include<glm.hpp>
#include<vector>
#include<stdexcept>
#include"Buffers/IndexBuffer.h"
#include"Buffers/VertexBuffer.h"
#include"TextureLoaders/Texture.h"
struct MeshData
{
	glm::vec3 positions;
	glm::vec3 colors; 
	glm::vec2 uvs;
	glm::vec3 normals; 
};
struct UBOModel {
		glm::mat4 model; 
};
class Mesh
{
public: 
private: 
	//VkBuffer m_VertexBuffer; 
	//VkDeviceMemory m_VertexBufferDeviceMemory;
	UBOModel m_Model;
	Texture* m_Texture; 
	VertexBuffer* m_VertexBuffer; 
	IndexBuffer * m_IndexBuffer; 
public: 
	MeshData m_MeshData; 
public : 
	Mesh(std::vector<MeshData>* vertices, std::vector<uint32_t> *indices,Texture* texture);
	~Mesh();
	VkBuffer getVertexBuffer();
	VkBuffer getIndexBuffer();
	Texture* getTexture(); 
	int getVertexCount();
	int getIndexCount() ;
	UBOModel getModel(); 

	//setters 
	void setModel(glm::mat4 model); 

private: 
	//void createVertexBuffer(std::vector<MeshData>* vertices);
	void cleanUp(); 

	void createBuffers(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<MeshData>* vertices, std::vector<uint32_t>* indices);

};

