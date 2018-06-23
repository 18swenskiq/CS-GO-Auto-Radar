#pragma once

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "GLFWUtil.hpp"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

class Mesh {
	int elementCount;

public:
	unsigned int VBO, VAO;

	std::vector<float> vertices;

	Mesh() {
		glGenVertexArrays(1, &this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	}

	Mesh(std::vector<float> vertices) {
		if (vertices.size() <= 0)
			return;

		this->vertices = vertices;
		this->elementCount = vertices.size() / 6;

		// first, configure the cube's VAO (and VBO)
		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);

		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

		glBindVertexArray(this->VAO);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		//Normal vector
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	~Mesh() {
		glDeleteVertexArrays(1, &this->VAO);
		glDeleteBuffers(1, &this->VBO);

		std::cout << "DESTRUCTED" << std::endl;
	}

	void Draw() {
		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, this->elementCount);
	}
};

class VertAlphaMesh {
	int elementCount;

public:
	unsigned int VBO, VAO;

	std::vector<float> vertices;

	VertAlphaMesh() {
		glGenVertexArrays(1, &this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	}

	VertAlphaMesh(std::vector<float> vertices) {
		if (vertices.size() <= 0)
			return;

		this->vertices = vertices;
		this->elementCount = vertices.size() / 7;

		// first, configure the cube's VAO (and VBO)
		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);

		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

		glBindVertexArray(this->VAO);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		//Normal vector
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		//alpha
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	~VertAlphaMesh() {
		glDeleteVertexArrays(1, &this->VAO);
		glDeleteBuffers(1, &this->VBO);

		std::cout << "DESTRUCTED" << std::endl;
	}

	void Draw() {
		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, this->elementCount);
	}
};