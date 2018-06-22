#pragma once
#include <vector>
#include <fstream>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "generic.hpp"

namespace bsp
{
#pragma pack(push, 1)
struct plane {
	glm::vec3 normal;
	float dist;
	int type;
};

struct vertex {
	glm::vec3 position;
};

struct edge {
	unsigned short vertex[2];
};

struct face {
	unsigned short planeNum; //Plane number
	char side; //Faces opposite to the node's plane direction
	char onNode; //1 if on node, 0 if in leaf

	int firstEdge; //Index into surfedges
	short numEdges; //Number of sufedges

	short texInfo; //Texture info
	short dispInfo; //Displacement info
	short surfaceFogVolumeID;
	char styles[4];
	int lightOffset; //Offset into lightmap lump
	float area; //Face area
	int lightmapTextureMinsInLuxels[2];
	int lightmapTextureMaxInLuxels[2];
	int origFace; //Face this was split from
	unsigned short numPrims; //Primitives
	unsigned short firstPrimID;
	unsigned int smoothingGroups; //Lightmap smoothing group
};

struct vertexSmooth {
	int smoothingGroup;
	std::vector<glm::vec3> influences;

	vertexSmooth()
		:smoothingGroup(-1) {}

	glm::vec3 getNormal() {
		glm::vec3 norm;
		for (int i = 0; i < influences.size(); i++) {
			norm = norm + influences[i];
		}
		norm = norm * (1.0f / influences.size());
		return norm;
	}
};

struct vertex_fixed {
	glm::vec3 position;
	glm::vec2 UV;
	glm::vec3 normal_hard;
	vertexSmooth* normalSource;
};

struct face_fixed {
	std::vector<vertex_fixed> vertices;
};

struct dispSubNeighbor {
	unsigned short neighborIndex;
	char neighborOrientation;
	char span;
	char neightborSpan;
	char pad1;
};

struct dispNeighbor {
	dispSubNeighbor n1;
	dispSubNeighbor n2;
};

struct dispCornerNeighbor {
	unsigned short neighbors[4];
	char nNeighbors;
	char pad1;
};

struct dispInfo {
	glm::vec3 startPosition;
	int dispVertStart;
	int dispTriStart;
	int power;
	int minTess;
	float smoothingAngle;
	int contents;
	unsigned short mapFace;

	char pad1[2];

	int lightmapAlphaStart;
	int lightmapSamplePositionStart;
	dispNeighbor edgeNeighbors[4];
	dispCornerNeighbor cornerNeighbors[4];
	unsigned int allowVerts[10];
};

struct dispVert {
	glm::vec3 vec;
	float dist;
	float alpha;
};

struct face_displacement {
	vertex_fixed* vertices;
	int num_vertices;

	short* tris;
	int num_tris;

	int power;

	face_displacement(int pwr) {
		if (pwr > 4) { pwr = 4; }
		if (pwr < 1) { pwr = 1; }

		power = pwr = (1 << pwr);
		int numVertices = pwr + 1;
		numVertices *= numVertices;
		vertices = new vertex_fixed[numVertices];
		num_vertices = numVertices;

		int numFaces = pwr;
		numFaces *= numFaces;
		tris = new short[numFaces * 6];
		num_tris = numFaces;																		//Possible *= 6 ?
		for (short i = 0; i < numFaces; ++i) {
			short off = (short)(i / pwr);
			tris[i * 6 + 0] = (short)(i + off);
			tris[i * 6 + 1] = (short)(i + off + pwr + 2);
			tris[i * 6 + 2] = (short)(i + off + 1);
			tris[i * 6 + 3] = (short)(i + off + pwr + 1);
			tris[i * 6 + 4] = (short)(i + off + pwr + 2);
			tris[i * 6 + 5] = (short)(i + off);
		}
	}
};

struct texinfo {
	float textureVecs[2][4];
	float lightmapVecs[2][4];
	int flags;
	int texdata;
};

struct texdata {
	glm::vec3 reflectivity;
	int nameStringTableID;
	int width;
	int height;
	int view_width;
	int view_height;
};

#pragma pack(pop)

//=========================================================================
// Definitions

std::vector<bsp::vertex> readVertices(std::ifstream* reader, bsp::lumpHeader info)
{
	return readLumpGeneric<bsp::vertex>(reader, info);
}

std::vector<bsp::edge> readEdges(std::ifstream* reader, bsp::lumpHeader info)
{
	return readLumpGeneric<bsp::edge>(reader, info);
}

std::vector<bsp::face> readFaces(std::ifstream* reader, bsp::lumpHeader info)
{
	return readLumpGeneric<bsp::face>(reader, info);
}

std::vector<bsp::plane> readPlanes(std::ifstream* reader, bsp::lumpHeader info)
{
	return readLumpGeneric<bsp::plane>(reader, info);
}

std::vector<bsp::texinfo> readTexInfos(std::ifstream* reader, bsp::lumpHeader info)
{
	return readLumpGeneric<bsp::texinfo>(reader, info);
}

std::vector<bsp::texdata> readTexDatas(std::ifstream* reader, bsp::lumpHeader info)
{
	return readLumpGeneric<bsp::texdata>(reader, info);
}
}