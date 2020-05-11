#pragma once
#include <glm.hpp>

struct DirectionalLightData
{
	//glm::vec3 color; 
	glm::vec3 direction;
	float ambientIntensity;
	float diffuseIntensity;
};

class DirectionalLight 
{
public:
	DirectionalLight();
	DirectionalLight(glm::vec3 color , float aIntensity, float dIntensity, glm::vec3 dir);
	~DirectionalLight();

	DirectionalLightData getDirectionalLightData(); 
private:
	DirectionalLightData m_DirectionalLight; 
};

