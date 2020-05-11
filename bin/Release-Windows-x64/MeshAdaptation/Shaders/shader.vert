#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position ; 
layout(location = 1) in vec3 color; 
layout(location = 2) in vec2 uv; 
layout(location = 3) in vec3 normals; 

layout (binding = 0 ) uniform VP 
{
	mat4 view; 
	mat4 proj; 
} vp;

layout (binding = 1 ) uniform Model
{
	mat4 model;
} m_Model; 

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 texCoords; 
layout(location = 2) out vec3 Normals; 


void main() {
    gl_Position = vp.proj * vp.view *m_Model.model *vec4(position, 1.0);
    fragColor = color; 
	texCoords = uv; 
	Normals = normals; 
}