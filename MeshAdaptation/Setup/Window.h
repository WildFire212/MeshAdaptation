#pragma once
#include<GLFW/glfw3.h>
#include<iostream>
class Window
{
private: 
	int m_WindowWidth, m_WindowHeight;
	GLFWwindow* m_MainWindow;

public:
	Window(int width , int height);
	~Window();
	
	void initWindow(); 
	bool getWindowShouldClose();
	GLFWwindow* getWindow(); 
	void cleanUp(); 
};

