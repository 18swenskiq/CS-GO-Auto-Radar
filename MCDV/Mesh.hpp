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

enum MeshMode {
	POS_XYZ_TEXCOORD_UV,
	POS_XYZ_NORMAL_XYZ,
	SCREEN_SPACE_UV
};

class Mesh {
	int elementCount;

public:
	unsigned int VBO, VAO;

	std::vector<float> vertices;

	Mesh() {
		glGenVertexArrays(1, &this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	}

	Mesh(std::vector<float> vertices, MeshMode mode) {
		if (vertices.size() <= 0)
			return;

		if (mode == MeshMode::POS_XYZ_TEXCOORD_UV) {
			this->vertices = vertices;
			this->elementCount = vertices.size() / 5;

			glGenVertexArrays(1, &this->VAO);
			glGenBuffers(1, &this->VBO);

			glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

			glBindVertexArray(this->VAO);

			// position attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			//UV Coords
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
		}
		else if (mode == MeshMode::POS_XYZ_NORMAL_XYZ) {
			this->vertices = vertices;
			this->elementCount = vertices.size() / 6;

			glGenVertexArrays(1, &this->VAO);
			glGenBuffers(1, &this->VBO);

			glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

			glBindVertexArray(this->VAO);

			// position attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			// Normal vector
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
		}
		else if (mode == MeshMode::SCREEN_SPACE_UV) {
			this->vertices = vertices;
			this->elementCount = vertices.size() / 2;

			glGenVertexArrays(1, &this->VAO);
			glGenBuffers(1, &this->VBO);

			glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

			glBindVertexArray(this->VAO);

			// position attribute
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
		}
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
	}

	void Draw() {
		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, this->elementCount);
	}
};