#include "DirectionalLight.h"


DirectionalLight::DirectionalLight(){

}
DirectionalLight::DirectionalLight(glm::vec3 color , float aIntensity, float dIntensity, glm::vec3 dir) 
{
	//m_DirectionalLight.color  = color;			
	m_DirectionalLight.ambientIntensity= aIntensity;
	m_DirectionalLight.diffuseIntensity= dIntensity;
	m_DirectionalLight.direction = dir; 
}

DirectionalLight::~DirectionalLight()
{
}

DirectionalLightData DirectionalLight::getDirectionalLightData()
{
	return m_DirectionalLight;
}
	