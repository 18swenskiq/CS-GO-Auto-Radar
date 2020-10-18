/* temp
*/
void
glmd_vec3d_to_vec3( double p[3], vec3 out )
{
	out[0] = (float)p[0];
	out[1] = (float)p[1];
	out[2] = (float)p[2];
}

#pragma pack(push, 1)
typedef struct
{

	float	co	[3];
	float	nrm[3];
	float xy	[2];	// Used for origin

} solidgen_vert_t;
#pragma pack(pop)

typedef struct
{
	solidgen_vert_t *verts;
	uint32_t *indices;
	uint32_t unVerts;
	uint32_t unCapacity;
	
	uint32_t idx;
	uint32_t idxCapacity;

} solidgen_ctx_t;

void solidgen_ctx_reset( solidgen_ctx_t *ctx )
{
	ctx->unVerts = 0;
	ctx->idx = 0;
}

void solidgen_ctx_init( solidgen_ctx_t *ctx )
{
	ctx->unCapacity = 128;
	ctx->verts = malloc( sizeof(solidgen_vert_t) * 128 );
	
	ctx->idxCapacity = 128;
	ctx->indices = malloc( sizeof(uint32_t) * 128 );
	
	solidgen_ctx_reset( ctx );
}

void solidgen_ctx_free( solidgen_ctx_t *ctx )
{
	free( ctx->verts );
	free( ctx->indices );
}

void solidgen_gl_buffer( solidgen_ctx_t *ctx, GLuint *pVao, GLuint *pVbo, GLuint *pEbo )
{
	glGenVertexArrays( 1, pVao );
	glGenBuffers( 1, pVbo );
	glGenBuffers( 1, pEbo );
	glBindVertexArray( *pVao );
		
	glBindBuffer( GL_ARRAY_BUFFER, *pVbo );
	glBufferData( GL_ARRAY_BUFFER, ctx->unVerts * sizeof( solidgen_vert_t ), ctx->verts, GL_STATIC_DRAW );
		
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, *pEbo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, ctx->idx * sizeof( uint32_t ), ctx->indices, GL_STATIC_DRAW );
	
	// co
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( solidgen_vert_t ), (void *)0 );
	glEnableVertexAttribArray( 0 );
	
	// normal
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( solidgen_vert_t ), (void *)( sizeof(float) * 3 ) );
	glEnableVertexAttribArray( 1 );
	
	// origin
	glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( solidgen_vert_t ), (void *)( sizeof(float) * 6 ) );
	glEnableVertexAttribArray( 2 );
}

typedef enum
{
	k_ESolidResult_valid,
	k_ESolidResult_maxsides,
	k_ESolidResult_invalid,
	k_ESolidResult_errnomem,
	k_ESolidResult_corrupt,
	k_ESolidResult_degenerate
} ESolidResult_t;

#define SOLID_MAX_SIDES 512

void sort_coplanar( double p[4], solidgen_vert_t *points, uint32_t *idxs, uint32_t unCount )
{
	vec3 center = GLM_VEC3_ZERO_INIT;
	vec3 norm;
	glmd_vec3d_to_vec3( p, norm );
	glm_vec3_normalize( norm );
	
	for( int i = 0; i < unCount; i ++ )
	{
		glm_vec3_add( points[ idxs[i] ].co, center, center );
	}
	glm_vec3_divs( center, unCount, center );
	
	vec3 ref;
	glm_vec3_sub( points[ idxs[0] ].co, center, ref );
	
	// Calc angles compared to ref
	float *angles = (float*)alloca(sizeof(float)*unCount);
	for( int i = 0; i < unCount; i ++ )
	{
		vec3 diff;
		vec3 c;
		
		glm_vec3_sub( points[ idxs[i] ].co, center, diff );
		glm_vec3_cross( diff, ref, c );
		
		angles[i] =
			atan2f( glm_vec3_norm(c), glm_vec3_dot( diff, ref ) )
			* (glm_vec3_dot( c, norm ) < 0.f ? -1.f: 1.f);
	}
	
	// Temporary local indexes
	uint32_t *tidx = (uint32_t*)alloca(sizeof(uint32_t)*unCount);
	for( uint32_t i = 0; i < unCount; i ++ ) tidx[i] = i;
	
	// Sort points. (naive sluggish sort, i know, too bad)
	int it = 0;
	while(1)
	{
		int bMod = 0;
		for( int i = 0; i < unCount-1; i ++ )
		{
			int s0 = i; int s1 = i + 1;
			
			if( angles[tidx[s0]] > angles[tidx[s1]] )
			{
				// swap indices and mirror on local
				uint32_t temp = idxs[s1];
				idxs[s1] = idxs[s0];
				idxs[s0] = temp;
				
				temp = tidx[s1];
				tidx[s1] = tidx[s0];
				tidx[s0] = temp;
				
				bMod = 1;
			}
		}
		
		it ++;
		if( !bMod ) break;
	}
}

int solid_has_displacement( vdf_node_t *node )
{
	int it = 0;
	vdf_node_t *pSide;
	while( (pSide = vdf_iter(node, "side", &it)) )
	{
		if( vdf_find( pSide, "dispinfo" ) )
		{
			return 1;
		}
	}
	return 0;
}

#define __DISPTRI( a,b,c  ) \
 ctx->indices[ ctx->idx ++ ] = a; \
 ctx->indices[ ctx->idx ++ ] = b; \
 ctx->indices[ ctx->idx ++ ] = c; 

ESolidResult_t solidgen_push( solidgen_ctx_t *ctx, vdf_node_t *node )
{
	ESolidResult_t flag = k_ESolidResult_valid;

	double planes[SOLID_MAX_SIDES*4];
	uint32_t **polygons = NULL;
	vdf_node_t **dispinfos = NULL;
	
	int bDisplaced = 0;
	
	// Extract planes from points to NRM/D format
	int it = 0; 
	int num = 0;
	vdf_node_t *pSide;
	while( (pSide = vdf_iter(node, "side", &it)) )
	{
		double points[3*3];
		
		const char *v = vdf_kv_get( pSide, "plane", NULL );
		
		if( !v )
		{ 
			// todo: fix leak here ( although the case should never happen in practice.. )
			return k_ESolidResult_corrupt;
		}
		
		sscanf( v, "(%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf)", 
			points+0, points+1, points+2,
			points+3, points+4, points+5,
			points+6, points+7, points+8 );
			
		glmd_triangle_to_plane( points+0, points+3, points+6, planes +num*4 );
		
		sb_push( polygons, NULL );

		num ++;
		
		if( num >= SOLID_MAX_SIDES )
		{
			flag = k_ESolidResult_maxsides;
			fprintf( stderr, "Solid over maxsides limit %i\n", SOLID_MAX_SIDES );
			break;
		}
		
		vdf_node_t *dispinfo = vdf_find( pSide, "dispinfo" );
		
		if( dispinfo )
		{
			bDisplaced = 1;
			sb_push( dispinfos, dispinfo );
		}
		else
		{
			sb_push( dispinfos, NULL );
		}
	}	
	
	// Generating unique permutations of the three planes
	// in a more effecient way than three nested loops..
	// Todo: Clean up the interface for this algorithm; its in a little bit of a mess
	
	int ti, tx, ty, tz, i, j, k;
	int *tp = (int *)malloc( (num*2+2) * sizeof(int) );
	int *tb = tp +num+2;
	inittwiddle(3, num, tp, tb);
	i = num-3;
	j = num-2;
	k = num-1;
	
	// For centering
	
	vec3 		center = GLM_VEC3_ZERO_INIT;
	int		numpoints = 0;
	uint32_t vStart = ctx->unVerts;
	
	while( 1 )
	{
		// DO something with i j k
		double p[3];
				
		if( !glmd_plane_intersect( planes+i*4, planes+j*4, planes+k*4, p ) ) goto IL_TWIDDLE;
		
		// Check for illegal verts
		int valid = 1;
		for( int m = 0; m < num; m ++ )
		{
			if( glmd_plane_polarity( planes+m*4, p ) < -1e-6f )
			{
				valid = 0;
				break;
			}
		}
		
		if( valid ) 
		{
			if( ctx->unVerts +3 >= ctx->unCapacity )
			{
				ctx->unCapacity = __max(ctx->unVerts + 3, ctx->unCapacity * 2 );
				solidgen_vert_t *r = realloc( ctx->verts, ctx->unCapacity * sizeof(solidgen_vert_t) );
				if( !r ) // No memory for further allocation
				{
					flag = k_ESolidResult_errnomem;
					goto IL_GEN_NOMEM;
				}
				else
				{
					ctx->verts = r;
				}
			}
			
			// Store point / respecive normal for each plane that triggered the collision
			glmd_vec3d_to_vec3( p, ctx->verts[ ctx->unVerts+0 ].co );
			glmd_vec3d_to_vec3( planes+i*4, ctx->verts[ ctx->unVerts+0 ].nrm );
			
			// Take the vertex position and add it for centering base on average
			numpoints ++;
			glm_vec3_add( ctx->verts[ ctx->unVerts+0 ].co, center, center );
			
			glmd_vec3d_to_vec3( p, ctx->verts[ ctx->unVerts+1 ].co );
			glmd_vec3d_to_vec3( planes+j*4, ctx->verts[ ctx->unVerts+1 ].nrm );
			glmd_vec3d_to_vec3( p, ctx->verts[ ctx->unVerts+2 ].co );
			glmd_vec3d_to_vec3( planes+k*4, ctx->verts[ ctx->unVerts+2 ].nrm );
			
			sb_push( polygons[i], ctx->unVerts );
			sb_push( polygons[j], ctx->unVerts + 1 );
			sb_push( polygons[k], ctx->unVerts + 2 );
			ctx->unVerts += 3;
		}

IL_TWIDDLE:

		// Todo: again to clean up twiddle interface
		if( twiddle(&tx, &ty, &tz, tp ) ) break;	
		tb[tx] = 1;
		tb[ty] = 0;
		int c = 0;
		int o[3];
		for(ti = 0; ti != num; ti++)
			if( tb[ti] ) 
				o[ c ++ ] = ti;
		i = o[0];
		j = o[1];
		k = o[2];
	}
	
	// Retrospectively set the center for each point
	glm_vec3_divs( center, (float)numpoints, center );
	for( ; vStart < ctx->unVerts; vStart ++ )
	{
		glm_vec2_copy( center, ctx->verts[ vStart ].xy );
	}
	
	// Sort each faces and trianglulalate them
	for( i = 0; i < num; i ++ )
	{
		if( sb_count(polygons[i]) < 3 )
		{
			flag = k_ESolidResult_degenerate;
			fprintf( stderr, "Skipping degenerate face\n" );
			continue;
		}
		
		// Sort only if there is no displacements, or if this side is
		if( !bDisplaced || (bDisplaced && dispinfos[ i ]) )
		{
			sort_coplanar( planes+i*4, ctx->verts, polygons[i], sb_count(polygons[i]) );
		}
		
		if( bDisplaced )
		{
			// Compute displacements
			if( dispinfos[ i ] )
			{
				if( sb_count( polygons[i] ) != 4 )
				{
					flag = k_ESolidResult_degenerate;
					fprintf( stderr, "Skipping degenerate displacement\n" );
					continue;
				}
			
				// Match starting position	
				float start[3];
				int sw = 0;
				float dmin = 999999.f;
				
				vdf_node_t *dispinfo = dispinfos[i];
				vdf_node_t *pNormals = vdf_find( dispinfo, "normals" );
				vdf_node_t *pDistances = vdf_find( dispinfo, "distances" );
				
				vdfreadarrf3( dispinfo, "startposition", start );
			
				for( int j = 0; j < sb_count( polygons[i] ); j ++ )
				{
					float d2 = glm_vec3_distance2( start, ctx->verts[ polygons[i][j] ].co );
					if( d2 < dmin )
					{
						dmin = d2;
						sw = j;
					}
				}
				
				// Get corners of displacement
				float *SW = ctx->verts[ polygons[i][sw] ].co;
				float *NW = ctx->verts[ polygons[i][(sw+1) % 4] ].co;
				float *NE = ctx->verts[ polygons[i][(sw+2) % 4] ].co;
				float *SE = ctx->verts[ polygons[i][(sw+3) % 4] ].co;
				
				// Can be either 5, 9, 17
				int numpoints = pow( 2, vdfreadi( dispinfo, "power", 2 ) ) + 1;
				
				uint32_t reqverts = numpoints*numpoints;
				if( ctx->unVerts +reqverts >= ctx->unCapacity )
				{
					ctx->unCapacity = __max(ctx->unVerts + reqverts, ctx->unCapacity * 2 );
					solidgen_vert_t *r = realloc( ctx->verts, ctx->unCapacity * sizeof(solidgen_vert_t) );
					if( !r ) // No memory for further allocation
					{
						flag = k_ESolidResult_errnomem;
						goto IL_GEN_NOMEM;
					}
					else
					{
						ctx->verts = r;
					}
				}
				
				uint32_t reqidx = (numpoints-1)*(numpoints-1)*6;
				if( ctx->idx + reqidx >= ctx->idxCapacity )
				{
					ctx->idxCapacity = __max( ctx->idx + reqidx, ctx->idxCapacity * 2 );
					uint32_t *r = realloc( ctx->indices, ctx->idxCapacity * 3 * sizeof(uint32_t) );
					if( !r )
					{
						flag = k_ESolidResult_errnomem;
						goto IL_GEN_NOMEM;
					}
					else
					{
						ctx->indices = r;
					}
				}
				
				float normals[ 17*3 ];
				float distances[ 17 ];
				
				// Calculate displacement positions
				for( int j = 0; j < numpoints; j ++ )
				{
					char key[14];
					sprintf( key, "row%i", j );
					
					vdfreadarrf( pNormals, key, normals );
					vdfreadarrf( pDistances, key, distances );
					
					float dx = (float)j / (float)(numpoints - 1); //Time values for linear interpolation
					
					for( int k = 0; k < numpoints; k ++ )
					{
						float dy = (float)k / (float)(numpoints - 1);

						vec3 lwr; vec3 upr;
						
						float *p = ctx->verts[ ctx->unVerts + j*numpoints + k ].co;
						
						glm_vec3_lerp( SW, SE, dx, lwr );
						glm_vec3_lerp( NW, NE, dx, upr );
						glm_vec3_lerp( lwr, upr, dy, p );

						glm_vec3_muladds( normals + k * 3, distances[ k ], p );
						
						// Todo, put correct normal
						glm_vec3_copy( (vec3){ 0.f, 0.f, 1.f }, p + 3 );
					}
				}
				
				// Build displacement indices
				int condition = 0;
				for ( int row = 0; row < numpoints - 1; row ++ ) {
					for ( int col = 0; col < numpoints - 1; col ++ ) {
						uint32_t idxsw = ctx->unVerts + ( row + 0 ) * numpoints + col + 0 ;
						uint32_t idxse = ctx->unVerts + ( row + 0 ) * numpoints + col + 1 ;
						uint32_t idxnw = ctx->unVerts + ( row + 1 ) * numpoints + col + 0 ;
						uint32_t idxne = ctx->unVerts + ( row + 1 ) * numpoints + col + 1 ;

						if( (condition ++) % 2 == 0 )
						{
							__DISPTRI( idxne, idxnw, idxsw );
							__DISPTRI( idxse, idxne, idxsw );
						}
						else
						{
							__DISPTRI( idxse, idxnw, idxsw );
							__DISPTRI( idxse, idxne, idxnw );
						}
					}
					condition ++;
				}
				
				ctx->unVerts += numpoints*numpoints;
			}
		}
		else
		{
			uint32_t tris = sb_count(polygons[i]) -2;
			if( ctx->idx + tris*3 >= ctx->idxCapacity )
			{
				ctx->idxCapacity = __max( ctx->idx + tris*3, ctx->idxCapacity * 2 );
				uint32_t *r = realloc( ctx->indices, ctx->idxCapacity * 3 * sizeof(uint32_t) );
				if( !r )
				{
					flag = k_ESolidResult_errnomem;
					goto IL_GEN_NOMEM;
				}
				else
				{
					ctx->indices = r;
				}
			}
			
			for( int j = 0; j < tris; j ++ )
			{
				ctx->indices[ ctx->idx +j*3 +0 ] = polygons[i][0];
				ctx->indices[ ctx->idx +j*3 +1 ] = polygons[i][j+1];
				ctx->indices[ ctx->idx +j*3 +2 ] = polygons[i][j+2];

				//   A			0,1,2:: A,B,C
				// D    B		0,2,3:: A,C,D
				//   C
			}
			
			ctx->idx += tris*3;
		}
	}

IL_GEN_NOMEM: // Out of memory was triggered, clean up all the buffers	

	free( tp );
	
	// Free polyon buffers
	for( int i = 0; i < num; i ++ )
	{
		sb_free( polygons[i] );
	}
	
	sb_free( polygons );
	
	return flag;
}

void solidgen_to_obj( solidgen_ctx_t *ctx, const char *path )
{
	FILE *fp = fopen(path,"w");

	if( fp ) 
	{
		fprintf( fp, "o vmf_export\n" );
	
		// Write vertex block
		for( int i = 0; i < ctx->unVerts; i ++ )
		{
			fprintf( fp, "v %f %f %f\n", ctx->verts[i].co[0], ctx->verts[i].co[1], ctx->verts[i].co[2] );
		}
		
		// Write normals block
		for( int i = 0; i < ctx->unVerts; i ++ )
		{
			fprintf( fp, "vn %f %f %f\n", ctx->verts[i].nrm[0], ctx->verts[i].nrm[1], ctx->verts[i].nrm[2] );
		}
		
		fprintf( fp, "s off\n" );
		
		// Indices
		for( int i = 0; i < ctx->idx/3; i ++ )
		{
			uint32_t * base = ctx->indices + i*3;
			fprintf( fp, "f %u//%u %u//%u %u//%u\n", base[0]+1, base[0]+1, base[1]+1, base[1]+1, base[2]+1, base[2]+1 );
		}
	
		fclose( fp );
	}
	else
	{
		fprintf( stderr, "Could not open %s for writing\n", path );
	}
}

// Get visgroup id from string name
int vmf_getvisgroupid( vdf_node_t *root, const char *szName )
{
	vdf_node_t *visgroups = VDF( root, "visgroups" );
	
	VDF_ITER( visgroups, "visgroup", 
	
		if( !strcmp( vdf_kv_get( NODE, "name", "" ), szName ) )
		{
			return vdfreadi( NODE, "visgroupid", -1 );
		}
	
	);
	
	return -1;
}

// Bake an entity transform to mat4
void vmf_bake_transform( vdf_node_t *ent, mat4 dst )
{
	glm_mat4_identity( dst );
	
	vec3 angles = GLM_VEC3_ZERO_INIT;
	vec3 offset = GLM_VEC3_ZERO_INIT;
	float scale;
	
	scale = vdfreadf( ent, "uniformscale", 1.f );
	
	vdfreadarrf( ent, "angles", angles );
	vdfreadarrf( ent, "origin", offset );
	
	// Translation
	glm_translate( dst, offset );
	
	// Make rotation ( Pitch yaw roll // YZX. Source->OpenGL ordering a lil messed up )
	glm_rotate_z( dst, glm_rad(angles[1]), dst );
	glm_rotate_y( dst, glm_rad(angles[0]), dst );
	glm_rotate_x( dst, glm_rad(angles[2]), dst );
	
	// Scale
	glm_scale_uni( dst, scale );
}
