#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "util.h"

namespace vtx
{
	//Make sure everything is nice and together
#pragma pack(push, 1)

	struct Vertex
	{
		// these index into the mesh's vert[origMeshVertID]'s bones
		unsigned char boneWeightIndex[3];
		unsigned char numBones;

		unsigned short origMeshVertID;

		// for sw skinned verts, these are indices into the global list of bones
		// for hw skinned verts, these are hardware bone indices
		char boneID[3];
	};

	enum StripGroupFlags
	{
		STRIPGROUP_IS_FLEXED = 0x01,
		STRIPGROUP_IS_HWSKINNED = 0x02,
		STRIPGROUP_IS_DELTA_FLEXED = 0x04,
		STRIPGROUP_SUPPRESS_HW_MORPH = 0x08,	// NOTE: This is a temporary flag used at run time.
	};

	// A strip is a piece of a stripgroup which is divided by bones 
	struct StripHeader
	{
		//Indices array
		int numIndices;
		int indexOffset;

		//Vertices array
		int numVerts;
		int vertOffset;

		short numBones;

		unsigned char flags;

		int numBoneStateChanges;
		int boneStateChangeOffset;
	};

	// a locking group
	// a single vertex buffer
	// a single index buffer
	struct StripGroupHeader
	{
		// These are the arrays of all verts and indices for this mesh.  strips index into this.
		int numVerts;
		int vertOffset;

		int numIndices;
		int indexOffset;

		int numStrips;
		int stripOffset;

		unsigned char flags;
	};

	struct MeshHeader
	{
		int numStripGroups;
		int stripGroupHeaderOffset;

		unsigned char flags;
	};

	struct ModelLODHeader
	{
		//Mesh array
		int numMeshes;
		int meshOffset;

		float switchPoint;
	};

	// This maps one to one with models in the mdl file.
	struct ModelHeader
	{
		//LOD mesh array
		int numLODs;   //This is also specified in FileHeader_t
		int lodOffset;
	};

	struct BodyPartHeader
	{
		//Model array
		int numModels;
		int modelOffset;
	};

	struct FileHeader
	{
		// file version as defined by OPTIMIZED_MODEL_FILE_VERSION (currently 7)
		int version;

		// hardware params that affect how the model is to be optimized.
		int vertCacheSize;
		unsigned short maxBonesPerStrip;
		unsigned short maxBonesPerTri;
		int maxBonesPerVert;

		// must match checkSum in the .mdl
		int checkSum;

		int numLODs; // Also specified in ModelHeader_t's and should match

						// Offset to materialReplacementList Array. one of these for each LOD, 8 in total
		int materialReplacementListOffset;

		//Defines the size and location of the body part array
		int numBodyParts;
		int bodyPartOffset;
	};

	/*
		             .VTX file structure
		=============================================

		FileHeader
		  L	BodyParts::
			  L	Models::
				  L	LODS::
					  L	Meshes::
						  L	StripGroups::
							  L	VerticesTable[StudioMDL.Vertex]
							  L	IndicesTable[UINT16]
							  |
							  L	Strips::
								  L	Vertices[UINT16]
								  L	Indices[UINT16]
	*/

#pragma pack(pop)
}

class vtx_mesh : public util::verboseControl
{
public:
	std::vector<unsigned short> vertexSequence;
	vtx::FileHeader header;
	bool read_success = true;

	vtx_mesh(std::ifstream* stream, bool verbost = false) {
		this->use_verbose = verbost;

		unsigned int offset = stream->tellg();

		//Read header
		stream->read((char*)&this->header, sizeof(this->header));
		this->debug("VTX version:", this->header.version);
		this->debug("Num LODS:", this->header.numLODs);

		/* Read bulk of .VTX file */

		/* Body part array */
		stream->seekg(offset + header.bodyPartOffset);
		int abs_body_base_offset = stream->tellg();

		for (int body = 0; body < header.numBodyParts; body++) {
			//Move to current body part array item
			stream->seekg(abs_body_base_offset);
			stream->seekg(body * sizeof(vtx::BodyPartHeader), std::ios::cur);

			//Read the body part
			vtx::BodyPartHeader BODY;
			stream->read((char*)&BODY, sizeof(BODY));

			/* Model array */
			stream->seekg(BODY.modelOffset - sizeof(vtx::BodyPartHeader), std::ios::cur);
			int abs_model_base_offset = stream->tellg();

			int total_verts = 0;
			//NOTE: Total verts may need to be initialized outside the body array

			for (int model = 0; model < BODY.numModels; model++) {
				//Move to current model array item
				stream->seekg(abs_model_base_offset);
				stream->seekg(model * sizeof(vtx::ModelHeader), std::ios::cur);

				//Read the Model
				vtx::ModelHeader MODEL;
				stream->read((char*)&MODEL, sizeof(MODEL));

				/* LOD array */
				stream->seekg(MODEL.lodOffset - sizeof(vtx::ModelHeader), std::ios::cur);
				int abs_lod_base_offset = stream->tellg();

				for (int lod = 0; lod < MODEL.numLODs; lod++) {
					if (lod > 0) goto IL_EXIT; // Skip all the other lods for now

											   //Move to the current LOD header array item
					stream->seekg(abs_lod_base_offset);
					stream->seekg(lod * sizeof(vtx::ModelLODHeader), std::ios::cur);

					//Read the LOD header
					vtx::ModelLODHeader LOD;
					stream->read((char*)&LOD, sizeof(LOD));

					/* Mesh array */
					stream->seekg(LOD.meshOffset - sizeof(vtx::ModelLODHeader), std::ios::cur);
					int abs_mesh_base_offset = stream->tellg();

					for (int mesh = 0; mesh < LOD.numMeshes; mesh++) {
						//Move to the current mesh array item
						stream->seekg(abs_mesh_base_offset);
						stream->seekg(mesh * sizeof(vtx::MeshHeader), std::ios::cur);

						//Read the Mesh header
						vtx::MeshHeader MESH;
						stream->read((char*)&MESH, sizeof(MESH));

						/* Strip Group array */
						stream->seekg(MESH.stripGroupHeaderOffset - sizeof(vtx::MeshHeader), std::ios::cur);
						int abs_strip_group_base_offset = stream->tellg();

						for (int sgroup = 0; sgroup < MESH.numStripGroups; sgroup++) {
							//Move to the current stripgroup array item
							stream->seekg(abs_strip_group_base_offset);
							stream->seekg(sgroup * sizeof(vtx::StripGroupHeader), std::ios::cur);

							//Read the strip group header
							vtx::StripGroupHeader SGROUP;
							stream->read((char*)&SGROUP, sizeof(SGROUP));

							int base_location = (int)stream->tellg() - sizeof(vtx::StripGroupHeader);
							int location_vertex_array = base_location + SGROUP.vertOffset;
							int location_indices_array = base_location + SGROUP.indexOffset;

							//Read vertex table
							std::vector<vtx::Vertex> vertexTable;
							stream->seekg(location_vertex_array);
							for (int i = 0; i < SGROUP.numVerts; i++)
							{
								vtx::Vertex vert;
								stream->read((char*)&vert, sizeof(vert));
								vertexTable.push_back(vert);
							}

							//Read indices set
							std::vector<unsigned short> indicesTable;
							stream->seekg(location_indices_array);
							for (int i = 0; i < SGROUP.numIndices; i++)
							{
								unsigned short index;
								stream->read((char*)&index, sizeof(index));
								indicesTable.push_back(index);
							}

							/* Strips array */
							stream->seekg(base_location);
							stream->seekg(SGROUP.stripOffset, std::ios::cur);
							int abs_strip_base_offset = stream->tellg();

							for (int strip = 0; strip < SGROUP.numStrips; strip++)
							{
								//Move to current strip array item
								stream->seekg(abs_strip_base_offset);
								stream->seekg(strip * sizeof(vtx::StripHeader), std::ios::cur);

								//Read the strip
								vtx::StripHeader STRIP;
								stream->read((char*)&STRIP, sizeof(STRIP));

								//Virtual vertices pool
								std::vector<vtx::Vertex> v_verts;
								for (int i = 0; i < STRIP.numVerts; i++)
									if ((STRIP.vertOffset + i) >= vertexTable.size())
										throw std::exception("VTX::DECOMPILE::VERT_TABLE OUT OF RANGE");
									else
										v_verts.push_back(vertexTable[STRIP.vertOffset + i]);

								//Virtual indices pool
								std::vector<unsigned short> v_indices;
								for (int i = 0; i < STRIP.numIndices; i++)
									if ((STRIP.indexOffset + i) >= indicesTable.size())
										throw std::exception("VTX::DECOMPILE::INDEX_TABLE OUT OF RANGE");
									else
										v_indices.push_back(indicesTable[STRIP.indexOffset + i]);

								for (int i = 0; i < v_indices.size(); i++)
								{
									this->vertexSequence.push_back(v_verts[v_indices[i]].origMeshVertID + total_verts);
								}
							}

							total_verts += SGROUP.numVerts;
						}
					}
				}
			}
		}
	IL_EXIT: stream->close();
	}
};