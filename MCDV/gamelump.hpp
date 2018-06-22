#pragma once
#include "generic.hpp"

#include <vector>
#include <fstream>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

namespace bsp {
#pragma pack(push, 1)

	struct dgamelump_header {
		int lumpCount;
	};

	struct dgamelump {
		int id;
		unsigned short flags;
		unsigned short version;
		int offset;
		int length;
	};

#pragma pack(pop)

	//Don't read as (char*)

	struct staticprop {
		int version; //Version of prop
		std::string mdlName; //Name of prop

		//V4+
		glm::vec3 Origin;
		glm::vec3 angle;
		unsigned short PropType; //Index into mdl name dict
		unsigned short FirstLeaf; //Leaf index
		unsigned short LeafCount;
		unsigned char solid;
		unsigned char flags;
		int skin; //Skin number
		float fademindist;
		float fademaxdist;
		glm::vec3 lightingorigin;

		//V5+
		float forcedFadeScale;

		//V6 & V7
		unsigned short MinDXLevel;
		unsigned short MaxDXLevel;

		//V8+
		unsigned char MinCPULevel;
		unsigned char MaxCPULevel;
		unsigned char MinGPULevel;
		unsigned char MaxGPULevel;

		//V7+
		unsigned char diffuseModulation[4]; // Color and alpha modulation

											//V10+
		float unkown;

		//V9+
		int DisableDX360; //Actually a boolean, but reads incorrectly that way

		//V11+
		float uniformscale;
	};

	std::vector<dgamelump> readGameLumps(std::ifstream* reader, bsp::lumpHeader info) {


		dgamelump_header dgHeader;

		reader->seekg(info.lumpOffset);
		reader->read((char*)&dgHeader, sizeof(dgHeader));

		std::vector<dgamelump> lumps;
		for (int i = 0; i < dgHeader.lumpCount; i++) {
			dgamelump lump;
			reader->read((char*)&lump, sizeof(lump));
			lumps.push_back(lump);
		}

		return lumps;
	}
}