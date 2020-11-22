#pragma once
#include <vector>
#include <wrl.h>
#include <d3d11.h>
#include "DXVertexBuffer.h"

enum DXMeshMode {
	POS_XYZ_TEXCOORD_UV,
	POS_XYZ_NORMAL_XYZ,
	SCREEN_SPACE_UV
};

class DXMesh {
	int elementCount;
	
public:
	unsigned int VBO, VAO;

	std::vector<float> vertices;

	DXMesh(DXRendering dxr, std::vector<float> vertices, DXMeshMode mode)
	{
		if (vertices.size() <= 0)
		{
			return;
		}

		if ((mode == DXMeshMode::POS_XYZ_NORMAL_XYZ))
		{
			this->vertices = vertices;
			this->elementCount = vertices.size() / 6;

			DXVertexBuffer* meshvertexbuffer = new DXVertexBuffer(dxr, vertices);
			meshvertexbuffer->Bind(dxr);

			// There is no direct equivalent to the OpenGL "Vertex Attribute Array". 
			// In DirectX 11, you just submit a distinct Draw for each group of 
			// like state settings (a.k.a material attribute).


		}
	}

};
