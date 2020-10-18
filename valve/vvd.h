//StudioMDL constants
#define MAX_NUM_LODS 8
#define MAX_NUM_BONES_PER_VERT 3

#pragma pack(push, 1)
typedef struct
{
	float	weight[MAX_NUM_BONES_PER_VERT];
	char 	bone[MAX_NUM_BONES_PER_VERT];
	char 	numbones;
} boneWeight_t;

typedef struct
{
	boneWeight_t boneweights;
	float 		pos[3];
	float 		norm[3];	
	float 		uv[2];
} mstudiovertex_t;

typedef struct
{
	int id;
	int version;
	int checksum;
	int numLods;
	int numLodVertexes[MAX_NUM_LODS];
	int numFixups;
	int fixupTableStart;
	int vertexDataStart;
	int tangentDataStart;
} vertexFileHeader_t;

#pragma pack(pop)

mstudiovertex_t *GetVertexData( vertexFileHeader_t *t ) 
{	
	return ( mstudiovertex_t * ) ( (char *)t + t->vertexDataStart );
}
