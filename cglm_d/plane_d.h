/*!
 * @brief calculates the plane from triangles vertices (favours accuracy over speed)
 *
 * @param[in] a point 0
 * @param[in] b point 1
 * @param[in] c point 2
 * @param[out] p plane
 */

void
glmd_triangle_to_plane(double a[3], double b[3], double c[3], double p[4])
{
  double edge0[3];
  double edge1[3];
  double l;
  
  edge0[0] = b[0] - a[0];
  edge0[1] = b[1] - a[1];
  edge0[2] = b[2] - a[2];
  
  edge1[0] = c[0] - a[0];
  edge1[1] = c[1] - a[1];
  edge1[2] = c[2] - a[2];
  
  p[0] = edge0[1] * edge1[2] - edge0[2] * edge1[1];
  p[1] = edge0[2] * edge1[0] - edge0[0] * edge1[2];
  p[2] = edge0[0] * edge1[1] - edge0[1] * edge1[0];

  l = sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
  p[3] = (p[0] * a[0] + p[1] * a[1] + p[2] * a[2]) / l;
  
  p[0] = p[0] / l;
  p[1] = p[1] / l;
  p[2] = p[2] / l;
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

int 
glmd_plane_intersect(double a[4], double b[4], double c[4], double p[4])
{
  double const epsilon = 1e-8f;

  double x[3];
  double d;
  
  x[0] = a[1] * b[2] - a[2] * b[1];
  x[1] = a[2] * b[0] - a[0] * b[2];
  x[2] = a[0] * b[1] - a[1] * b[0];
  
  d = x[0] * c[0] + x[1] * c[1] + x[2] * c[2];
  
  if( d < epsilon && d > -epsilon ) return 0;
  
  p[0] = (b[1] * c[2] - b[2] * c[1]) * -a[3];
  p[1] = (b[2] * c[0] - b[0] * c[2]) * -a[3];
  p[2] = (b[0] * c[1] - b[1] * c[0]) * -a[3];
  
  p[0] += (c[1] * a[2] - c[2] * a[1]) * -b[3];
  p[1] += (c[2] * a[0] - c[0] * a[2]) * -b[3];
  p[2] += (c[0] * a[1] - c[1] * a[0]) * -b[3];

  p[0] += (a[1] * b[2] - a[2] * b[1]) * -c[3];
  p[1] += (a[2] * b[0] - a[0] * b[2]) * -c[3];
  p[2] += (a[0] * b[1] - a[1] * b[0]) * -c[3];
  
  p[0] = -p[0] / d;
  p[1] = -p[1] / d;
  p[2] = -p[2] / d;

  return 1;
}


double
glmd_plane_polarity(double p[4], double a[3])
{
	return 
	(a[0] * p[0] + a[1] * p[1] + a[2] * p[2])
	-(p[0]*p[3] * p[0] + p[1]*p[3] * p[1] + p[2]*p[3] * p[2])
	;
}
