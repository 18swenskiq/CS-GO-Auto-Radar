/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef cglm_plane_h
#define cglm_plane_h

#include "common.h"
#include "vec4.h"
#include "vec3.h"

/*
 Plane equation:  Ax + By + Cz + D = 0;

 It stored in vec4 as [A, B, C, D]. (A, B, C) is normal and D is distance
*/

/*
 Functions:
   CGLM_INLINE void  glm_plane_normalize(vec4 plane);
   CGLM_INLINE int   glm_plane_intersect(vec4 a, vec4 b, vec4 c, vec3 p);   (WIP)
   CGLM_INLINE void  glm_triangle_to_plane(vec3 a, vec3 b, vec3 c, vec4 p); (WIP)
 */

/*!
 * @brief normalizes a plane
 *
 * @param[in, out] plane plane to normalize
 */
CGLM_INLINE
void
glm_plane_normalize(vec4 plane) {
  float norm;
  
  if ((norm = glm_vec3_norm(plane)) == 0.0f) {
    glm_vec4_zero(plane);
    return;
  }
  
  glm_vec4_scale(plane, 1.0f / norm, plane);
}

/*!
 * @brief intersection point of 3 planes
 *
 * @param[in] a plane A
 * @param[in] b plane B
 * @param[in] c plane C
 * @param[out] p intersection point
 * @return true if planes have valid intersection and are not parallel in some way
 */
CGLM_INLINE
int 
glm_plane_intersect(vec4 a, vec4 b, vec4 c, vec3 p) {
  float const epsilon = 1e-5f;

  vec3 acb;
  vec3 term;
  float det;
  
  glm_vec3_cross( a, b, acb );
  det = glm_vec3_dot( acb, c );
  
  if( det < epsilon && det > -epsilon ) return 0;

  glm_vec3_cross( b, c, term );
  glm_vec3_scale( term, -a[3], p );
  
  glm_vec3_cross( c, a, term );
  glm_vec3_muladds( term, -b[3], p );
  
  glm_vec3_cross( a, b, term );
  glm_vec3_muladds( term, -c[3], p );
  
  glm_vec3_negate( p );
  
  glm_vec3_cross( b, c, term );
  glm_vec3_divs( p, glm_vec3_dot( a, term ), p );
  
  return 1;
}

/*!
 * @brief calculates the plane from triangles vertices (favours accuracy over speed)
 *
 * @param[in] a point 0
 * @param[in] b point 1
 * @param[in] c point 2
 * @param[out] p plane
 */
CGLM_INLINE
void
glm_triangle_to_plane(vec3 a, vec3 b, vec3 c, vec4 p){
  vec3 edge0;
  vec3 edge1;
  float l;
  
  glm_vec3_sub( b, a, edge0 );
  glm_vec3_sub( c, a, edge1 );
  
  glm_vec3_cross( edge0, edge1, p );
  
  l = glm_vec3_norm( p );
  p[3] = glm_vec3_dot( p, a ) / l;
  
  glm_vec3_divs( p, l, p );
}

#endif /* cglm_plane_h */
