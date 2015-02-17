#include "render_spheres.h"
#include "collision_spheres.h"

// OpenGL related
#ifdef __APPLE__
#  include <unistd.h>
#  include <OpenGL/gl.h>
// For functions like gluLookAt
#  include <OpenGL/glu.h>
// For glutSolidTeapot, maybe now this can be removed?
#  include <GLUT/glut.h>
#else //__APPLE__
#  include <GL/glu.h>
#  ifdef _WIN32
#    define NOMINMAX
#    include <Windows.h>
#    undef NOMINMAX
//#define FREEGLUT_STATIC
#    include <GL/freeglut.h>
#  else
#    include <GL/glut.h>
#  endif
#endif

const int slices = 16;
const int stacks = 16;

//this is probably going to be pretty slow, but I don't really care right now...
//We can show the spheres and then turn them off during the animation?

void render_sphere_tree(Spheres *root, bool is_deformed)
{
    if(root->left == NULL && root->right == NULL)
    {   
        Sphere cur_s = root->sphere;
        if(is_deformed)
            cur_s = root->deformed_sphere;
        
        if(root->is_colliding)
        {
            glColor4d(1,0,0,0.2);
        } 
        else 
        {
            glColor4d(0,1,0,0.2);
        }
        
        glPushMatrix();
        glTranslated(cur_s.center[0], cur_s.center[1], cur_s.center[2]);        
        glutSolidSphere(cur_s.radius, slices, stacks);
        glPopMatrix(); 

        return;
    }
    
    render_sphere_tree(root->left, is_deformed);
    render_sphere_tree(root->right, is_deformed);
}

void render_spheres(Spheres &S)
{
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  GLboolean old_lighting;
  glGetBooleanv(GL_LIGHTING,&old_lighting);
  glDisable(GL_LIGHTING);

    glEnable (GL_BLEND); 
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4d(0,1,0,0.2);
    
    render_sphere_tree(&S, false);
    glDisable (GL_BLEND);

  if(old_lighting) glEnable(GL_LIGHTING);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void render_deformed_spheres(Spheres &S)
{
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  GLboolean old_lighting;
  glGetBooleanv(GL_LIGHTING,&old_lighting);
  glDisable(GL_LIGHTING);

    glEnable (GL_BLEND); 
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4d(0,1,0,0.2);  
    
    render_sphere_tree(&S, true);
    glDisable (GL_BLEND);
  if(old_lighting) glEnable(GL_LIGHTING);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
