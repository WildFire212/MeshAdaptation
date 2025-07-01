#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include<iostream>
#include"Setup/Window.h"
#include"Setup/VulkanSetup.h"

int main() {
	
	Window* window = new Window(1366, 786);
	VulkanSetup* vulkanSetup = new VulkanSetup(window->getWindow());
	
	float deltaTime = 0.0f;
	float lastTime = 0.0f;
	float newTime = 0.0f;
	float angle = 0.0f;


	while (!window->getWindowShouldClose())
	{
		newTime = glfwGetTime();
		deltaTime = newTime - lastTime;
		glfwPollEvents();
		angle += 10 * deltaTime;
		if (angle > 360)
		{
			angle = 0.0f;
		}
		//vulkanSetup->updateModel(glm::rotate(glm::mat4(1.0),(angle),  glm::vec3(0.0f, 1.0f, 0.0f)));
		vulkanSetup->drawFrame();
	}

	delete vulkanSetup; 
	delete window; 
	return 0; 
}