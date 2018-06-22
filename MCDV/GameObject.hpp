#pragma once
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <vector>

#include "Shader.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>



class GameObject {
public:
	glm::vec3 WorldPosition;
	glm::vec3 WorldRotation;

	Mesh* mesh = NULL;
	GameObject()
	{

	}

	GameObject(Mesh* _Mesh, glm::vec3 WorldPos, glm::vec3 WorldRot)
	{
		this->mesh = _Mesh;
		this->WorldPosition = WorldPos;
		this->WorldRotation = WorldRot;
	}

	GameObject(Mesh* _Mesh)
	{
		this->mesh = _Mesh;
		this->WorldPosition = glm::vec3(0, 0, 0);
		this->WorldRotation = glm::vec3(0, 0, 0);
	}

	GameObject(Mesh* _Mesh, glm::vec3 WorldPos)
	{
		this->mesh = _Mesh;
		this->WorldPosition = WorldPos;
		this->WorldRotation = glm::vec3(0, 0, 0);
	}

	~GameObject() {}

	void DrawMesh()
	{
		if (this->mesh != NULL)
			this->mesh->Draw();
	}

	glm::mat4 GetModelMatrix()
	{
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, this->WorldPosition);
		return model;
	}
};