#pragma once
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "Shader.hpp"

class IRenderable {
public:
	bool isGen = false;

	virtual void _Draw(Shader* shader) = 0;
	virtual void _Init() = 0;

	void Draw(Shader* shader) {
		if (!this->isGen) { this->_Init(); this->isGen = true; };
		this->_Draw(shader);
	}
};