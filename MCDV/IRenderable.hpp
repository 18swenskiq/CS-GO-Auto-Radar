#pragma once
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "Shader.hpp"

class IRenderable {
public:
	glm::vec3 position;
	glm::vec3 eulerAngles;

	Mesh* m_mesh = NULL;

	virtual void _Draw(Shader* shader, std::vector<glm::mat4> transform_stack = {}) = 0;
	virtual void SetupDrawable() = 0;

	void Draw(Shader* shader) {
		if (this->m_mesh == NULL) SetupDrawable();
		this->_Draw(shader);
	}
};