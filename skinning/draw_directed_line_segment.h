#ifndef DRAW_DIRECTED_LINE_SEGMENT_H
#define DRAW_DIRECTED_LINE_SEGMENT_H
// Draw a nice looking dot at a given point in 3d
//
// Note: expects that GL_CURRENT_COLOR is set with the desired foreground color
// 
// Inputs:
//   sx  x-coordinate of start point
//   sy  y-coordinate of start point
//   sz  z-coordinate of start point
//   dx  x-coordinate of destination point
//   dy  y-coordinate of destination point
//   dz  z-coordinate of destination point
//   t  outer-most thickness of line segment
inline void draw_directed_line_segment(
  double sx,
  double sy,
  double sz,
  double dx,
  double dy,
  double dz,
  double t = 8);

// Implementation

#if __APPLE__
#  include <OpenGL/gl.h>
#else
#  ifdef _WIN32
#    define NOMINMAX
#    include <Windows.h>
#    undef NOMINMAX
#  endif
#  include <GL/gl.h>
#endif

inline void draw_directed_line_segment(
  double sx,
  double sy,
  double sz,
  double dx,
  double dy,
  double dz,
  double t)
{
  // Push GL settings
  //GLboolean old_depth_test;
  //glGetBooleanv(GL_DEPTH_TEST,&old_depth_test);
  GLboolean old_lighting;
  glGetBooleanv(GL_LIGHTING,&old_lighting);
  GLboolean old_line_stipple;
  glGetBooleanv(GL_LINE_STIPPLE,&old_line_stipple);

  //glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_LINE_STIPPLE);

  // get current color
  float color[4];
  glGetFloatv(GL_CURRENT_COLOR,color);

  // White outline
  glColor4f(1,1,1,color[3]);
  glLineWidth(t);
  glBegin(GL_LINES);
  glVertex3d(sx,sy,sz);
  glVertex3d(dx,dy,dz);
  glEnd();
  // Black outline
  glColor4f(0,0,0,color[3]);
  glLineWidth(t-2);
  glBegin(GL_LINES);
  glVertex3d(sx,sy,sz);
  glVertex3d(dx,dy,dz);
  glEnd();
  // Foreground
  glLineWidth(t-4);
  glBegin(GL_LINES);
  // Inverse
  //glColor4f(1-color[0],1-color[1],1-color[2],color[3]);
  glColor4f(0.9*color[0],0.9*color[1],0.9*color[2],color[3]);
  glVertex3d(sx,sy,sz);
  //glColor4fv(color);
  glColor4f(1-(0.9*(1-color[0])),1-(0.9*(1-color[1])),1-(0.9*(1-color[2])),color[3]);
  glVertex3d(dx,dy,dz);
  glEnd();

  // reset color
  glColor4fv(color);

  // Pop GL settings
  if(old_line_stipple) glEnable(GL_LINE_STIPPLE);
  if(old_lighting) glEnable(GL_LIGHTING);
  //if(old_depth_test) glEnable(GL_DEPTH_TEST);
}

#endif
