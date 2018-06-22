#pragma once
#include <vector>
#include <iostream>
#include <fstream>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "vbsp.hpp"


namespace tbsp
{
	struct vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uvCoord;
	};

	struct plane {
		glm::vec3 normal; //Normal direction of the plane
		float dist; //Distance from 0,0,0 along normal
	};

	struct node {
		int planeNum; //Indexes into the plane array

		unsigned short firstFace; //Indexes into the indices array
		unsigned short numFaces; //Amount of indices to use *3
	};

	//Leaf currently unused
	struct leaf {
		int blank; 

		leaf() :
			blank(0x00) {}
	};

	struct header {
		int magicnum; //0xBEEFBEEF
		int version; //V1

		//Arrays
		int indicescount;
		int indicesoffset;

		int vertexcount;
		int vertexoffset;

		int planecount;
		int planeoffset;

		int nodecount;
		int nodeoffset;

		int leafcount; //Leaf should contain drawing information
		int leafoffset;

		header() :
			magicnum(0xBEEFBEEF), version(1) {}
	};
}

/*


1: Extract all face data into a vertex array
   - Create face table (indices block)

*/

void convertToTBSP(vbsp_level source, std::string filepath)
{
	//Testing
	std::ofstream of;
	of.open("test.obj");

	

	std::fstream writer(filepath, std::ios::out | std::ios::binary);

	//Write blank header
	tbsp::header header;
	writer.write((char*)&header, sizeof(header));

	for (int c_face_i = 0; c_face_i < source.faces.size(); c_face_i++)
	{
		bsp::face face = source.faces[c_face_i];
	}

	of.close();
	writer.close();

}