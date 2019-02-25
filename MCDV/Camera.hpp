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




	//Used for true fps control
	glm::vec3 travelFront = glm::vec3(0.0f, 0.0f, 1.0f);



	util_keyHandler* keyHandler;
public:
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -16.0f);

	float yaw = 90.0f;
	float pitch = 0.0f;

	float sensitivity = 0.05f;
	float speed = 2.5f;
	float fov = 90.0f;

	float near_z = 0.1f;
	float far_z = 1000.0f;

	float pitch_max = 89.0f;
	float pitch_min = -89.0f;

	int window_width = 800;
	int window_height = 600;

	bool isNoclip = true;

	void handleInput(float deltaTime);

	void mouseUpdate(double xpos, double ypos, bool isClicking = true);

	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();

	glm::vec3 getViewRay(double xpos, double ypos, int viewWidth, int viewHeight);

	Camera(util_keyHandler* keyHandler);
	Camera();
	~Camera();
};

glm::vec2 getNormalizedDeviceCoords(double xpos, double ypos, int viewWidth, int viewHeight) {
	float x = (2.0f * xpos) / viewWidth - 1.0f;
	float y = (2.0f * ypos) / viewHeight - 1.0f;

	return glm::vec2(x, -y);
}

glm::vec4 calculateViewCoords(glm::vec4 clipCoords, glm::mat4 projectionMatrix)
{
	glm::mat4 inverted = glm::inverse(projectionMatrix);
	glm::vec4 viewCoords = inverted * clipCoords;
	return glm::vec4(viewCoords.x, viewCoords.y, -1.0f, 0.0f);
}

glm::vec3 calculateWorldVector(glm::vec4 viewCoords, glm::mat4 viewMatrix)
{
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

glm::vec3 Camera::getViewRay(double xpos, double ypos, int viewWidth, int viewHeight)
{
	return calculateMouseRay(xpos, ypos, viewWidth, viewHeight, this->getProjectionMatrix(), this->getViewMatrix());
}


Camera::Camera()
{

}

void Camera::handleInput(float deltaTime)
{
	glm::vec3 travelDir = cameraFront;

	if (!isNoclip)
		travelDir = this->travelFront;

	if (keyHandler->getKey(GLFW_KEY_LEFT_SHIFT))
		this->speed = 20.0f;
	else
		this->speed = 2.5f;

	if (keyHandler->getKey(GLFW_KEY_W))
		cameraPos += speed * travelDir * deltaTime;

	if (keyHandler->getKey(GLFW_KEY_S))
		cameraPos -= speed * travelDir * deltaTime;

	if (keyHandler->getKey(GLFW_KEY_A))
		cameraPos -= glm::normalize(glm::cross(travelDir, cameraUp)) * speed * deltaTime;

	if (keyHandler->getKey(GLFW_KEY_D))
		cameraPos += glm::normalize(glm::cross(travelDir, cameraUp)) * speed * deltaTime;
}

void Camera::mouseUpdate(double xpos, double ypos, bool isClicking)
{
	if (isClicking && (isClicking != this->lastStatus)) {
		lastX = xpos;
		lastY = ypos;
	}

	this->lastStatus = isClicking;

	if (!isClicking)
		return;

	//Removes first movement skips
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;

		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	xoffset *= (fov / 90.0f) * this->sensitivity;
	yoffset *= (fov / 90.0f) * this->sensitivity;

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
	this->travelFront = glm::normalize(tFront);
}

glm::mat4 Camera::getViewMatrix()
{
	//return glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 10) + glm::vec3(0, 0, -1), glm::vec3(1, 0, 0));
	return glm::lookAt(this->cameraPos, this->cameraPos + this->cameraFront, this->cameraUp);
}

glm::mat4 Camera::getProjectionMatrix()
{
	//return glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.0f);
	return glm::perspective(glm::radians(fov / 2), (float)window_width / (float)window_height, near_z, far_z);
}

Camera::Camera(util_keyHandler* keyHandler)
{
	this->keyHandler = keyHandler;
}

Camera::~Camera()
{
}


