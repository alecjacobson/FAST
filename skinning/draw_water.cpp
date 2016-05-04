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
  // http://stackoverflow.com/a/327135/148668
  const auto & drawBox = [](GLfloat size, GLenum type)
  {
    static GLfloat n[6][3] =
    {
      {-1.0, 0.0, 0.0},
      {0.0, 1.0, 0.0},
      {1.0, 0.0, 0.0},
      {0.0, -1.0, 0.0},
      {0.0, 0.0, 1.0},
      {0.0, 0.0, -1.0}
    };
    static GLint faces[6][4] =
    {
      {0, 1, 2, 3},
      {3, 2, 6, 7},
      {7, 6, 5, 4},
      {4, 5, 1, 0},
      {5, 6, 2, 1},
      {7, 4, 0, 3}
    };
    GLfloat v[8][3];
    GLint i;
  
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;
  
    for (i = 5; i >= 0; i--)
    {
      glBegin(type);
      glNormal3fv(&n[i][0]);
      glVertex3fv(&v[faces[i][0]][0]);
      glVertex3fv(&v[faces[i][1]][0]);
      glVertex3fv(&v[faces[i][2]][0]);
      glVertex3fv(&v[faces[i][3]][0]);
      glEnd();
    }
  };
  const auto & glutSolidCube = [&drawBox](GLdouble size)
  {
    drawBox(size, GL_QUADS);
  };
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

