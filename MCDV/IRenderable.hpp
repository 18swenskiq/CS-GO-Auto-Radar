#pragma once

#ifdef DXBUILD
#include <d3d11.h>
#include <DirectXMath.h>
#include "DXShaderCombo.h"
#include "DXMesh.h"
#endif

#ifdef GLBUILD
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "Shader.hpp"
#endif

class IRenderable {
public:
	#ifdef DXBUILD
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 eulerAngles;

	DXMesh* m_mesh = NULL;

	//virtual void _Draw(DXShaderCombo* shader, std::vector<DirectX::XMMATRIX> transform_stack = {}) = 0;
	virtual void _Draw(DXShaderCombo* shader, DXRendering& dxr, std::vector<DirectX::XMMATRIX> transform_stack = {}) = 0;
	virtual void SetupDrawable(DXRendering& dxr) = 0;

	void Draw(DXShaderCombo* shader, DXRendering& dxr)
	{
		if (this->m_mesh = NULL) SetupDrawable(dxr);
		this->_Draw(shader, dxr);
	}
	#endif

	#ifdef GLBUILD
	glm::vec3 position;
	glm::vec3 eulerAngles;

	Mesh* m_mesh = NULL;

	virtual void _Draw(Shader* shader, std::vector<glm::mat4> transform_stack = {}) = 0;
	virtual void SetupDrawable() = 0;

	void Draw(Shader* shader) {
		if (this->m_mesh == NULL) SetupDrawable();
		this->_Draw(shader);
	}
	#endif
};