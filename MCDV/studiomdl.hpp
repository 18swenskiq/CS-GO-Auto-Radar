#pragma once
#include <vector>
#include <map>

#include "vtx.hpp"
#include "vvd.hpp"

#include "vfilesys.hpp"

#include "Mesh.hpp"

class studiomdl {
	inline static std::map<std::string, studiomdl*> modelDict;

	int elementCount;
	unsigned int VBO, VAO, EBO;
public:
	studiomdl(const std::vector<float>& v, const std::vector<unsigned short>& i){
		this->elementCount = i.size();

		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);
		glGenBuffers(1, &this->EBO);

		// Attribute pointers
		glBindVertexArray(this->VAO);

		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), &v[0], GL_STATIC_DRAW);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, i.size() * sizeof(unsigned short), &i[0], GL_STATIC_DRAW);


		// Position XYZ
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// Normal XYZ
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
		glEnableVertexAttribArray(1);

		// TexCoords UV
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}

	/* Gets a studiomdl from filesystem */
	static studiomdl* getModel(const std::string& modelname, vfilesys* fs) {
		if(studiomdl::modelDict.count(modelname)) return modelDict[modelname];

		studiomdl* ptr = NULL;

		std::string basename = split(modelname, '.')[0];

		// Get required sub-files
		vtx_mesh* vtx = fs->get_resource_handle<vtx_mesh>(basename + ".dx90.vtx");
		vvd_data* vvd = fs->get_resource_handle<vvd_data>(basename + ".vvd");

		// If the file streams failed for some reason...
		if (vvd == NULL || vtx == NULL) goto IL_RET;

		// Mesh data (format XYZ:XYZ:UV)
		{
			std::vector<float> meshData;

			meshData.reserve(vvd->verticesLOD0.size()*8);

			for (auto&& vert: vvd->verticesLOD0) {
				meshData.push_back(vert.m_vecPosition.x);
				meshData.push_back(vert.m_vecPosition.y);
				meshData.push_back(vert.m_vecPosition.z);

				meshData.push_back(vert.m_vecNormal.x);
				meshData.push_back(vert.m_vecNormal.y);
				meshData.push_back(vert.m_vecNormal.z);

				meshData.push_back(vert.m_vecTexCoord.x);
				meshData.push_back(vert.m_vecTexCoord.y);
			}

			ptr = new studiomdl(meshData, vtx->vertexSequence);
		}
		// Delete memory from vtx/vvd and return pointer.
IL_RET:	delete vvd; delete vtx;
		studiomdl::modelDict.insert({ modelname, ptr });
		return ptr;
	}

	~studiomdl() {
		glDeleteVertexArrays(1, &this->VAO);
		glDeleteBuffers(1, &this->VBO);
		glDeleteBuffers(1, &this->EBO);
	}

	// Bind vertex array
	void Bind() {
		glBindVertexArray(this->VAO);
	}

	// Draw object (needs to be bound first)
	void Draw() {
		glDrawElements(GL_TRIANGLES, this->elementCount, GL_UNSIGNED_SHORT, 0);
	}
};