#include "draw_water.h"

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <GLUT/glut.h>
#  include <unistd.h>
#elif defined(_WIN32)
//#define FREEGLUT_STATIC
#  include <GL/freeglut.h>
#  define NOMINMAX
#  include <Windows.h>
#  undef NOMINMAX
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/gl.h>
#  include <GL/glext.h>
#  include <GL/glut.h>
#endif

void draw_water(const float * blue, float depth, float size)
{
  glEnable(GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_LIGHTING);
  glColorMaterial( GL_FRONT, GL_EMISSION);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  // Set material
  float black[] = {0.,0.,0.,1.};
  //float grey[] = {0.80,0.80,0.80,1.};
  //float white[] = {0.95,0.95,0.95,1.};
  glMaterialfv(GL_FRONT, GL_AMBIENT, blue);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
  glMaterialfv(GL_FRONT, GL_SPECULAR, black);
  glMaterialfv(GL_FRONT, GL_EMISSION, blue);
  glMaterialf(GL_FRONT, GL_SHININESS,0);

  glColor4fv(blue);
  glPushMatrix();
  glTranslated(0,depth,0);
  glTranslated(0,-size/2,0);
  glutSolidCube(size);
  glPopMatrix();
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_BLEND);
} 

