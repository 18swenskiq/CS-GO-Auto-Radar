#pragma once

#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "GLFWUtil.hpp"

#include <iostream>

class Camera
{
private:
	bool firstMouse = true;
	bool lastStatus = false;

	double lastX = 400;
	double lastY = 300;
public:
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -16.0f);

	float yaw = 90.0f;
	float pitch = 0.0f;

	float sensitivity = 0.05f;
	float speed = 2.5f;
	float fov = 90.0f;

	float near_z = 32.0f;
	float far_z = 10000.0f;

	float pitch_max = 89.0f;
	float pitch_min = -89.0f;

	bool isDirty = true;

	// Process movement from GLFW
	void handleInput(GLFWwindow* hWindow, float deltaTime){
		if (glfwGetKey(hWindow, GLFW_KEY_LEFT_SHIFT))
			this->speed = 2000.0f;
		else
			this->speed = 200.0f;

		if (glfwGetKey(hWindow, GLFW_KEY_W))
			cameraPos += speed * this->cameraFront * deltaTime;

		if (glfwGetKey(hWindow, GLFW_KEY_S))
			cameraPos -= speed * this->cameraFront * deltaTime;

		if (glfwGetKey(hWindow, GLFW_KEY_A))
			cameraPos -= glm::normalize(glm::cross(this->cameraFront, cameraUp)) * speed * deltaTime;

		if (glfwGetKey(hWindow, GLFW_KEY_D))
			cameraPos += glm::normalize(glm::cross(this->cameraFront, cameraUp)) * speed * deltaTime;

		if (glfwGetKey(hWindow, GLFW_KEY_W) || glfwGetKey(hWindow, GLFW_KEY_S) || glfwGetKey(hWindow, GLFW_KEY_A) || glfwGetKey(hWindow, GLFW_KEY_D)) this->isDirty = true;
	}

	void mouseUpdate(const double& xpos, const double& ypos, const bool& isClicking = true){
		if (isClicking && (isClicking != this->lastStatus)) {
			this->lastX = xpos;
			this->lastY = ypos;
		}

		this->lastStatus = isClicking;

		if (!isClicking)
			return;

		this->isDirty = true;

		//Removes first movement skips
		if (firstMouse){
			this->lastX = xpos;
			this->lastY = ypos;

			this->firstMouse = false;
		}

		float xoffset = xpos - this->lastX;
		float yoffset = this->lastY - ypos;

		this->lastX = xpos;
		this->lastY = ypos;

		xoffset *= (this->fov / 90.0f) * this->sensitivity;
		yoffset *= (this->fov / 90.0f) * this->sensitivity;

		this->yaw = glm::mod(this->yaw + xoffset, 360.0f);
		this->pitch = glm::clamp(this->pitch + yoffset, this->pitch_min, this->pitch_max);

		//Front facing vector
		glm::vec3 front;
		front.x = cos(glm::radians(this->pitch)) * cos(glm::radians(this->yaw));
		front.y = sin(glm::radians(this->pitch));
		front.z = cos(glm::radians(this->pitch)) * sin(glm::radians(this->yaw));

		//Used for fps movement
		glm::vec3 tFront;
		tFront.x = cos(glm::radians(this->yaw));
		tFront.z = sin(glm::radians(this->yaw));
		tFront.y = 0;

		//Update class vectors
		this->cameraFront = glm::normalize(front);
	}

	void startFrame() {
		this->isDirty = false;
	}

	// Create view matrix from current state
	glm::mat4 getViewMatrix() const {
		return glm::lookAt(this->cameraPos, this->cameraPos + this->cameraFront, this->cameraUp);
	}

	// Create projection matrix from current state
	glm::mat4 getProjectionMatrix(const int& window_width, const int& window_height) const {
		return glm::perspective(glm::radians(this->fov / 2), (float)window_width / (float)window_height, near_z, far_z);
	}

	// Get a ray pointing from mouse into 3d space
	glm::vec3 getViewRay(const double& xpos, const double& ypos, const int& window_width, const int& window_height) const;

	// Constructors
	Camera(const glm::vec3& cam_pos, const glm::vec3& cam_dir):
		cameraPos(cam_pos){
		this->cameraFront = glm::normalize(cam_dir);
	}

	Camera(const glm::vec3& cam_pos) :
		cameraPos(cam_pos) { }
};

glm::vec2 getNormalizedDeviceCoords(double xpos, double ypos, int viewWidth, int viewHeight) {
	float x = (2.0f * xpos) / viewWidth - 1.0f;
	float y = (2.0f * ypos) / viewHeight - 1.0f;

	return glm::vec2(x, -y);
}

glm::vec4 calculateViewCoords(glm::vec4 clipCoords, glm::mat4 projectionMatrix){
	glm::mat4 inverted = glm::inverse(projectionMatrix);
	glm::vec4 viewCoords = inverted * clipCoords;
	return glm::vec4(viewCoords.x, viewCoords.y, -1.0f, 0.0f);
}

glm::vec3 calculateWorldVector(glm::vec4 viewCoords, glm::mat4 viewMatrix){
	glm::mat4 inverted = glm::inverse(viewMatrix);
	glm::vec4 worldCoord = inverted * viewCoords;
	glm::vec3 mouseRay = glm::vec3(worldCoord.x, worldCoord.y, worldCoord.z);
	mouseRay = glm::normalize(mouseRay);
	return mouseRay;
}

glm::vec3 calculateMouseRay(double xpos, double ypos, int viewWidth, int viewHeight, glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
	glm::vec2 normalizedCoords = getNormalizedDeviceCoords(xpos, ypos, viewWidth, viewHeight);
	glm::vec4 clipCoords = glm::vec4(normalizedCoords.x, normalizedCoords.y, -1.0f, 1.0f);
	glm::vec4 viewCoords = calculateViewCoords(clipCoords, projectionMatrix);
	glm::vec3 worldRay = calculateWorldVector(viewCoords, viewMatrix);
	return worldRay;
}

glm::vec3 Camera::getViewRay(const double& xpos, const double& ypos, const int& window_width, const int& window_height) const {
	return calculateMouseRay(xpos, ypos, window_width, window_height, this->getProjectionMatrix(window_width, window_height), this->getViewMatrix());
}