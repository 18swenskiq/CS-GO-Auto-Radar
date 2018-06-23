#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

//Nav mesh reader
namespace Nav
{
	struct Place {
		unsigned int ID;
		std::string Name;
	};

	struct Area {
		unsigned int ID;
		glm::vec3 NW_Point;
		glm::vec3 SW_Point;
		glm::vec3 NE_Point;
		glm::vec3 SE_Point;

		unsigned int flags;
		float NE_Z;
		float SW_Z;
		float NW_Light;
		float NE_Light;
		float SW_Light;
		float SE_Light;
		float earliestOccupyA;
		float earliestOccupyB;
		unsigned short placeID;
	};

	class Mesh {
	public:
		std::vector<Area> areas;
		float latestOccupy = 0.0f;
		unsigned int majorVersion = 0;
		unsigned int minorVersion = 0;
		unsigned int BSPSize = 0;
		unsigned char meshAnal = 0x0;

		Mesh(std::string filename) {
			std::ifstream file(filename, std::ios::in | std::ios::binary);
			file.seekg(0);
			//Read magic number
			unsigned int magicnum = 0;
			file.read((char*)&magicnum, sizeof(magicnum));

			if (magicnum != 0xFEEDFACE)
				throw std::exception("Invalid nav mesh file");

			//Read version number
			file.read((char*)&this->majorVersion, sizeof(this->majorVersion));

			//if (this->majorVersion < 6 || this->majorVersion > 16)	throw std::exception("Major version out of bounds");

			std::cout << "Major version: " << majorVersion << std::endl;

			//Minor version (m10+)
			if (majorVersion >= 10) {
				file.read((char*)&this->minorVersion, sizeof(this->minorVersion));
				std::cout << "Minor version: " << minorVersion << std::endl;
			}


			//BSP size
			file.read((char*)&this->BSPSize, sizeof(this->BSPSize));
			std::cout << "BSP Size (b) " << this->BSPSize << std::endl;


			if (this->majorVersion >= 14){
				file.read((char*)&this->meshAnal, 1);
				std::cout << "mesh analysis: " << (int)this->meshAnal << std::endl;
			}


			file.seekg(17);

			//Getting place count on mesh
			unsigned short placecount = 0;
			file.read((char*)&placecount, 2);
			std::cout << "Places: " << placecount << std::endl;

			//read placenames
			for (int i = 0; i < placecount; i++) {
				unsigned short namelength = 0;
				file.read((char*)&namelength, sizeof(namelength));
				char* name = new char[namelength];

				file.read(name, namelength);

				std::cout << i << " : " << namelength << " : " << name << std::endl;

				delete[] name;
			}

			//Unnamed areas
			bool hasUnnamedAreas = false;
			if (majorVersion > 11) {
				unsigned char v = 0;
				file.read((char*)&v, sizeof(v));
				if (v > 0)
					hasUnnamedAreas = true;
			}

			std::cout << "Mesh has unnamed areas? " << (hasUnnamedAreas ? "True" : "False") << std::endl;

			//Navmesh data
			unsigned int areaCount = 0;
			file.read((char*)&areaCount, sizeof(areaCount));

			std::cout << "Areas: " << areaCount << std::endl;

			for (int i = 0; i < areaCount; i++) {
				Area thisarea;

				unsigned int areaID = 0;
				file.read((char*)&areaID, sizeof(areaID));

				if (majorVersion <= 8) {
					unsigned char flags = 0x0;
					file.read((char*)&flags, 1);
				}
				else if (majorVersion < 13) {
					unsigned short flags = 0x0;
					file.read((char*)&flags, 2);
				}
				else {
					unsigned int flags = 0x0;
					file.read((char*)&flags, 4);
				}

				//Read the NW position
				file.read((char*)&thisarea.NW_Point, sizeof(glm::vec3));
				file.read((char*)&thisarea.SE_Point, sizeof(glm::vec3));

				file.read((char*)&thisarea.NE_Z, sizeof(float));
				file.read((char*)&thisarea.SW_Z, sizeof(float));

				thisarea.NE_Point.x = thisarea.SE_Point.x;
				thisarea.NE_Point.y = thisarea.NW_Point.y;
				thisarea.NE_Point.z = thisarea.NE_Z;

				thisarea.SW_Point.x = thisarea.NW_Point.x;
				thisarea.SW_Point.y = thisarea.SE_Point.y;
				thisarea.SW_Point.z = thisarea.SW_Z;

				//Connections
				for (int c = 0; c < 4; c++) {
					unsigned int conCount;
					file.read((char*)&conCount, sizeof(conCount));

					for (int ci = 0; ci < conCount; ci++) {
						unsigned int targetAreaID;
						file.read((char*)&targetAreaID, sizeof(targetAreaID));
					}
				}

				//How many hiding spots are there in this area?
				unsigned char hidingSpotsCount;
				file.read((char*)&hidingSpotsCount, 1);

				//Loop each spot to get its flags and locations
				for (int hidingindex = 0; hidingindex < hidingSpotsCount; hidingindex++) {
					unsigned int hidingID = 0;
					file.read((char*)&hidingID, sizeof(hidingID));

					glm::vec3 location;
					file.read((char*)&location, sizeof(location));

					unsigned char hidingFlags;
					file.read((char*)&hidingFlags, 1);
				}

				if (majorVersion < 15) {
					unsigned char apprAreaCount;
					file.read((char*)&apprAreaCount, 1);

					//Skip junk
					int junksize = (4 * 3 + 2) * (int)apprAreaCount;
					char* junk = new char[junksize];
					file.read(junk, junksize);
					delete[] junk;
				}

				//Encounter paths
				unsigned int encounterpaths;
				file.read((char*)&encounterpaths, sizeof(encounterpaths));

				for (int path = 0; path < encounterpaths; path++) {
					unsigned int fromareaID;
					file.read((char*)&fromareaID, sizeof(fromareaID));
					unsigned char navDir;
					file.read((char*)&navDir, 1);
					unsigned int toareaID;
					file.read((char*)&toareaID, sizeof(toareaID));
					unsigned char navTargetDir;
					file.read((char*)&navTargetDir, 1);

					unsigned char spotcount;
					file.read((char*)&spotcount, 1);
					for (int spotindex = 0; spotindex < spotcount; spotindex++) {
						unsigned int orderID;
						file.read((char*)&orderID, sizeof(orderID));
						unsigned char distance;
						file.read((char*)&distance, 1);
					}
				}

				//Handle placenames
				unsigned short placeID;
				file.read((char*)&placeID, sizeof(placeID));

				//Ladder stuffs
				for (int dir = 0; dir < 2; dir++) {
					unsigned int ladderConnections;
					file.read((char*)&ladderConnections, sizeof(ladderConnections));
					for (int conn = 0; conn < ladderConnections; conn++) {
						unsigned int targetID;
						file.read((char*)&targetID, sizeof(targetID));
					}
				}

				file.read((char*)&thisarea.earliestOccupyA, sizeof(thisarea.earliestOccupyA));
				file.read((char*)&thisarea.earliestOccupyB, sizeof(thisarea.earliestOccupyB));

				if(thisarea.earliestOccupyA > latestOccupy)
					latestOccupy = thisarea.earliestOccupyA;

				if (thisarea.earliestOccupyB > latestOccupy)
					latestOccupy = thisarea.earliestOccupyB;

				//Lighting intensity on area
				if (majorVersion >= 11) {
					file.read((char*)&thisarea.NW_Light, sizeof(thisarea.NW_Light));
					file.read((char*)&thisarea.NE_Light, sizeof(thisarea.NE_Light));
					file.read((char*)&thisarea.SE_Light, sizeof(thisarea.SE_Light));
					file.read((char*)&thisarea.SW_Light, sizeof(thisarea.SW_Light));
				}

				//Visible areas
				if (majorVersion >= 16) {
					unsigned int visareaCount;
					file.read((char*)&visareaCount, sizeof(visareaCount));

					for (int visarea = 0; visarea < visareaCount; visarea++) {
						unsigned int visibileArea;
						file.read((char*)&visibileArea, sizeof(visibileArea));

						unsigned char attr;
						file.read((char*)&attr, 1);
					}
				}

				unsigned int inheritVisibility;
				file.read((char*)&inheritVisibility, sizeof(inheritVisibility));

				//Unkown
				unsigned char unknown;
				file.read((char*)&unknown, 1);

				char* bytes = new char[(int)unknown * 14];
				file.read(bytes, (int)unknown * 14);
				delete[] bytes;

				areas.push_back(thisarea);
			}

			file.close();
		}

		std::vector<float> generateGLMesh() {
			std::vector<float> m;

			for (int i = 0; i < this->areas.size(); i++) {
				Area area = this->areas[i];

				m.push_back(this->areas[i].NW_Point.x* 0.01f);
				m.push_back(this->areas[i].NW_Point.z* 0.01f);
				m.push_back(-this->areas[i].NW_Point.y* 0.01f);

				m.push_back(0); m.push_back(0); m.push_back(1);

				m.push_back(this->areas[i].NE_Point.x* 0.01f);
				m.push_back(this->areas[i].NE_Point.z* 0.01f);
				m.push_back(-this->areas[i].NE_Point.y* 0.01f);

				m.push_back(0); m.push_back(0); m.push_back(1);

				m.push_back(this->areas[i].SE_Point.x* 0.01f);
				m.push_back(this->areas[i].SE_Point.z* 0.01f);
				m.push_back(-this->areas[i].SE_Point.y* 0.01f);

				m.push_back(0); m.push_back(0); m.push_back(1);

				m.push_back(this->areas[i].NW_Point.x* 0.01f);
				m.push_back(this->areas[i].NW_Point.z* 0.01f);
				m.push_back(-this->areas[i].NW_Point.y* 0.01f);

				m.push_back(0); m.push_back(0); m.push_back(1);

				m.push_back(this->areas[i].SE_Point.x* 0.01f);
				m.push_back(this->areas[i].SE_Point.z* 0.01f);
				m.push_back(-this->areas[i].SE_Point.y* 0.01f);

				m.push_back(0); m.push_back(0); m.push_back(1);

				m.push_back(this->areas[i].SW_Point.x* 0.01f);
				m.push_back(this->areas[i].SW_Point.z* 0.01f);
				m.push_back(-this->areas[i].SW_Point.y* 0.01f);

				m.push_back(0); m.push_back(0); m.push_back(1);
			}

			std::cout << m.size() << std::endl;
			return m;
		}
	};
}