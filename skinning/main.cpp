#include "main.h"
#include "Skinning.h"
#include <cstdio>
#include <string>
using namespace std;
#include <igl/dirname.h>
#include <igl/get_seconds_hires.h>
using namespace igl;
#include <iomanip>
#include <iostream>

#ifdef SINGLE_TBT
#include "Skeleton.h"
#include "Bone.h"
#include <PuppetReader.h>
#include <TBT.h>
#include <igl/get_seconds.h>
#include <igl/PI.h>
#include <Eigen/Geometry>
PuppetReader * pr;
#endif

#ifdef __APPLE__
#  define SHADER_DIR "./GLSL"
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

#include "CTGALoader.h"

#ifdef _WIN32
	// I use left-hand trackball... :)
	#define OUR_RIGHT_BUTTON GLUT_LEFT_BUTTON
	#define OUR_LEFT_BUTTON GLUT_RIGHT_BUTTON
#else
	#define OUR_RIGHT_BUTTON GLUT_RIGHT_BUTTON
	#define OUR_LEFT_BUTTON GLUT_LEFT_BUTTON
#endif

//#define SCREEN_CAPTURE
//#define DISPLAY_CROWD

int g_width = -1;
int g_height = -1;

// Remember which mouse button is down
int down_glutButton;

#ifdef SCREEN_CAPTURE
int g_imgCnt = 0;
double g_timerStart = 0.0;

void saveDoubleToFile(const string &fname, double number)
{
	FILE *f = fopen(fname.c_str(), "wt");
	fprintf(f, "%f", number);
	fclose(f);
}
#endif

#ifdef SINGLE_TBT
bool init_puppet()
{
  using namespace Eigen;
  using namespace std;
  pr = new PuppetReader("ignored",115200);
  return pr->is_attached();
}

const TBT * get_const_tbt(const PuppetReader & pr)
{
  if(const Node * r = pr.get_const_root())
  {
    return dynamic_cast<const TBT *>(r->get_child(0));
  }
  return NULL;
}

Bone * get_third_bone(Skeleton<Bone> & s)
{
  for(
    std::vector<Bone*>::const_iterator bit = s.roots.begin();
    bit != s.roots.end();
    bit++)
  {
    Bone * first = *bit;
    if(first != NULL && first->get_children().size()>0)
    {
      Bone * second = first->get_children().front();
      if(second != NULL && second->get_children().size()>0)
      {
        Bone * third = second->get_children().front();
        return third;
      }
    }
  }
  return NULL;
}

void sync_single_tbt(PuppetReader & pr, Skeleton<Bone> & s)
{
  using namespace Eigen;
  using namespace std;
  using namespace igl;
  if(pr.is_attached())
  {
    pr.sync();
  }
  const TBT * t = get_const_tbt(pr);
  Bone * second = get_third_bone(s);
  if(t != NULL && second != NULL)
  {
    vector<Quaterniond,aligned_allocator<Quaterniond> > dQ;
    Node::relative_rotations(pr.get_const_root(),dQ);
    assert(dQ.size() >= 2);
    Quaterniond q = 
      Quaterniond(AngleAxisd(PI/2,Vector3d(1,0,0)))*
      Quaterniond(AngleAxisd(PI/2,Vector3d(0,1,0)));
    second->rotation = 
      q*
      dQ[1]*
      q.conjugate();
  }else if(second)
  {
    second->rotation = 
      Quaterniond(AngleAxisd(0.01*get_seconds()*(2.*PI),Vector3d(0,0,1)));
  }
  Vector3d origin = second->rest_tail(); 
  second->translation = second->rotation.conjugate() * (origin) - origin;
}
#endif

void Display(void)
{
#ifdef SINGLE_TBT
  assert(skinning->skel != NULL && "Null Skeleton.");
  assert(pr != NULL && "Null device.");
  sync_single_tbt(*pr,*(skinning->skel));
#endif
  skinning->display();
  // Present frame buffer
  glutSwapBuffers();
  // Recall Display at next frame
  glutPostRedisplay();

#ifdef SCREEN_CAPTURE
  double elapsedFPS = 1.0 / (get_seconds_hires() - g_timerStart);

  stringstream padnum; padnum << setw(4) << setfill('0') << g_imgCnt;
  string imgFname = "out/image" + padnum.str() + ".tga";  
  string fpsFname = "out/fps" + padnum.str() + ".txt";  
	printf("Saved image %i\n", g_imgCnt);
  CTGALoader tgaLoader;
  tgaLoader.SaveTGAScreenShot(imgFname.c_str(), g_width, g_height);
  saveDoubleToFile(fpsFname, elapsedFPS);
  g_imgCnt++;
  g_timerStart = get_seconds_hires();
#endif
}

void Reshape(int width, int height)
{
  skinning->resize(width,height);
  g_width = width;
  g_height = height;
}

// Function called at exit
void Terminate(void)
{ 
  delete skinning;
}

void key(unsigned char key,int mouse_x, int mouse_y)
{
  using namespace std;
  if(
#ifdef WIN32
      key == 3 || 
#endif
      key == 27)
  {
    exit(0);
  }

  if(!skinning->key_down(key,mouse_x,g_height-mouse_y,false,false,false))
  {
  }
}

void mouse(int glutButton, int glutState, int mouse_x, int mouse_y)
{

  int mod = glutGetModifiers();
  bool shift_down = mod & GLUT_ACTIVE_SHIFT;
  bool meta_down = mod & GLUT_ACTIVE_ALT;
  bool control_down = mod & GLUT_ACTIVE_CTRL;

  //Hack because linux has no meta
  if(shift_down && control_down)
  {
    meta_down = true;
    shift_down = false;
    control_down = false;
  }

  down_glutButton = glutButton;

  using namespace std;
  if(glutState==1)
  {
    // Should differentiate between types of clicks
    switch(glutButton)
    {
      // Scroll
      case 4:
      case 3:
        skinning->mouse_scroll(mouse_x,mouse_y,0,(glutButton==3?0.5:-0.5));
        break;
      case OUR_RIGHT_BUTTON:
        skinning->right_mouse_up(mouse_x,g_height-mouse_y,shift_down,control_down,meta_down);
        break;
      case OUR_LEFT_BUTTON:
      default:
        skinning->mouse_up(mouse_x,g_height-mouse_y,shift_down,control_down,meta_down);
        break;
    }
  }else if(glutState==0)
  {
    switch(glutButton)
    {
      // Scroll
      case 4:
      case 3:
        skinning->mouse_scroll(mouse_x,mouse_y,0,(glutButton==3?0.5:-0.5));
        break;
      case OUR_RIGHT_BUTTON:
        skinning->right_mouse_down(mouse_x,g_height-mouse_y,shift_down,control_down,meta_down);
        break;
      case OUR_LEFT_BUTTON:
      default:
        skinning->mouse_down(mouse_x,g_height-mouse_y,shift_down,control_down,meta_down);
        break;
    }
  }
}

void mouse_drag(int mouse_x, int mouse_y)
{

#ifndef __APPLE__
  int mod = glutGetModifiers();
#else
  int mod = 0;
#endif
  bool shift_down = mod & GLUT_ACTIVE_SHIFT;
  bool meta_down = mod & GLUT_ACTIVE_ALT;
  bool control_down = mod & GLUT_ACTIVE_CTRL;
  //Hack because linux has no meta
  if(shift_down && control_down)
  {
    meta_down = true;
    shift_down = false;
    control_down = false;
  }

  if(down_glutButton == OUR_RIGHT_BUTTON)
  {
    if(!skinning->right_mouse_drag(mouse_x,g_height-mouse_y,shift_down,control_down,meta_down))
    {
    }
  }
  else
  {
    if(!skinning->mouse_drag(mouse_x,g_height-mouse_y,shift_down,control_down,meta_down))
    {
    }
  }
}

void mouse_move(int mouse_x, int mouse_y)
{

#ifndef __APPLE__
  int mod = glutGetModifiers();
#else
  int mod = 0;
#endif
  bool shift_down = mod & GLUT_ACTIVE_SHIFT;
  bool meta_down = mod & GLUT_ACTIVE_ALT;
  bool control_down = mod & GLUT_ACTIVE_CTRL;
  //Hack because linux has no meta
  if(shift_down && control_down)
  {
    meta_down = true;
    shift_down = false;
    control_down = false;
  }

  if(!skinning->mouse_move(mouse_x,g_height-mouse_y,shift_down,control_down,meta_down))
  {
  }
}


int main(int argc, char *argv[])
{
  // Make directory same as executable
  printf("getcwd(): %s\n", getcwd(NULL, 0));
  printf("cd %s\n", igl::dirname(argv[0]).c_str());
  chdir(dirname(argv[0]).c_str());
  printf("getcwd(): %s\n", getcwd(NULL, 0));

  // Initialize GLUT
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  // Anti-aliasing
  glutInitDisplayString( "rgba depth double samples>=8 ");
  glutInitWindowSize(960, 540);
  //glutInitWindowSize(1088, 612);
  //glutInitWindowSize(glutGet(GLUT_SCREEN_WIDTH),glutGet(GLUT_SCREEN_HEIGHT));
  // Center window
  glutInitWindowPosition(
      glutGet(GLUT_SCREEN_WIDTH)/2-glutGet(GLUT_INIT_WINDOW_WIDTH)/2,
      glutGet(GLUT_SCREEN_HEIGHT)/2-glutGet(GLUT_INIT_WINDOW_HEIGHT)/2);
  glutCreateWindow("SkinningGLUT");
  glutCreateMenu(NULL);
  // Set GLUT callbacks
  glutDisplayFunc(Display);
  glutReshapeFunc(Reshape);
#ifndef WIN32
  atexit(Terminate);  // this was causing funny issues in the destructor (weird string bugs)
#else
  glutWMCloseFunc(Terminate);
#endif
  
#ifdef _WIN32
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
      /* Problem: glewInit failed, something is seriously wrong. */
      fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      return 1;
  }
#endif

  skinning = new Skinning();
  if(argc > 1)
  {
    if(string(argv[1]) == "-h" || string(argv[1]) == "--help")
    {
      printf("Usage:\n  ./skinning\nor\n  ./skinning [path to obj,dmat dir]\n");
      return 1;
    }
    skinning->load(string(argv[1]));
    skinning->load_shader_pair_from_files(LBS,"GLSL/lbs.frag");
  }
#ifndef DISPLAY_CROWD
  skinning->display_crowd = false;
  skinning->crowd_x = 10;
  skinning->crowd_y = 10;
  skinning->crowd_x_off = 1.0;
  skinning->crowd_y_off = 1.0;
#endif

  // Hook up AntTweakBar to GLUT events
  glutMouseFunc(mouse);
  glutMotionFunc(mouse_drag);
  glutPassiveMotionFunc(mouse_move);
  glutKeyboardFunc(key);
#ifdef SINGLE_TBT
  init_puppet();
#endif

  // Call the GLUT main loop
  glutMainLoop();
  return 0;
}
