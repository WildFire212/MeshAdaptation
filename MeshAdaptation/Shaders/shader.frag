#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoords; 
layout(location = 2) in vec3 Normals; 
layout(set = 1, binding = 0) uniform sampler2D textureSampler; 

layout ( set = 2, binding = 0 ) uniform DirectionalLight{
vec3 direction; 	
float ambientIntensity;
float diffuseIntensity;	
}directionalLight;

layout(location = 0) out vec4 outColor;

vec3 colour = vec3(1.0,1.0,1.0); 

vec4 CalcLightByDirection( vec3 direction)
{
	
	vec4 ambientColour = vec4(colour, 1.0f) * directionalLight.ambientIntensity;
	
	float diffuseFactor = max(dot(normalize(Normals), normalize(direction)), 0.0f);
	vec4 diffuseColour = vec4(colour * directionalLight.diffuseIntensity * diffuseFactor, 1.0f);
	

	return (ambientColour + diffuseColour );
}

vec4 CalcDirectionalLight()
{
	return CalcLightByDirection( directionalLight.direction);
}



void main() {
    vec4 finalColour = CalcDirectionalLight();
    outColor =   finalColour * vec4(fragColor,1.0); //texture(textureSampler, texCoords) * 
}