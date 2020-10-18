/*
	vdf.h - v1.10 - public domain
	vdf lightweight keyvalue parser for valvesoftware's VDF format
	
	version history:
			1.0 - 	init
			1.1 -		type converters [int, float, color, arrf, vecs]

	Main:
		vdf_node_t *vdf_open_file( const char *fn )		Load VDF from file fn. NULL if critical error
		int 			vdf_free( vdf_node_t *p )				Free VDF tree, has to be root node (returns 0 if warnings)
	
	Queries:
		const char *vdf_kv_get( vdf_node_t *pNode, const char *strKey, const char *strDefault )
		vdf_node_t *vdf_find( vdf_node_t *node, const char *szName )
			( Shorthand macro: VDF( NODE, SEARCHFOR ) ... NULL )
			
		KV type converters:
			PROP		CTYPE		PROTOTYPE( Node, key, default )	scanf format
			integer	int		vdfreadi									int32
			real		float		vdfreadf									float32
			
			Misc
			
			color		uint32	vdfreadcolor							uint8 uint8 uint8 uint8
			
			Short array types
			
						float[2]	vdfreadarrf2							[ float32 float32 ]
						float[3]	vdfreadarrf3							[ float32 float32 float32 ]
						float[4] vdfreadarrf4							[ float32 float32 float32 float32 ]
						
			Long arrays
						float *  vdfreadarrf								float32 ...
			
			Vector types
			
			vec2		float[2] vdfreadvec2								float32 float32
			vec3		float[3] vdfreadvec3								float32 float32 float32
			vec4		float[4] vdfreadvec4								float32 float32 float32 float32
			
			
	Examples:
	
	Find a node in pRoot named 'james':
	
		vdf_node_t *node = vdf_find(pRoot, "james");
		
		
	Find 'james' inside 'bob' in pRoot:
	
		vdf_node_t *jamebob = VDF( VDF( pRoot, "bob" ), "james" );
		
		
	Get a keyvalue 'colour' from jamebob:
	
		printf( "Value: %s\n", vdf_kv_get( jamebob, "colour", "[0 0 0]" ))
	
	
	Iterate through all nodes of pRoot named 'bob':
	
		int it = 0; 
		vdf_node_t *node;
		while( (node = vdf_iter(pRoot, "bob", &it)) )
		{
			// Do something with node
		}

*/

#define VDF_ITER( INODE, STR, EXPR ) \
{\
	int __ITER_IT__ = 0; vdf_node_t *NODE;\
	while( (NODE = vdf_iter( INODE, STR, &__ITER_IT__ )) )\
	{\
		EXPR ;\
	}\
}

#define VDF_KV_ITER( NODE, STR, EXPR ) \
{\
	int __ITER_IT__ = 0; char *KV;\
	while( (KV = vdf_kv_iter( NODE, STR, &__ITER_IT__ )) )\
	{\
		EXPR ;\
	}\
}

// C STD
// ===============================
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

// STB
#include "stb/stretchy_buffer.h"

typedef struct {

	char *szKey;
	char *szValue;

} vdf_kv_t;

typedef struct vdf_node_t vdf_node_t;
struct vdf_node_t
{
	char *szName;
	
	vdf_node_t *pParent;
	vdf_node_t **pChildren;
	vdf_kv_t *kvs;
};

/*
 Get a key from the KV pack
*/
const char *vdf_kv_get( vdf_node_t *pNode, const char *strKey, const char *strDefault )
{
	if( !pNode ) return strDefault;

	for( int i = 0; i < sb_count(pNode->kvs); i ++ )
	{
		if( !strcmp( pNode->kvs[i].szKey, strKey ) ){
			return pNode->kvs[i].szValue;
		}
	}
	
	return strDefault;
}

char *vdf_kv_iter( vdf_node_t *pNode, const char *strKey, int *it )
{
	if( !pNode ) return NULL;
	

	while( *it < sb_count( pNode->kvs ) )
	{	
		if( !strcmp( pNode->kvs[ *it ].szKey, strKey ) )
		{
			char *val = pNode->kvs[ *it ].szValue;
			*it = *it + 1;
			return val;
		}
		
		*it = *it + 1;
	}
	
	return NULL;
}

int vdfreadi( vdf_node_t *pNode, const char *strKey, const int iDefault )
{
	const char *v = vdf_kv_get( pNode, strKey, NULL );
	return v? atoi(v): iDefault;
}

uint32_t vdfreadcolor( vdf_node_t *pNode, const char *strKey, const uint32_t uDefault )
{
	const char *v = vdf_kv_get( pNode, strKey, NULL );
	
	if( v )
	{
		uint32_t val = 0xFF000000;
		sscanf( v, "%hhu %hhu %hhu %hhu", ((char *)&val)+0, ((char *)&val)+1, ((char *)&val)+2, ((char *)&val)+3 );
		return val;
	}
	
	return uDefault;
}

float vdfreadf( vdf_node_t *pNode, const char *strKey, float fDefault )
{
	const char *v = vdf_kv_get( pNode, strKey, NULL );
	if( v ) return atof( v );
	return fDefault;
}

// Read variable length array
void vdfreadarrf( vdf_node_t *pNode, const char *strKey, float *arr )
{
	char *val = NULL;	
	const char *_c = vdf_kv_get( pNode, strKey, NULL );
	
	if( !_c ) return;
	
	uint32_t len = strlen( _c ) + 1;
	char *buf = malloc( len );
	
	if( !buf ) return;
	
	memcpy( buf, _c, len );
	
	char *c = buf;
	
	while( *c )
	{
		if( isspace( *c ) )
		{
			*c = 0x00;
			
			if( val )
			{
				*(arr ++) = atof( val );
				val = NULL;
			}
		}
		else
		{
			if( !val )
			{
				val = c;
			}
		}
	
		c ++;
	}
	
	// Add remaining case if we hit null
	if( val )
	{
		*(arr ++) = atof( val );
	}
	
	free( buf );
}

void vdfreadarrf2( vdf_node_t *pNode, const char *strKey, float *vec )
{
	const char *v = vdf_kv_get( pNode, strKey, NULL );
	if( v ) sscanf( v, "[%f %f]", vec, vec+1 );
}
void vdfreadarrf3( vdf_node_t *pNode, const char *strKey, float *vec )
{
	const char *v = vdf_kv_get( pNode, strKey, NULL );
	if( v ) sscanf( v, "[%f %f %f]", vec, vec+1, vec+2 );
}
void vdfreadarrf4( vdf_node_t *pNode, const char *strKey, float *vec )
{
	const char *v = vdf_kv_get( pNode, strKey, NULL );
	if( v ) sscanf( v, "[%f %f %f %f]", vec, vec+1, vec+2, vec+3 );
}

void vdfreadvec2( vdf_node_t *pNode, const char *strKey, float *vec )
{
	const char *v = vdf_kv_get( pNode, strKey, NULL );
	if( v ) sscanf( v, "%f %f", vec, vec+1 );
}
void vdfreadvec3( vdf_node_t *pNode, const char *strKey, float *vec )
{
	const char *v = vdf_kv_get( pNode, strKey, NULL );
	if( v ) sscanf( v, "%f %f %f", vec, vec+1, vec+2 );
}
void vdfreadvec4( vdf_node_t *pNode, const char *strKey, float *vec )
{
	const char *v = vdf_kv_get( pNode, strKey, NULL );
	if( v ) sscanf( v, "%f %f %f %f", vec, vec+1, vec+2, vec+3 );
}

/*
 (Debug) Indent output
*/
void oindent( const int n )
{
	for( int x = 0; x < n; x ++ ) printf( "  " );
}

/*
 (Debug) Print the tree structure of a vdf node
*/
void _vdf_inspect( vdf_node_t *h, int lvl, int announce )
{
	if( announce )
	{ 
		oindent( lvl ); printf( "\"%s\"\n", h->szName );
	}
	
	for( int i = 0; i < sb_count( h->kvs ); i ++ )
	{
		oindent( lvl+1 ); printf( "\"%s\": \"%s\"\n", h->kvs[i].szKey, h->kvs[i].szValue );
	}
	
	for( int i = 0; i < sb_count( h->pChildren ); i ++ )
	{
		_vdf_inspect( h->pChildren[i], lvl + 1, 1 );
	}
}
void vdf_inspect( vdf_node_t *h ) { _vdf_inspect( h, -1, 0 ); }

/*
 Create a node with name, append to p if not NULL else it is root
*/
vdf_node_t *vdf_create( vdf_node_t *p, const char *szName )
{
	vdf_node_t *head = calloc( 1, sizeof( vdf_node_t ) );
	
	head->szName = malloc( strlen( szName )+1 );
	strcpy( head->szName, szName );
	
	if( p )
	{
		head->pParent = p;
		sb_push( p->pChildren, head );
	}
	
	return head;
}

/*
 Append keyvalue [ k: v ] to node p
*/
void vdf_kv_append( vdf_node_t *p, const char *k, const char *v )
{
	uint32_t sv = strlen(v)+1;
	uint32_t sk = strlen(k)+1;
	
	vdf_kv_t kv;
	kv.szKey = malloc( sv+sk );
	kv.szValue = kv.szKey+sk;
	
	memcpy( kv.szKey, k, sk );
	memcpy( kv.szValue, v, sv );
	sb_push( p->kvs, kv );
}

/*
 Free VDF tree from node, no error check
*/
void _vdf_free( vdf_node_t *p )
{
	for( int i = 0; i < sb_count( p->kvs ); i ++ )
	{
		free( p->kvs[i].szKey );
	}

	for( int i = 0; i < sb_count( p->pChildren ); i ++ )
	{
		_vdf_free( p->pChildren[i] );
	}
	
	sb_free( p->pChildren );
	sb_free( p->kvs );
	free( p );
}

/*
 Free VDF tree from root node. Error checks non-root
*/
int vdf_free( vdf_node_t *p )
{
	if( p )
	{
		_vdf_free( p );
		
		if( p->pParent )
		{
			fprintf( stderr, "Freeing vdf node with parent. This will segfault!\n" );
			return 0;
		}
	}
	return 1;
}

/*
 Parsing context
*/
typedef struct 
{
	char szName[2048];
	int bUplvl;
	
	vdf_node_t *pHead;
	vdf_node_t *pRoot;
	
	uint32_t unLine;
	uint32_t unErrors;
	
	// State
	struct {
		int bQuot;
		char *szTokens[2];
		int nTok;
		char cLast;
		int wait;
	} st;
	
} vdf_ctx_t;

void vdf_ctx_newln( vdf_ctx_t *ctx )
{
	ctx->unLine ++;

	ctx->st.bQuot = 0;
	ctx->st.szTokens[0] = NULL;
	ctx->st.szTokens[1] = NULL;
	ctx->st.nTok = 0;
	ctx->st.cLast = 0x00;
	ctx->st.wait = 0;
}

void vdf_ctx_endl( vdf_ctx_t *ctx )
{
	// Final out-tokens
	if( ctx->st.szTokens[ 0 ] )
	{
		if( ctx->st.szTokens[ 1 ] )
		{
			vdf_kv_append( ctx->pHead, ctx->st.szTokens[0], ctx->st.szTokens[1] );
		}
		else
		{
			strcpy( ctx->szName, ctx->st.szTokens[0] );
			ctx->bUplvl = 1;
		}
	}
	
	vdf_ctx_newln( ctx );
}

/*
 Input a new line (ln) into the parser(ctx)
*/
void vdf_parse_feedch( vdf_ctx_t *ctx, char *c )
{
	char cur = *c;

	if( *c == 0x0D )
	{
		*c = 0x00;
		ctx->st.wait = 1;
	}
	
	else if( *c == 0x0A )
	{
		*c = 0x00;
		vdf_ctx_endl( ctx );
	}
	
	else if( !ctx->st.wait )
	{
		// Break on comments
		if( *c == '/' && ctx->st.cLast == '/' )
		{
			*(c-1) = 0x00;
			ctx->st.wait = 1;
		}
		
		// Switch quotation-read mode
		else if( *c == '"' )
		{
			ctx->st.bQuot = !ctx->st.bQuot;
			
			if( ctx->st.cLast == '"' )
			{
				ctx->st.szTokens[ ctx->st.nTok ++ ] = (c-1);
			}
			
			*c = 0x00;
		}
		
		// Enter new block (named previous line tks[0])
		else if( *c == '{' )
		{
			if( ctx->st.szTokens[0] || !ctx->bUplvl )
			{
				fprintf( stderr, "Unexpected token '{' (Line: %u)\n", ctx->unLine );
				ctx->unErrors ++;
			}
			
			ctx->bUplvl = 0;
			ctx->pHead = vdf_create( ctx->pHead, ctx->szName );
			ctx->st.wait = 1;
		}
		
		// Closing block, jump read head back to parent
		else if( *c == '}' )
		{
			if( !ctx->pHead->pParent )
			{
				fprintf( stderr, "Unexpected token '}' (Line: %u)\n", ctx->unLine );
				ctx->unErrors ++;
			}
			else
			{
				ctx->pHead = ctx->pHead->pParent;
			}
			ctx->st.wait = 1;
		}
		
		else
		{
			// Non-quote token seperation (whitespace delimited)

			if( isspace( *c ) )
			{
				if( !ctx->st.bQuot )
				{
					*c = 0x00;

					if( ctx->st.szTokens[ ctx->st.nTok ] )
					{
						ctx->st.nTok ++;
						
						if( ctx->st.nTok == 2 ) ctx->st.wait = 1;
					}
				}
			}
			// Start new entry
			else if( !ctx->st.szTokens[ ctx->st.nTok ] )
			{
				// Make sure we dont start a token using //
				// (lookahead is fine; worst case it indexes 0x00)
				if( !( *c == '/' && *(c + 1) == *c ) )
				{	
					ctx->st.szTokens[ ctx->st.nTok ] = c;

					if( ctx->bUplvl )
					{
						fprintf( stderr, "Unexpected token '%s' (Line: %u)\n", ctx->szName, ctx->unLine );
						ctx->unErrors ++;
					}
				}
			}
		}
	}

	ctx->st.cLast = cur;
}

/*
 Find VDF node in tree with name. Search begins after child 'pAfter'
*/
vdf_node_t *vdf_iter( vdf_node_t *node, const char *szName, int *it )
{	
	if( !node ) return NULL;

	for( int i = it? *it: 0; i < sb_count( node->pChildren ); i ++ )
	{
		if( !strcmp( szName, node->pChildren[i]->szName ))
		{
			if( it ) *it = i+1;
			return node->pChildren[i];
		}
	}
	
	return NULL;
}
#define vdf_find( N, S ) vdf_iter( N, S, 0 )
#define VDF( n, str ) vdf_find( n, str )

/*
 Load VDF from file fn
*/
vdf_node_t *vdf_open_file( const char *fn )
{	
	EFileSysResult_t result;
	char *megabuf = textfile_to_buffer( fn, &result );
	
	if( result != k_EFileSysResult_complete ) 
	{ 
		fprintf( stderr, "Unable to open file ( code %i ) @'%s'\n", result, fn );
		return NULL;
	}
	
	vdf_ctx_t ctx = {0};
	ctx.pRoot = ctx.pHead = vdf_create( NULL, "root" );
	
	vdf_ctx_newln( &ctx );
	
	for( char *c = megabuf; *c; c ++ )
	{
		vdf_parse_feedch( &ctx, c );
	}
	
	free( megabuf );
	
	return ctx.pRoot;
}


void _vdf_out_indent( const int n, FILE *file )
{
	for( int x = 0; x < n; x ++ ) fprintf( file, "\t" );
}

/*
 (Debug) Print the tree structure of a vdf node
*/
void _vdf_out( vdf_node_t *h, int lvl, int announce, FILE *file )
{
	if( announce )
	{ 
		_vdf_out_indent( lvl, file ); fprintf( file, "\"%s\"\n", h->szName );
		_vdf_out_indent( lvl, file ); fprintf( file, "{\n" );
	}
	
	for( int i = 0; i < sb_count( h->kvs ); i ++ )
	{
		_vdf_out_indent( lvl+1, file ); fprintf( file, "\"%s\" \"%s\"\n", h->kvs[i].szKey, h->kvs[i].szValue );
	}
	
	for( int i = 0; i < sb_count( h->pChildren ); i ++ )
	{
		_vdf_out( h->pChildren[i], lvl + 1, 1, file );
	}
	
	if( announce )
	{
		_vdf_out_indent( lvl, file ); fprintf( file, "}\n" );
	}
}

void vdf_save( vdf_node_t *node, const char *fn )
{
	FILE* file = fopen( fn, "w" );
	
	_vdf_out( node, -1, 0, file );
	
	fclose( file );
}

