#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "util.h"
#include "vector.h"

namespace mdl
{
#pragma pack(push, 1)

	struct header
	{
		int					id;
		int					version;

		int					checksum;		// this has to be the same in the phy and vtx files to load!

		char				name[64];
		int					length;

		vec3				eyeposition;	// ideal eye position

		vec3				illumposition;	// illumination center

		vec3				hull_min;		// ideal movement hull size
		vec3				hull_max;

		vec3				view_bbmin;		// clipping bounding box
		vec3				view_bbmax;

		int					flags;

		int					numbones;			// bones
		int					boneindex;

		int					numbonecontrollers;		// bone controllers
		int					bonecontrollerindex;

		int					numhitboxsets;
		int					hitboxsetindex;

		// file local animations? and sequences
		//private:
		int					numlocalanim;			// animations/poses
		int					localanimindex;		// animation descriptions

		int					numlocalseq;				// sequences
		int					localseqindex;

		// raw textures
		int					numtextures;
		int					textureindex;

		// raw textures search paths
		int					numcdtextures;
		int					cdtextureindex;

		// replaceable textures tables
		int					numskinref;
		int					numskinfamilies;
		int					skinindex;

		int					numbodyparts;
		int					bodypartindex;

		// queryable attachable points
		//private:
		int					numlocalattachments;
		int					localattachmentindex;

		// animation node to animation node transition graph
		//private:
		int					numlocalnodes;
		int					localnodeindex;
		int					localnodenameindex;

		int					numflexdesc;
		int					flexdescindex;

		int					numflexcontrollers;
		int					flexcontrollerindex;

		int					numflexrules;
		int					flexruleindex;

		int					numikchains;
		int					ikchainindex;

		int					nummouths;
		int					mouthindex;

		//private:
		int					numlocalposeparameters;
		int					localposeparamindex;

		int					surfacepropindex;

		// Key values
		int					keyvalueindex;
		int					keyvaluesize;

		int					numlocalikautoplaylocks;
		int					localikautoplaylockindex;

		// The collision model mass that jay wanted
		float				mass;
		int					contents;

		// external animations, models, etc.
		int					numincludemodels;
		int					includemodelindex;

		// for demand loaded animation blocks
		int					szanimblocknameindex;
		int					numanimblocks;
		int					animblockindex;

		int					bonetablebynameindex;
		char				constdirectionallightdot;
		char				rootLOD;
		char				numAllowedRootLODs;
		char				unused[1];
		int					unused4; // zero out if version < 47
		int					numflexcontrollerui;
		int					flexcontrolleruiindex;
		float				flVertAnimFixedPointScale;
		int					unused3[1];
		int					studiohdr2index;
		int					unused2[1];
	};

	// skin info
	struct textureHeader
	{
		int		name_offset; 	// Offset for null-terminated string
		int		flags;
		int		used; 		// ??

		int		unused; 	// ??

		int	material;		// Placeholder for IMaterial
		int	client_material;	// Placeholder for void*

		int		unused2[10];
	};

#pragma pack(pop)
}

class mdl_model : public verboseControl
{
public:
	mdl::header header;

	mdl_model(std::string mdl, bool verbose)
	{
		this->use_verbose = verbose;
		std::ifstream reader(mdl, std::ios::in | std::ios::binary);

		if (!reader) {
			throw std::exception("MDL::LOAD FAILED"); return;
		}


		reader.read((char*)&this->header, sizeof(this->header));
		this->debug("Version", this->header.version);

		//Read texture data
		reader.seekg(this->header.cdtextureindex);

		mdl::textureHeader test;
		reader.read((char*)&test, sizeof(test));

		reader.seekg(test.name_offset - sizeof(mdl::textureHeader), std::ios::cur);

		std::string name = "";
		while (true)
		{
			char c;
			reader.read(&c, 1);

			if (c == (char)0)
				break;

			name += c;
		}

		this->debug(name);


		reader.close();
	}

	virtual ~mdl_model() {}
};