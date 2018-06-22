#pragma once
#include <GLFW\glfw3.h>

class util_keyHandler {
private:
	GLFWwindow* windowHandle;
public:
	bool getKeyDown(int key) {
		if (glfwGetKey(this->windowHandle, key) == GLFW_PRESS)
			return true;
	}

	util_keyHandler(GLFWwindow* window) {
		this->windowHandle = window;
	}
};