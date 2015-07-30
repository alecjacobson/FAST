#ifndef INSIDE_LINE_SEGMENT_H
#define INSIDE_LINE_SEGMENT_H
// Determine if a screen space position (s_z, s_y) is inside a given
// "line segment" defined by the projection of the point (sx,sy,sz) to
// (dx,dy,dz) of screen space radius s_r
// Inputs:
//   s_qx  x-coordinate of screen space query point
//   s_qy  y-coordinate of screen space query point
//   sx  x-coordinate of source point in 3D
//   sy  y-coordinate of source point in 3D
//   sz  z-coordinate of source point in 3D
//   dx  x-coordinate of destination point in 3D
//   dy  y-coordinate of destination point in 3D
//   dz  z-coordinate of destination point in 3D
//   s_r  radius of point 
inline bool inside_line_segment(
  const double s_qx,
  const double s_qy,
  const double sx,
  const double sy,
  const double sz,
  const double dx,
  const double dy,
  const double dz,
  double s_r);

// Implementation
#include <igl/opengl2/project.h>
#include <igl/point_in_circle.h>
#include <igl/project_to_line.h>

inline bool inside_line_segment(
  const double s_qx,
  const double s_qy,
  const double sx,
  const double sy,
  const double sz,
  const double dx,
  const double dy,
  const double dz,
  double s_r)
{
  // Get screen position of (sx,sy,sz) and (dx,dy,dz)
  double s_sx, s_sy, s_sz;
  igl::opengl2::project(sx,sy,sz,&s_sx,&s_sy,&s_sz);
  double s_dx, s_dy, s_dz;
  igl::opengl2::project(dx,dy,dz,&s_dx,&s_dy,&s_dz);
  // Test if (s_qx, s_y) is in circle with radius s_r, centered
  // at (s_x,s_y,s_z)
  if(igl::point_in_circle(s_qx,s_qy,s_sx,s_sy,s_r)) return true;
  if(igl::point_in_circle(s_qx,s_qy,s_dx,s_dy,s_r)) return true;
  double t,sqrd;
  igl::project_to_line(
    s_qx,s_qy,0.0,
    s_sx,s_sy,0.0,
    s_dx,s_dy,0.0,
    t,sqrd);
  return (t<=1 && t>=0) && (sqrd <= s_r*s_r);
}

#endif

