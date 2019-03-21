#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "util.h"

//StudioMDL constants
#define MAX_NUM_LODS 8
#define MAX_NUM_BONES_PER_VERT 3

namespace VVD
{
#pragma pack(push, 1)

struct boneWeight
{
	float weight[MAX_NUM_BONES_PER_VERT];
	char bone[MAX_NUM_BONES_PER_VERT];
	char numbones;
};

struct Vertex
{
	boneWeight m_boneweights;
	glm::vec3 m_vecPosition;
	glm::vec3 m_vecNormal;
	glm::vec2 m_vecTexCoord;
};

struct Header
{
	int id;
	int version;
	long checksum;
	int numLods;
	int numLodVertexes[MAX_NUM_LODS];
	int numFixups;
	int fixupTableStart;
	int vertexDataStart;
	int tangentDataStart;
};

#pragma pack(pop)
}

class vvd_data : public util::verboseControl
{
private:
public:
	VVD::Header header;

	std::vector<VVD::Vertex> verticesLOD0;

	vvd_data(std::string filepath, bool verbose = false)
	{
		this->use_verbose = verbose;

		//Create file handle
		std::ifstream reader(filepath, std::ios::in | std::ios::binary);

		if (!reader) {
			throw std::exception("VVD::OPEN FAILED"); return;
		}

		reader.read((char*)&this->header, sizeof(this->header));
		this->debug("VVD Version:", this->header.version);

		//Read vertex data
		reader.seekg(header.vertexDataStart);

		//Read LOD0
		for (int i = 0; i < header.numLodVertexes[0]; i++)
		{
			VVD::Vertex vert;
			reader.read((char*)&vert, sizeof(vert));

			this->verticesLOD0.push_back(vert);
		}

		reader.close();
	}

	vvd_data(std::ifstream* stream, unsigned int offset, bool verbost = false) {
		this->use_verbose = verbost;

		stream->seekg(offset);
		stream->read((char*)&this->header, sizeof(this->header));
		this->debug("VVD Version:", this->header.version);

		//Read vertex data
		stream->seekg(offset + header.vertexDataStart);

		//Read LOD0
		for (int i = 0; i < header.numLodVertexes[0]; i++)
		{
			VVD::Vertex vert;
			stream->read((char*)&vert, sizeof(vert));

			// Do the sorce->opengl flipperoo
			glm::vec3 temp = vert.m_vecPosition;
			vert.m_vecPosition = glm::vec3(-temp.x, temp.z, temp.y);

			this->verticesLOD0.push_back(vert);
		}

		this->debug("Data length: ", this->verticesLOD0.size());
	}
	~vvd_data() {};
};