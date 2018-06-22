#pragma once
#include <string>
#include <fstream>
#include <iostream>

#include "util.h"
#include "interpolation.h"

#include "generic.hpp"
#include "lumps_geometry.hpp"
#include "lumps_visibility.hpp"
#include "gamelump.hpp"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

namespace bsp {
#pragma pack(push, 1)

struct header {
	unsigned int magicNum;
	int version;
	bsp::lumpHeader lumps[64];
	int mapRevision;
};

#pragma pack(pop)
}

//Transfer structs
class basic_mesh {
public:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<int> indices;
	std::vector<glm::vec2> uvs;

	void test_save_obj(std::string filepath)
	{
		std::ofstream outfile;
		outfile.open(filepath);

		outfile << "# test object from vbsp parser" << std::endl;
		outfile << "# Written by Sven Monhof, adapted for c++ by Harry Godden" << std::endl;

		for (int i = 0; i < vertices.size(); i++) {
			glm::vec3 vert = vertices[i];
			outfile << "v " << vert.x << " " << vert.y << " " << vert.z << std::endl;
		}

		for (int i = 0; i < normals.size(); i++) {
			glm::vec3 normal = normals[i];
			outfile << "vn " << normal.x << " " << normal.y << " " << normal.z << std::endl;
		}

		for (int i = 0; i < uvs.size(); i++) {
			glm::vec2 uv = uvs[i];
			outfile << "vt " << uv.x << " " << uv.y << std::endl;
		}

		for (int i = 0; i < indices.size() / 3; i++) {
			int i1 = indices[i * 3 + 0] + 1;
			int i2 = indices[i * 3 + 1] + 1;
			int i3 = indices[i * 3 + 2] + 1;

			outfile << "f " << i1 << "/" << i1 << "/" << i1 << " ";
			outfile << i2 << "/" << i2 << "/" << i2 << " ";
			outfile << i3 << "/" << i3 << "/" << i3 << std::endl;
		}

		outfile.close();
	}
};

class vbsp_level : public util::verboseControl
{
private:
	bsp::header header;

public:
	// Geo
	std::vector<bsp::plane> planes;
	std::vector<bsp::vertex> vertices;
	std::vector<bsp::edge> edges;
	std::vector<int> surfEdges;
	std::vector<bsp::face> faces;

	std::vector<bsp::dispInfo> dispInfo;
	std::vector<bsp::dispVert> dispVert;

	// Tex info
	std::vector<bsp::texinfo> texinfos;
	std::vector<bsp::texdata> texdatas;
	std::vector<std::string> texDataString;

	// Vis
	std::vector<vis::node> vis_nodes;
	std::vector<vis::leaf> vis_leaves;
	std::vector<vis::model> vis_models;

	std::vector<unsigned short> vis_leaf_faces;
	std::vector<unsigned short> vis_leafbrushes;

	//Gamelump stuff
	std::vector<bsp::dgamelump> gameLumps;
	std::vector<std::string> mdlNamesDict;
	std::vector<bsp::staticprop> staticProps;

	vbsp_level(std::string path, bool verbose = false){
		this->use_verbose = verbose;

		//Create main file handle
		std::ifstream reader(path, std::ios::in | std::ios::binary);

		if (!reader) {
			throw std::exception("VBSP::LOAD Failed"); return;
		}

		//Read header
		reader.read((char*)&this->header, sizeof(this->header));
		this->debug("Reading VBSP, file version:", this->header.version);


		//==============================================================================
		// Read lumps
		this->debug("\n==== GEO LUMPS 1,3,12,13 ====\n");

		this->planes = bsp::readPlanes(&reader, this->header.lumps[1]); //Planes
		this->vertices = bsp::readVertices(&reader, this->header.lumps[3]); //Vertices
		this->edges = bsp::readEdges(&reader, this->header.lumps[12]); //Edges
		this->faces = bsp::readFaces(&reader, this->header.lumps[7]); //Faces
		this->surfEdges = bsp::readLumpGeneric<int>(&reader, this->header.lumps[13]); //Surf edges
		this->texinfos = bsp::readTexInfos(&reader, this->header.lumps[6]);
		this->texdatas = bsp::readTexDatas(&reader, this->header.lumps[2]);
		this->texDataString = this->readTexDataString(&reader, this->header.lumps[44], this->header.lumps[43]);

		//Displacement
		this->dispInfo = bsp::readLumpGeneric<bsp::dispInfo>(&reader, this->header.lumps[26]);
		this->dispVert = bsp::readLumpGeneric<bsp::dispVert>(&reader, this->header.lumps[33]);


		this->debug("Planes count:", this->planes.size());
		this->debug("Vertices count:", this->vertices.size());
		this->debug("Edges count:", this->edges.size());
		this->debug("Faces count:", this->faces.size());
		this->debug("SurfEdges count:", this->surfEdges.size());
		this->debug("Texinfo count:", this->texinfos.size());
		this->debug("Texdatas count:", this->texdatas.size());
		

		//==============================================================================
		// Vis lumps and BSP trees
		this->debug("\n==== VIS LUMPS 5,10,14 ====\n");

		this->vis_nodes = vis::readNodes(&reader, this->header.lumps[5]);
		this->vis_leaves = vis::readLeaves(&reader, this->header.lumps[10]);
		this->vis_models = vis::readModels(&reader, this->header.lumps[14]);
		this->vis_leaf_faces = bsp::readLumpGeneric<unsigned short>(&reader, this->header.lumps[16]);

		this->debug("Nodes:", this->vis_nodes.size());
		this->debug("Leaves:", this->vis_leaves.size());
		this->debug("Models:", this->vis_models.size());
		this->debug("Leaf faces:", this->vis_leaf_faces.size());

		//==============================================================================
		// Game Lumps
		this->debug("\n=== Game Lumps [35] ====\n");

		this->gameLumps = bsp::readGameLumps(&reader, this->header.lumps[35]);

		if (this->getGameLumpByID(0x73707270) != NULL) //sprp
			this->staticProps = this->readStaticProps(&reader, *this->getGameLumpByID(0x73707270));

		this->debug("Game lumps:", this->gameLumps.size());
		this->debug("[0x73707270|'sprp'] Static props:", this->staticProps.size());

		reader.close();
		std::cout << "Load complete" << std::endl;
	}

	vbsp_level() {}

	virtual ~vbsp_level() {}

	//Mesh tools

	//Get vertex index via surf edges
	int getVertIndex(int i)
	{
		if (this->surfEdges[i] < 0) 
			return this->edges[this->surfEdges[i] * -1].vertex[1];
		return this->edges[this->surfEdges[i]].vertex[0];
	}

	glm::vec2 getVertexUV(glm::vec3 pos, bsp::texinfo info)
	{
		glm::vec3 UVec = glm::vec3(info.textureVecs[0][0], info.textureVecs[0][1], info.textureVecs[0][2]);
		glm::vec3 VVec = glm::vec3(info.textureVecs[1][0], info.textureVecs[1][1], info.textureVecs[1][2]);

		float u = (glm::dot(UVec, pos) + info.textureVecs[0][3]) / this->texdatas[info.texdata].width;
		float v = (glm::dot(VVec, pos) + info.textureVecs[1][3]) / this->texdatas[info.texdata].height;

		return glm::vec2(u, v);
	}

	std::vector<bsp::staticprop> readStaticProps(std::ifstream* reader, bsp::dgamelump info)
	{
		reader->seekg(info.offset);

		int ver = info.version;

		//Read the dictionary size
		int dictEntries = 0;
		reader->read((char*)&dictEntries, sizeof(dictEntries));



		//Read dictionary
		for (int i = 0; i < dictEntries; i++) {
			char source[128];

			reader->read((char*)source, 128);

			std::string mdlName;
			for (int x = 0; x < 128; x++) {
				if (source[x] == (char)0)
					break;

				mdlName += source[x];
			}

			this->mdlNamesDict.push_back(mdlName);
		}

		//Leaf array (just skip this its junk to us)
		int leafEntries = 0;
		reader->read((char*)&leafEntries, sizeof(leafEntries));
		for (int i = 0; i < leafEntries; i++) {
			unsigned short leaf = 0;
			reader->read((char*)&leaf, sizeof(leaf));
		}


		//Read all the props and interpret based on the version
		int numProps = 0;
		reader->read((char*)&numProps, sizeof(numProps));

		std::vector<bsp::staticprop> props;

		for (int i = 0; i < numProps; i++) {

			bsp::staticprop prop;
			prop.version = ver;

			//Read in version four stuff always
			reader->read((char*)&prop.Origin, sizeof(prop.Origin));
			reader->read((char*)&prop.angle, sizeof(prop.angle));

			//Since 11+
			if (ver >= 11)
				reader->read((char*)&prop.uniformscale, sizeof(prop.uniformscale));

			reader->read((char*)&prop.PropType, sizeof(prop.PropType));
			reader->read((char*)&prop.FirstLeaf, sizeof(prop.FirstLeaf));
			reader->read((char*)&prop.LeafCount, sizeof(prop.LeafCount));
			reader->read((char*)&prop.solid, sizeof(prop.solid));
			reader->read((char*)&prop.flags, sizeof(prop.flags));
			reader->read((char*)&prop.skin, sizeof(prop.skin));
			reader->read((char*)&prop.fademindist, sizeof(prop.fademindist));
			reader->read((char*)&prop.fademaxdist, sizeof(prop.fademaxdist));
			reader->read((char*)&prop.lightingorigin, sizeof(prop.lightingorigin));

			//Since V5
			if (ver >= 5)
				reader->read((char*)&prop.forcedFadeScale, sizeof(prop.forcedFadeScale));

			//V6 & V7
			if (ver == 6 || ver == 7)
			{
				reader->read((char*)&prop.MinDXLevel, sizeof(prop.MinDXLevel));
				reader->read((char*)&prop.MaxDXLevel, sizeof(prop.MaxDXLevel));
			}

			//V8+
			if (ver >= 8)
			{
				reader->read((char*)&prop.MinCPULevel, sizeof(prop.MinCPULevel));
				reader->read((char*)&prop.MaxCPULevel, sizeof(prop.MaxCPULevel));
				reader->read((char*)&prop.MinGPULevel, sizeof(prop.MinGPULevel));
				reader->read((char*)&prop.MaxGPULevel, sizeof(prop.MaxGPULevel));
			}

			//V7+
			if (ver >= 7) {
				reader->read((char*)&prop.diffuseModulation, 4);
			}

			//V10+
			if (ver >= 10) {
				reader->read((char*)&prop.unkown, sizeof(prop.unkown));
			}

			//V9+
			if (ver >= 9) {
				reader->read((char*)&prop.DisableDX360, sizeof(prop.DisableDX360));
			}

			//Set to the string instead of using dictionary
			prop.mdlName = this->mdlNamesDict[prop.PropType];

			props.push_back(prop);
		}

		return props;
	}

	std::string readString(std::ifstream* reader, bsp::lumpHeader info) {
		reader->seekg(info.lumpOffset);
		char* str = new char[info.lumpLength];

		reader->read(str, info.lumpLength);
		return str;
	}

	std::vector<std::string> readTexDataString(std::ifstream* reader, bsp::lumpHeader info, bsp::lumpHeader info2) {
		int numIndexes = info.lumpLength / sizeof(int);
		reader->seekg(info.lumpOffset);

		std::vector<int> indexes = std::vector < int>();

		for (int i = 0; i < numIndexes; i++) {
			int index = 0;
			reader->read((char*)&index, sizeof(index));

			indexes.push_back(index);
		}

		char* texstr = new char[info2.lumpLength];
		
		reader->seekg(info2.lumpOffset);
		reader->read(texstr, info2.lumpLength);

		std::vector<std::string> strings = std::vector<std::string>();

		for (int i = 0; i < indexes.size(); i++) {
			int pos = indexes[i];

			std::string str = "";

			while (true) {
				char c = texstr[pos++];
				if (c == (char)0x0)
					break;
				str += c;
			}

			strings.push_back(str);
		}

		return strings;
	}

	bsp::dgamelump* getGameLumpByID(int id) {
		for (int i = 0; i < this->gameLumps.size(); i++) {
			bsp::dgamelump lump = this->gameLumps[i];

			if (lump.id == id) {
				return &lump;
			}
		}

		return NULL;
	}

	basic_mesh generate_mesh(int textureIndex) {
		std::vector<bsp::face_fixed> faces_fixed;
		std::vector<bsp::face_displacement> faces_disp;

		//Prepare faces
		for (int c_face_i = 0; c_face_i < faces.size(); c_face_i++)
		{
			bsp::face f = this->faces[c_face_i];

			if (this->texinfos[f.texInfo].texdata != -1 && (textureIndex == -1 || textureIndex == this->texinfos[f.texInfo].texdata)) {
				if (f.dispInfo == -1) {
					bsp::face_fixed ff;
					for (int i = 0; i < f.numEdges; ++i) {
						bsp::vertex_fixed vf;
						vf.position = vertices[this->getVertIndex(f.firstEdge + i)].position;
						vf.UV = getVertexUV(vf.position, this->texinfos[f.texInfo]);
						vf.normal_hard = this->planes[f.planeNum].normal;

						ff.vertices.push_back(vf);
					}

					faces_fixed.push_back(ff);
				}
				else if (f.numEdges == 4) {
					bsp::dispInfo info = this->dispInfo[f.dispInfo];
					bsp::face_displacement fd(info.power);
					glm::vec3 pos[4];
					pos[0] = vertices[this->getVertIndex(f.firstEdge + 0)].position;
					pos[1] = vertices[this->getVertIndex(f.firstEdge + 1)].position;
					pos[2] = vertices[this->getVertIndex(f.firstEdge + 2)].position;
					pos[3] = vertices[this->getVertIndex(f.firstEdge + 3)].position;
					int iterator = 0;
					while ((((pos[0].x - info.startPosition.x) > 0.25) ||
						((pos[0].y - info.startPosition.y) > 0.25) ||
						((pos[0].z - info.startPosition.z) > 0.25)) &&
						iterator < 4) {
						glm::vec3 temp = pos[0];
						pos[0] = pos[1];
						pos[1] = pos[2];
						pos[2] = pos[3];
						pos[3] = temp;
						++iterator;
					}

					for (int i = 0; i < fd.num_vertices; i++) {
						bsp::vertex_fixed vf;
						float x;
						float y;
						x = (float)((int)(i % (fd.power + 1)) / (float)(fd.power));
						y = 1 - (float)((int)(i / fd.power + 1)) / (float)(fd.power);
						glm::vec3 originalPosition = lerp(lerp(pos[1], pos[2], x), lerp(pos[0], pos[3], x), y);
						if (info.dispVertStart + i >= this->dispVert.size() || info.dispVertStart + i < 0) {
							vf.position = originalPosition;
						}
						else {
							vf.position = dispVert[info.dispVertStart + i].vec*dispVert[info.dispVertStart + i].dist + originalPosition;
						}

						vf.UV = getVertexUV(vf.position, this->texinfos[f.texInfo]);
						vf.normal_hard = planes[f.planeNum].normal;
						vf.normalSource = NULL;
						fd.vertices[i] = vf;
					}
					faces_disp.push_back(fd);
				}
			}
		}

		basic_mesh RV;

		std::vector<glm::vec3> verts;
		std::vector<glm::vec2> uvs;
		std::vector<int> inds;
		std::vector<glm::vec3> norms;

		for (int fi = 0; fi < faces_fixed.size(); fi++) {
			bsp::face_fixed f = faces_fixed[fi];

			int i = verts.size();
			if (i + f.vertices.size() >= USHRT_MAX) {
				std::cout << "WARNING::HIT_VERTEX_LIMIT" << std::endl;
				break;
			}
			for (int j = 0; j < f.vertices.size(); ++j) {
				verts.push_back(f.vertices[j].position);
				uvs.push_back(f.vertices[j].UV);

				norms.push_back(f.vertices[j].normal_hard);

				/*
				if (f.vertices[j].normalSource == NULL) {
					norms.push_back(f.vertices[j].normal_hard);
				}
				else {
					norms.push_back(f.vertices[j].normalSource->getNormal());
				}*/
			}
			for (int j = 0; j < f.vertices.size() - 2; ++j) {
				inds.push_back(i);
				inds.push_back(i + j + 1);
				inds.push_back(i + j + 2);
			}
		}

		for (int fd = 0; fd < faces_disp.size(); fd++) {
			bsp::face_displacement f = faces_disp[fd];

			int i = verts.size();
			if (i + vertices.size() >= USHRT_MAX) {
				std::cout << "WARNING::HIT_VERTEX_LIMIT" << std::endl;
				break;
			}
			for (int j = 0; j < f.num_vertices; ++j) {
				verts.push_back(f.vertices[j].position);
				uvs.push_back(f.vertices[j].UV);
				if (f.vertices[j].normalSource == NULL) {
					norms.push_back(f.vertices[j].normal_hard);
				}
				else {
					norms.push_back(f.vertices[j].normalSource->getNormal());
				}
			}
			for (int j = 0; j < f.num_tris; ++j) {
				inds.push_back(i + f.tris[j]);
			}
		}

		//Adjust position
		glm::vec3 vec;
		for (int i = 0; i < verts.size(); ++i) {
			vec = verts[i] * (2.5f / 100);
			vec.y *= -1;

			//Swizzle (Y/Z)
			float temp = vec.z;
			vec.z = vec.y;
			vec.y = temp;

			verts[i] = vec;
		}

		RV.vertices = verts;
		RV.indices = inds;
		RV.normals = norms;
		RV.uvs = uvs;

		return RV;
	}

	std::vector<float> generate_bigmesh() {
		std::vector<float> verts;
		for (int i = 0; i < this->faces.size(); i++) {
			bsp::face face = this->faces[i];

			std::vector<bsp::vertex> vertices;
			for (int e = face.firstEdge; e < face.firstEdge + face.numEdges; e++) {
				//edge_indexes.push_back);
				int index = this->surfEdges[e];
				if (index >= 0) //Trace forwards
				{
					vertices.push_back(this->vertices[this->edges[index].vertex[0]]);
					vertices.push_back(this->vertices[this->edges[index].vertex[1]]);
				}
				else
				{
					vertices.push_back(this->vertices[this->edges[std::abs(index)].vertex[1]]);
					vertices.push_back(this->vertices[this->edges[std::abs(index)].vertex[0]]);
				}
			}

			//Get face normal
			glm::vec3 normal = this->planes[face.planeNum].normal;
			if (face.side != 0)
				normal = -normal;

			normal = glm::normalize(normal);

			//Write to verts array
			for (int v = 1; v < vertices.size() -1; v++) {
				//Get verts positions
				bsp::vertex v0 = vertices[0];
				bsp::vertex v1 = vertices[v];
				bsp::vertex v2 = vertices[v + 1];

				//Write
				verts.push_back(v0.position.x* 0.01f);
				verts.push_back(v0.position.z* 0.01f);
				verts.push_back(v0.position.y* 0.01f);
				

				verts.push_back(normal.x);
				verts.push_back(normal.z);
				verts.push_back(normal.y);
				

				verts.push_back(v1.position.x* 0.01f);
				verts.push_back(v1.position.z* 0.01f);
				verts.push_back(v1.position.y* 0.01f);
				

				verts.push_back(normal.x);
				verts.push_back(normal.z);
				verts.push_back(normal.y);
				

				verts.push_back(v2.position.x* 0.01f);
				verts.push_back(v2.position.z* 0.01f);
				verts.push_back(v2.position.y* 0.01f);
				

				verts.push_back(normal.x);
				verts.push_back(normal.z);
				verts.push_back(normal.y);
				
			}
			
			if (i > 35000) {
				std::cout << "LIMIT HIT FOR NOW" << std::endl;
				break;
			}
		}

		return verts;
	}
};