#ifndef INSIDE_POINT_H
#define INSIDE_POINT_H
// Determine if a screen space position (s_z, s_y) is inside a given
// "point" defined by the projection of the point (x,y,z) onto the screen as a
// circle of screen space radius s_r
// Inputs:
//   s_qx  x-coordinate of screen space query point
//   s_qy  y-coordinate of screen space query point
//   x  x-coordinate of point in 3D
//   y  y-coordinate of point in 3D
//   z  z-coordinate of point in 3D
//   s_r  radius of point 
inline bool inside_point(
  const double s_qx,
  const double s_qy,
  const double x,
  const double y,
  const double z,
  double s_r);

// Implementation

#include <igl/point_in_circle.h>
#include <igl/opengl2/project.h>
#include <igl/opengl/report_gl_error.h>
inline bool inside_point(
  const double s_qx,
  const double s_qy,
  const double x,
  const double y,
  const double z,
  double s_r)
{
  using namespace igl::opengl2;
  using namespace igl::opengl;
  using namespace igl;
  // Get screen position of (x,y,z)
  double s_x, s_y, s_z;
  igl::opengl2::project(x,y,z,&s_x,&s_y,&s_z);
  igl::opengl::report_gl_error();

//#ifdef EXTREME_VERBOSE
//  verbose("project(%g,%g,%g) --> %g %g %g\n",x,y,z,s_x,s_y,s_z);
//#endif
  // Test if (s_qx, s_y) is in circle with radius s_r, centered
  // at (s_x,s_y,s_z)
  return igl::point_in_circle(s_qx,s_qy,s_x,s_y,s_r);
}

#endif
