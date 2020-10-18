//Make sure everything is nice and together
#pragma pack(push, 1)

typedef struct
{
	// these index into the mesh's vert[origMeshVertID]'s bones
	unsigned char boneWeightIndex[3];
	unsigned char numBones;

	unsigned short origMeshVertID;

	// for sw skinned verts, these are indices into the global list of bones
	// for hw skinned verts, these are hardware bone indices
	char boneID[3];
} VTXVertex_t;

enum StripGroupFlags
{
	STRIPGROUP_IS_FLEXED = 0x01,
	STRIPGROUP_IS_HWSKINNED = 0x02,
	STRIPGROUP_IS_DELTA_FLEXED = 0x04,
	STRIPGROUP_SUPPRESS_HW_MORPH = 0x08,	// NOTE: This is a temporary flag used at run time.
};

enum StripHeaderFlags_t {
	STRIP_IS_TRILIST	= 0x01,
	STRIP_IS_TRISTRIP	= 0x02
};

// A strip is a piece of a stripgroup which is divided by bones 
typedef struct 
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
} VTXStripHeader_t;
// Bone state change inline code ommited

// a locking group
// a single vertex buffer
// a single index buffer
typedef struct
{
	// These are the arrays of all verts and indices for this mesh.  strips index into this.
	int numVerts;
	int vertOffset;

	int numIndices;
	int indexOffset;

	int numStrips;
	int stripOffset;

	unsigned char flags;
} VTXStripGroupHeader_t;
VTXVertex_t *pVertexVTX( VTXStripGroupHeader_t *t, int i ) { 
	return (VTXVertex_t *)(((char *)t) + t->vertOffset) + i;
}
unsigned short *pIndexVTX( VTXStripGroupHeader_t *t, int i ) { 
	return (unsigned short *)(((char *)t) + t->indexOffset) + i;
}
VTXStripHeader_t *pStripVTX( VTXStripGroupHeader_t *t, int i ) { 
	return (VTXStripHeader_t *)(((char *)t) + t->stripOffset) + i; 
}

typedef struct
{
	int numStripGroups;
	int stripGroupHeaderOffset;

	unsigned char flags;
} VTXMeshHeader_t;
VTXStripGroupHeader_t *pStripGroupVTX( VTXMeshHeader_t *t, int i ) { 
	return (VTXStripGroupHeader_t *)(((char *)t) + t->stripGroupHeaderOffset) + i; 
}

typedef struct
{
	//Mesh array
	int numMeshes;
	int meshOffset;

	float switchPoint;
} VTXModelLODHeader_t;
VTXMeshHeader_t *pMeshVTX( VTXModelLODHeader_t *t, int i ) { 
	return (VTXMeshHeader_t *)(((char *)t) + t->meshOffset) + i; 
}

// This maps one to one with models in the mdl file.
typedef struct
{
	//LOD mesh array
	int numLODs;   //This is also specified in FileHeader_t
	int lodOffset;
} VTXModelHeader_t;
VTXModelLODHeader_t *pLODVTX( VTXModelHeader_t *t, int i ) { 
	return (VTXModelLODHeader_t *)(((char *)t) + t->lodOffset) + i; 
}

typedef struct
{
	//Model array
	int numModels;
	int modelOffset;
} VTXBodyPartHeader_t;
VTXModelHeader_t *pModelVTX( VTXBodyPartHeader_t *t, int i ) { 
	return (VTXModelHeader_t *)(((char *)t) + t->modelOffset) + i;
}

typedef struct
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
} VTXFileHeader_t;
VTXBodyPartHeader_t *pBodyPartVTX( VTXFileHeader_t *t, int i ) {
	return (VTXBodyPartHeader_t *)(((char *)t) + t->bodyPartOffset) + i;
}

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

void iter_vtx( VTXFileHeader_t *t, void( *onStrip )( VTXStripHeader_t*, void* ), void *usr )
{
	for ( int bodyID = 0; bodyID < t->numBodyParts; ++bodyID )
	{
		VTXBodyPartHeader_t* pVtxBodyPart = pBodyPartVTX( t, bodyID );
		for ( int modelID = 0; modelID < pVtxBodyPart->numModels; ++modelID )
		{
			VTXModelHeader_t* pVtxModel = pModelVTX( pVtxBodyPart, modelID );

			int nLod = 0;
			VTXModelLODHeader_t *pVtxLOD = pLODVTX( pVtxModel, nLod );

			for ( int nMesh = 0; nMesh < pVtxLOD->numMeshes; ++nMesh )
			{
				VTXMeshHeader_t* pVtxMesh = pMeshVTX( pVtxLOD, nMesh );

				for ( int nGroup = 0; nGroup < pVtxMesh->numStripGroups; ++nGroup )
				{
					VTXStripGroupHeader_t* pStripGroup = pStripGroupVTX( pVtxMesh, nGroup );

					for ( int nStrip = 0; nStrip < pStripGroup->numStrips; nStrip++ )
					{
						VTXStripHeader_t *pStrip = pStripVTX( pStripGroup, nStrip );

						if ( pStrip->flags & STRIP_IS_TRILIST )
						{
							onStrip( pStrip, usr );
						}
					}
				}
			}
		}
	}
}
