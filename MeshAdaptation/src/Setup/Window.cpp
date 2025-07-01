#include "Window.h"



Window::Window(int width, int height)
{
	m_WindowWidth = width;
	m_WindowHeight = height;
	initWindow(); 
}

void Window::initWindow()
{
	if (!glfwInit())
	{
		std::cout << "ERROR : Failed to initialize glfw!" << std::endl; 
		glfwTerminate(); 
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);			//TELL GLFW NOT TO CREATE OPENGL CONTEXT
	m_MainWindow = glfwCreateWindow(m_WindowWidth, m_WindowHeight, "Mesh Adaptation", nullptr, nullptr);

	glfwSetWindowUserPointer(m_MainWindow, this);
}

bool Window::getWindowShouldClose()
{
	return glfwWindowShouldClose(m_MainWindow);
}

GLFWwindow* Window::getWindow()
{
	return m_MainWindow;
}

void Window::cleanUp()
{
	m_WindowWidth = 0; 
	m_WindowHeight = 0; 
	glfwDestroyWindow(m_MainWindow); 
	glfwTerminate();
	
}


Window::~Window()
{
	cleanUp(); 
}
