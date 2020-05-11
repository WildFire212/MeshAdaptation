#include "Model.h"
#include"MeshLoader/Mesh.h"
#include"Setup/VulkanSetup.h"

Model* Model::instance = nullptr; 

void Model::start()
{
	init(fileLoc); 
	
}
Model::Model(std::string fileName, bool joinIdenticalVertices) : 
	m_JoinIdenticalVertices(joinIdenticalVertices)
{
	m_UBOModel.model = glm::mat4(1.0);
	fileLoc = "Models/" + fileName; 
	//init(fileLoc); 
}

std::vector<MeshData>* Model::getVertices()
{
	return vertices;
}

std::vector<uint32_t>* Model::getIndices()
{
	return indices;
}

void Model::init(std::string fileLocation)
{
	Assimp::Importer importer;
	const aiScene* scene;
	if (m_JoinIdenticalVertices)
	{

	scene = importer.ReadFile(fileLocation, aiProcess_Triangulate| aiProcess_JoinIdenticalVertices|
											aiProcess_FlipUVs  | aiProcess_GenSmoothNormals);
	}
	else {
		scene = importer.ReadFile(fileLocation, aiProcess_Triangulate |
			aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

	}
	

	Mesh* mainMesh;
	vertices = new std::vector<MeshData>();
	indices = new std::vector<uint32_t>;
	loadNode(scene->mRootNode, scene); 
	alignMesh(); 

	
	
	for (size_t i = 0;  i < vertices->size();  i++)
	{
		vertices->at(i).colors = { 1.0,1.0,1.0 };
	}
	

	mainMesh = new Mesh(vertices, indices,nullptr);
	m_Mesh.push_back(mainMesh); 


}

void Model::createMesh()
{
	m_Mesh[0] = new Mesh(vertices, indices, nullptr);
}

std::vector<Texture*> Model::loadMaterials(const aiScene* scene)
{
	std::vector<std::string> textureList(scene->mNumMaterials);

	for (size_t i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i]; 

		textureList[i] = ""; 
		
		if (material->GetTextureCount(aiTextureType_DIFFUSE))
		{
			aiString path; 
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				int idx = std::string(path.data).rfind("\\");
				std::string fileName = std::string(path.data).substr(idx + 1); 
				textureList[i] = fileName; 
			}
		}
	}
	std::vector<Texture*> textures; 
	Texture* plainTexture = new Texture("plain.png");
	
	for (size_t i = 0; i < textureList.size(); i++)
	{
		if (textureList[i].empty())
		{
			textures.push_back(plainTexture);
		}
		else {
			Texture* texture = new Texture(textureList[i]);
			textures.push_back(texture);
		}
	}

	return textures; 
}

void Model::alignMesh()
{
	glm::vec3 minAABB(1000, 1000, 1000);
	glm::vec3 maxAABB(-1000, -1000, -1000);

	for (size_t i = 0; i < vertices->size(); i++)
	{
		glm::vec3 pos = vertices->at(i).positions;

		//For max AABBS
		if (pos.x > maxAABB.x)
		{
			maxAABB.x = pos.x;
		}
		if (pos.y > maxAABB.y)
		{
			maxAABB.y = pos.y;
		}
		if (pos.z > maxAABB.z)
		{
			maxAABB.z = pos.z;
		}

		//For min AABBS
		if (pos.x < minAABB.x)
		{
			minAABB.x = pos.x;
		}
		if (pos.y < minAABB.y)
		{
			minAABB.y = pos.y;
		}
		if (pos.z < minAABB.z)
		{
			minAABB.z = pos.z;
		}
	}

	float longestSide = (maxAABB.x - minAABB.x);

	if ((maxAABB.y - minAABB.y) > longestSide)
	{
		longestSide = maxAABB.y - minAABB.y;
	}
	if ((maxAABB.z - minAABB.z) > longestSide)
	{
		longestSide = maxAABB.z - minAABB.z;
	}
	
	glm::vec3 translate = { -0.5f * (maxAABB.x + minAABB.x),-0.5f * (maxAABB.y + minAABB.y), -0.5f * (maxAABB.z + minAABB.z) };

	for (size_t i = 0; i < vertices->size(); i++)
	{
		vertices->at(i).positions.x += translate.x; 
		vertices->at(i).positions.y += translate.y; 
		vertices->at(i).positions.z += translate.z; 

		vertices->at(i).positions.x *= (1.0f/longestSide);
		vertices->at(i).positions.y *= (1.0f/longestSide);
		vertices->at(i).positions.z *= (1.0f/longestSide);	
	}
}

void Model::cleanUp()
{
	for (size_t i = 0; i < m_Mesh.size(); i++)
	{
		delete m_Mesh[i];
	}

	delete vertices;
	delete indices;
}


 std::vector<Mesh*>& Model::getMeshList()  
{
	return m_Mesh; 
}

UBOModel Model::getModel()
{
	return m_UBOModel;
}

void Model::createTextureDescriptors(VkDescriptorPool descriptorPool, VkDescriptorSetLayout samplerLayout, VkSampler sampler)
{

}

void Model::loadNode(const aiNode* node , const aiScene* scene)
{
	//load all meshes in current node 
	for (size_t i = 0; i < node->mNumMeshes ; i++)
	{
		loadMesh(scene->mMeshes[node->mMeshes[i]], scene); 
	}

	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		loadNode(node->mChildren[i], scene); 
	}
}

void Model::loadMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<MeshData>* vertices1= new std::vector<MeshData>(mesh->mNumVertices);
	
	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		vertices1->at(i).positions = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

		if (mesh->mTextureCoords[0])
		{
			vertices1->at(i).uvs = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		}
		else
		{
			vertices1->at(i).uvs = { 0.0f,0.0f };
		}

		vertices1->at(i).colors = { 0.0f,0.0f,0.0f };

		
			vertices1->at(i).normals = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
		

	}

	
	
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < mesh->mFaces[i].mNumIndices; j++)
		{
			indices->push_back(face.mIndices[j]);
		}
	}

	for (size_t i = 0; i < vertices1->size(); i++)
	{
		vertices->push_back(vertices1->at(i)); 
	}

	meshToTex.push_back(mesh->mMaterialIndex); 
	//Mesh* newMesh = new Mesh(vertices, indices, m_Textures[mesh->mMaterialIndex]);
	//m_ModelMeshes.push_back(newMesh); 
}

Model::~Model()
{
	cleanUp(); 
}


Model& Model::getThisModel()
{
	// TODO: insert return statement here
	return *instance;
}
