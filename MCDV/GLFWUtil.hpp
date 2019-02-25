#pragma once
#include <GLFW\glfw3.h>

class util_keyHandler {
private:
	GLFWwindow* windowHandle;
	int* states_previous_frame = new int[GLFW_KEY_LAST]; // About a kb

public:
	
	bool getKeyDown(int key) {
		
		if ((states_previous_frame[key] != GLFW_PRESS) && (glfwGetKey(this->windowHandle, key) == GLFW_PRESS)) {
			states_previous_frame[key] = glfwGetKey(this->windowHandle, key);
			return true;
		}

		states_previous_frame[key] = glfwGetKey(this->windowHandle, key);

		return false;
	}

	bool getKey(int key) {
		if (glfwGetKey(this->windowHandle, key) == GLFW_PRESS)
			return true;

		return false;
	}

	util_keyHandler(GLFWwindow* window) {
		this->windowHandle = window;
	}
};