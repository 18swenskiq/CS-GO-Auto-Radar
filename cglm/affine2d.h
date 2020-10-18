/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

/*
 Functions:
   CGLM_INLINE void glm_translate2d(mat3 m, vec2 v);
   CGLM_INLINE void glm_translate2d_make(mat3, m, vec2 v);
   CGLM_INLINE void glm_rotate2d(mat3 m, float angle, mat3 dest);
   CGLM_INLINE void glm_rotate2d_make(mat3 m, float angle);
   CGLM_INLINE void glm_scale_uni2d(mat3 m, float scale);
   CGLM_INLINE void glm_scale2d(mat3 m, vec2 v);
 */

#ifndef cglm_affine2d_h
#define cglm_affine2d_h

#include "common.h"
#include "util.h"
#include "vec2.h"
#include "vec3.h"
#include "mat3.h"

/*!
 * @brief translate existing transform matrix by v vector
 *        and stores result in same matrix
 *
 * @param[in, out]  m  affine transfrom
 * @param[in]       v  translate vector [x, y]
 */
CGLM_INLINE
void
glm_translate2d(mat3 m, vec2 v) {
  vec3 v1, v2;

  glm_vec3_scale(m[0], v[0], v1);
  glm_vec3_scale(m[1], v[1], v2);

  glm_vec3_add(v1, m[2], m[2]);
  glm_vec3_add(v2, m[2], m[2]);
}

/*!
 * @brief creates NEW translate transform matrix by v vector
 *
 * @param[out]  m  affine transfrom
 * @param[in]   v  translate vector [x, y]
 */
CGLM_INLINE
void
glm_translate2d_make(mat3 m, vec2 v) {
  glm_mat3_identity(m);
  glm_vec2_copy(v, m[2]);
}

/*!
 * @brief rotate existing transform matrix by angle
 *        and store result in dest
 *
 * @param[in]   m      affine transfrom
 * @param[in]   angle  angle (radians)
 * @param[out]  dest   rotated matrix
 */
CGLM_INLINE
void
glm_rotate2d(mat3 m, float angle, mat3 dest){
  CGLM_ALIGN_MAT mat3 t = GLM_MAT3_IDENTITY_INIT;
  float c, s;

  c = cosf(angle);
  s = sinf(angle);

  t[0][0] =  c;
  t[0][1] = -s;
  t[1][0] =  s;
  t[1][1] =  c;

  glm_mat3_mul(m, t, dest);
}

/*!
 * @brief scale existing transform matrix by v vector
 *        and store result in dest
 *
 * @param[in]  m    affine transfrom
 * @param[in]  v    scale vector [x, y]
 * @param[out] dest scaled matrix
 */
CGLM_INLINE
void
glm_scale_to2d(mat3 m, vec2 v, mat3 dest) {
  glm_vec3_scale(m[0], v[0], dest[0]);
  glm_vec3_scale(m[1], v[1], dest[1]);

  glm_vec3_copy(m[2], dest[2]);
}

/*!
 * @brief scales existing transform matrix by v vector
 *        and stores result in same matrix
 *
 * @param[in, out]  m  affine transfrom
 * @param[in]       v  scale vector [x, y]
 */
CGLM_INLINE
void
glm_scale2d(mat3 m, vec2 v) {
  glm_scale_to2d(m, v, m);
}

/*!
 * @brief applies uniform scale to existing transform matrix v = [s, s]
 *        and stores result in same matrix
 *
 * @param[in, out]  m  affine transfrom
 * @param[in]       s  scale factor
 */
CGLM_INLINE
void
glm_scale_uni2d(mat3 m, float s) {
  vec2 v = { s, s };
  glm_scale_to2d(m, v, m);
}

#endif /* cglm_affine2d_h */
