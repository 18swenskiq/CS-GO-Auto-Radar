#include "valve/vvd.h"
#include "valve/vtx.h"

#pragma pack(push, 1)

typedef struct
{
	// base of external vertex data stores
	void			*pVertexData;
	void			*pTangentData;
} mstudio_modelvertexdata_t;

typedef struct
{
	// indirection to this mesh's model's vertex data
	int					unused_modelvertexdata; // 64b - Moved to follow num_LOD_Vertexes. 

	// used for fixup calcs when culling top level lods
	// expected number of mesh verts at desired lod
	int					numLODVertexes[MAX_NUM_LODS];
	
	mstudio_modelvertexdata_t *_the_death_ptr;
} mstudio_meshvertexdata_t;

typedef struct mstudiomodel_t mstudiomodel_t;

typedef struct
{
	int					material;
	int					modelindex; 
	int					numvertices;		// number of unique vertices/normals/texcoords
	int					vertexoffset;		// vertex mstudiovertex_t
	int					numflexes;			// vertex animation
	int					flexindex;
	// special codes for material operations
	int					materialtype;
	int					materialparam;
	// a unique ordinal for this mesh
	int					meshid;
	float					center[3];
	mstudio_meshvertexdata_t vertexdata;

	int					unused[6]; // remove as appropriate
} mstudiomesh_t;

// studio models
struct mstudiomodel_t
{
	char					name[64];
	int					type;
	float					boundingradius;

	int					nummeshes;	
	int					meshindex;

	int					numvertices;		// number of unique vertices/normals/texcoords
	int					vertexindex;		// vertex Vector
	int					tangentsindex;		// tangents Vector

	int					numattachments;
	int					attachmentindex;

	int					numeyeballs;
	int					eyeballindex;

	mstudio_modelvertexdata_t vertexdata;

	int					unused[8];		// remove as appropriate
};
mstudiomesh_t *studiomodel_pMesh( mstudiomodel_t *t, int i ) { return (mstudiomesh_t *)(((char *)t) + t->meshindex) + i; };

typedef struct
{
	int					sznameindex;
	int					nummodels;
	int					base;
	int					modelindex; // index into models array
} mstudiobodyparts_t;

mstudiomodel_t *mstudiobodyparts_pModel( mstudiobodyparts_t *t, int i ) { return (mstudiomodel_t *)(((char *)t) + t->modelindex) + i; };

typedef struct {
	int					id;
	int					version;
	int					checksum;		// this has to be the same in the phy and vtx files to load!
	char					name[64];
	int					length;
	float					eyeposition[3];	// ideal eye position
	float					illumposition[3];	// illumination center
	float					hull_min[3];		// ideal movement hull size
	float					hull_max[3];
	float					view_bbmin[3];		// clipping bounding box
	float					view_bbmax[3];
	int					flags;
	int					numbones;			// bones
	int					boneindex;
	int					numbonecontrollers;		// bone controllers
	int					bonecontrollerindex;
	int					numhitboxsets;
	int					hitboxsetindex;
	int					numlocalanim;			// animations/poses
	int					localanimindex;		// animation descriptions
	int					numlocalseq;				// sequences
	int					localseqindex;
	int 					activitylistversion;
	int 					eventsindexed;
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
	int					numlocalattachments;
	int					localattachmentindex;
	// animation node to animation node transition graph
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
	int					numlocalposeparameters;
	int					localposeparamindex;
	int					surfacepropindex;
	// Key values
	int					keyvalueindex;
	int					keyvaluesize;
	int					numlocalikautoplaylocks;
	int					localikautoplaylockindex;
	// The collision model mass that jay wanted
	float					mass;
	int					contents;
	// external animations, models, etc.
	int					numincludemodels;
	int					includemodelindex;
	// for demand loaded animation blocks
	int					szanimblocknameindex;
	int					numanimblocks;
	int					animblockindex;
	int					bonetablebynameindex;
	char					constdirectionallightdot;
	char					rootLOD;
	char					numAllowedRootLODs;
	char					unused[1];
	int					unused4; // zero out if version < 47
	int					numflexcontrollerui;
	int					flexcontrolleruiindex;
	float					flVertAnimFixedPointScale;
	int					unused3[1];
	int					studiohdr2index;
	int					unused2[1];
} studiohdr_t;

mstudiobodyparts_t	*studiohdr_pBodypart( studiohdr_t *t, int i ) { return (mstudiobodyparts_t *)(((char *)t) + t->bodypartindex) + i; };

#pragma pack(pop)

// For counting indices (memory allocation)
void mdlRead_count( VTXStripHeader_t *strip, void *usr )
{
	*(uint32_t *)usr += strip->numIndices;
}

typedef enum
{
	k_EMdlLoad_valid,
	k_EMdlLoad_missing_vtx,
	k_EMdlLoad_missing_vvd,
	k_EMdlLoad_missing_mdl,
	k_EMdlLoad_nomem
} EMdlLoad_t;

typedef struct {

	uint16_t *indices;	
	uint32_t unIndices;
	
	float *vertices;
	uint32_t unVertices;

} mdl_mesh_t;

void mdl_free( mdl_mesh_t *m )
{
	free( m->indices );
	free( m->vertices );
}

EMdlLoad_t mdlRead_chkmem( EFileSysResult_t rr, EMdlLoad_t ii )
{
	if( rr == k_EFileSysResult_nomem ) return k_EMdlLoad_nomem;
	return ii;
}

EMdlLoad_t mdl_from_find_files( const char *mdlname, char *( *loader )( const char *, EFileSysResult_t * ), mdl_mesh_t *ctx )
{
	EFileSysResult_t read_result;
	// Read entire files into memory (inline functions map memory already)
	// .DX90.VTX
	char path[1024];
	strcpy( path, mdlname );
	path_stripext( path );
	strcat( path, ".dx90.vtx" );
	VTXFileHeader_t *pVtxHdr = (VTXFileHeader_t *)loader( path, &read_result );
	
	if( read_result != k_EFileSysResult_complete ){
		return mdlRead_chkmem( read_result, k_EMdlLoad_missing_vtx );
	}
	
	// .VVD
	strcpy( path, mdlname );
	path_stripext( path );
	strcat( path, ".vvd" );
	vertexFileHeader_t *pVvdHdr = (vertexFileHeader_t *)loader( path, &read_result );
	
	if( read_result != k_EFileSysResult_complete ){
		free( pVtxHdr );
		return mdlRead_chkmem( read_result, k_EMdlLoad_missing_vvd );
	}
	
	// .MDL
	strcpy( path, mdlname );
	path_stripext( path );
	strcat( path, ".mdl" );
	studiohdr_t *pMdl = (studiohdr_t *)loader( path, &read_result );
	
	if( read_result != k_EFileSysResult_complete ){
		free( pVtxHdr );
		free( pVvdHdr );
		return mdlRead_chkmem( read_result, k_EMdlLoad_missing_mdl );
	}
	
	iter_vtx( pVtxHdr, mdlRead_count, &ctx->unIndices );

	// Allocate and read indices
	ctx->indices = (uint16_t *)malloc( ctx->unIndices * sizeof(uint16_t) );
	ctx->unIndices = 0;	
	
	for ( int bodyID = 0; bodyID < pMdl->numbodyparts; ++bodyID )
	{
		// Body parts
		VTXBodyPartHeader_t* pVtxBodyPart = pBodyPartVTX( pVtxHdr, bodyID );
		mstudiobodyparts_t *pBodyPart = studiohdr_pBodypart( pMdl, bodyID );
		
		for ( int modelID = 0; modelID < pBodyPart->nummodels; ++modelID )
		{	
			// Models
			VTXModelHeader_t* pVtxModel = pModelVTX( pVtxBodyPart, modelID );
			mstudiomodel_t *pStudioModel = mstudiobodyparts_pModel( pBodyPart, modelID );

			int nLod = 0;
			VTXModelLODHeader_t *pVtxLOD = pLODVTX( pVtxModel, nLod );

			for ( int nMesh = 0; nMesh < pStudioModel->nummeshes; ++nMesh )
			{
				// Meshes
				VTXMeshHeader_t* pVtxMesh = pMeshVTX( pVtxLOD, nMesh );
				mstudiomesh_t* pMesh = studiomodel_pMesh( pStudioModel, nMesh );

				for ( int nGroup = 0; nGroup < pVtxMesh->numStripGroups; ++nGroup )
				{
					// Groups
					VTXStripGroupHeader_t* pStripGroup = pStripGroupVTX( pVtxMesh, nGroup );

					for ( int nStrip = 0; nStrip < pStripGroup->numStrips; nStrip++ )
					{
						// Strips
						VTXStripHeader_t *pStrip = pStripVTX( pStripGroup, nStrip );

						if ( pStrip->flags & STRIP_IS_TRILIST )
						{
							// Indices
							for ( int i = 0; i < pStrip->numIndices; i ++ )
							{
								uint16_t i1 =  *pIndexVTX( pStripGroup, pStrip->indexOffset + i );								
								ctx->indices[ ctx->unIndices ++ ] = pVertexVTX( pStripGroup, i1 )->origMeshVertID + pMesh->vertexoffset;
							}
						}
					}
				}
			}
		}
	}
	
	mstudiovertex_t *vertexData = GetVertexData( pVvdHdr );
	
	// Allocate vertex blob (XYZ|NRM|UV)
	ctx->unVertices = pVvdHdr->numLodVertexes[0];
	ctx->vertices = (float *)malloc( ctx->unVertices * 8 * sizeof( float ) );
	
	for( int i = 0; i < ctx->unVertices; i ++ )
	{
		mstudiovertex_t *vert = vertexData + i;
		memcpy( ctx->vertices + i * 8, vert->pos, 8 * sizeof(float) );
	}
	
	free( pVtxHdr );
	free( pVvdHdr );
	free( pMdl );
	
	return k_EMdlLoad_valid;
}

EMdlLoad_t mdl_read_fs( const char *mdlname, mdl_mesh_t *ctx )
{
	return mdl_from_find_files( mdlname, fs_get, ctx );
}

EMdlLoad_t mdl_read( const char *mdlname, mdl_mesh_t *ctx )
{
	return mdl_from_find_files( mdlname, file_to_buffer, ctx );
}

int mdl_to_obj( mdl_mesh_t *ctx, const char *path, const char *name )
{
	FILE *fp = fopen( path, "w" );

	if( fp ) 
	{
		fprintf( fp, "o %s\n", name );

		// Write vert blob
		for( int i = 0; i < ctx->unVertices; i ++ )
		{
			float *vert = ctx->vertices + i * 8;
			fprintf( fp, "v %f %f %f\n", vert[0], vert[1], vert[2] );
		}
		
		// Write normals block
		for( int i = 0; i < ctx->unVertices; i ++ )
		{
			float *vert = ctx->vertices + i * 8;
			fprintf( fp, "vn %f %f %f\n", vert[3], vert[4], vert[5] );
		}
		
		// UVS
		for( int i = 0; i < ctx->unVertices; i ++ )
		{
			float *vert = ctx->vertices + i * 8;
			fprintf( fp, "vt %f %f\n", vert[6], vert[7] );
		}
		
		fprintf( fp, "s on\n" );
		
		// Indices
		for( int i = 0; i < ctx->unIndices/3; i ++ )
		{
			uint16_t * base = ctx->indices + i*3;
			fprintf( fp, "f %u/%u/%u %u/%u/%u %u/%u/%u\n", base[0]+1, base[0]+1, base[0]+1, base[1]+1, base[1]+1, base[1]+1, base[2]+1, base[2]+1, base[2]+1 );
		}
	
		fclose( fp );
		return 1;
	}

	return 0;
}
