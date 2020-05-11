#pragma once
#include<assimp/scene.h>
#include<assimp/Importer.hpp>
#include<assimp/postprocess.h>
#include<vector>
#include<map>
#include"MeshLoader/Mesh.h"

class Model
{
private: 
	bool m_JoinIdenticalVertices; 
	std::vector<Mesh*>  m_Mesh;
	std::vector<MeshData>* vertices;
	std::vector<uint32_t>* indices;
	std::vector<int> meshToTex;			
public: 
	void createMesh(); 
	std::string fileLoc;	
	void start(); 
	static Model* instance; 
	static Model& getThisModel();
	UBOModel m_UBOModel;
	Model(std::string fileName,bool joinIdenticalVertices); 
	~Model(); 
	
	std::vector<Mesh*>& getMeshList(); 
	UBOModel getModel();
	void createTextureDescriptors(VkDescriptorPool descriptorPool, VkDescriptorSetLayout samplerLayout, VkSampler sampler);
	

	std::vector<MeshData>* getVertices();
	std::vector<uint32_t>* getIndices();
private: 
	void init(std::string fileLoc); 
	void loadNode(const aiNode* node, const aiScene* scene);
	void loadMesh(aiMesh* mesh, const aiScene* scene); 
	std::vector<Texture*> loadMaterials(const aiScene* scene); 
	
	void alignMesh(); 
	void cleanUp(); 

};

