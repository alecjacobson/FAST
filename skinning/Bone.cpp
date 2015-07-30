#include <algorithm>
#include <cstdio>
#include <iostream>
#include <igl/verbose.h>
#include <igl/PI.h>
#include <igl/opengl2/project.h>
#include <igl/opengl2/unproject.h>
#include <igl/opengl2/draw_point.h>
#include <igl/opengl2/view_axis.h>
#include <igl/opengl2/up_axis.h>
#include <igl/opengl2/right_axis.h>

#include "Bone.h"
#include "Skeleton.h"
#include "Skeleton.cpp"
#include "draw_directed_line_segment.h"
#include "inside_point.h"
#include "inside_line_segment.h"

#if __APPLE__
#  include <OpenGL/gl.h>
#else
#  ifdef _WIN32
#    include <Windows.h>
#  endif
#  include <GL/gl.h>
#endif

using namespace std;
using namespace igl;
using namespace igl::opengl2;

static double POINT_COLOR[3] = {239./255.,213./255.,46./255.};
static double FREE_POINT_COLOR[3] = {113./255.,239./255.,46./255.};
static double FIXED_POINT_COLOR[3] = {239./255.,113./255.,46./255.};
static double LINEAR_POINT_COLOR[3] = {253./255.,130./255.,244./255.};
//static double ORPHAN_ROOT_POINT_COLOR[3] = {113./255.,113./255.,113./255.};
//static double TAIL_POINT_COLOR[3] = {200./255.,200./255.,200./255.};
static double HOVER_COLOR_OFFSET[3] = {-0.1,-0.1,-0.1};
static double DIRECTED_LINE_SEGMENT_COLOR[3] = {106./255.,106./255.,255./255.};
static double SELECTED_DIRECTED_LINE_SEGMENT_COLOR[3] = {
  DIRECTED_LINE_SEGMENT_COLOR[0] - 0.5,
  DIRECTED_LINE_SEGMENT_COLOR[1] - 0.5,
  DIRECTED_LINE_SEGMENT_COLOR[2] - 0.5};

static const double BONE_WI_UNSET = -3;
static const double BONE_POINT_RADIUS = 7;
static const double BONE_DIRECTED_LINE_SEGMENT_WIDTH = 8;

static double DRAG_LINE_GUIDE_COLOR[3] = {120./255.,113./255.,113./255.};
static double DRAG_LINE_COLOR[3] = {239./255.,46./255.,46./255.};

Bone::Bone(
  Skeleton<Bone> * skel_,
  Bone * parent_,
  const Vec3 & offset_):
    //public
    offset(offset_),
    rotation(Quat(1,0,0,0)),
    translation(Vec3(0,0,0)),
    stretch(1.0),
    twist(0),
    is_line_segment_selected(false),
    is_line_segment_hover(false),
    is_tip_selected(false),
    is_tip_hover(false),
    tip_dof_type(DOF_TYPE_FIXED_POSITION),
    last_T(Tform3::Identity()),
    skel(skel_),
    // private
    parent(NULL),
    wi(BONE_WI_UNSET),
    dragging_rotation(false)
{
  if(parent_!=NULL)
  {
    set_parent(parent_);
  }
}

Bone::Bone(const Bone * that):
  parent(NULL)
{
  skel = that->skel; 
  // Copy everything that is safe to copy
  offset = that->offset;
  rotation = that->rotation;
  translation = that->translation;
  stretch = that->stretch;
  twist = that->twist;
  is_line_segment_selected = that->is_line_segment_selected;
  is_line_segment_hover = that->is_line_segment_hover;
  is_tip_selected = that->is_tip_selected;
  is_tip_hover = that->is_tip_hover;
  tip_dof_type = that->tip_dof_type;
  last_T = that->last_T;
  wi = that->wi;
  dragging_rotation = that->dragging_rotation;
  // from widget
  down_x = that->down_x;
  down_y = that->down_y;
  last_x = that->last_x;
  last_y = that->last_y;
}

Bone::~Bone()
{
  // Tell parent that I'm dead
  if(parent != NULL)
  {
    parent->children.erase(
      remove(parent->children.begin(), parent->children.end(), this), 
      parent->children.end());
  }
  // Delete children (assuming they will tell me that they died)
  while(children.size() > 0)
  {
    delete (*children.begin());
  }
  //verbose("Bone destroyed\n");
}

Bone * Bone::set_wi(int wi)
{
  assert(wi != BONE_WI_UNSET);
  this->wi = wi;
  return this;
}

int Bone::get_wi() const
{
  return wi;
}

bool Bone::wi_is_set() const
{
  return wi != BONE_WI_UNSET;
}

Bone * Bone::set_parent(Bone * parent)
{
  // Don't make roots this way
  assert(parent != NULL);
  // Only let this happen once!
  assert(this->parent == NULL);
  this->parent = parent;
  // Tell parent she has just given birth
  this->parent->children.push_back(this);

  return this;
}

const Bone * Bone::get_parent() const
{
  return this->parent;
}

const std::vector<Bone*> & Bone::get_children() const
{
  return children;
}

bool Bone::is_root() const
{
  return parent==NULL;
}

void Bone::reset()
{
  translation = Vec3(0,0,0);
  rotation = Quat(1,0,0,0);
  stretch = 1.0;
  twist = 0.0;
  last_T = Tform3::Identity();
}

bool Bone::down(
  int x,
  int y, 
  bool /*right_click*/, 
  bool shift_down, 
  bool control_down, 
  bool /*meta_down*/)
{
  // Keep track of click position
  down_x = x;
  down_y = y;
  last_x = x;
  last_y = y;
  // reset to false on each mouse down
  is_tip_hover = false;
  is_line_segment_hover = false;

  // Keep track of what used to be selected
  was_tip_selected = is_tip_selected;
  was_line_segment_selected = is_line_segment_selected;

  // Check endpoint
  Vec3 d = tip_as_drawn();
  bool in_tip = inside_point(x,y,d[0],d[1],d[2],BONE_POINT_RADIUS);
  bool in_line_seg = false;

  if(skel->get_editing() && in_tip && shift_down)
  {
    // Create new child
    Bone * b = new Bone(this);
    b->set_parent(this);
    // Set offset to mouse position minus parent
    // unproject back to 3d
    //// Get screen position of tip
    //Vec3 s_d;
    //project(d[0],d[1],d[2],&s_d[0],&s_d[1],&s_d[2]);
    //Vec3 mouse;
    //unproject(x,y,s_d[2],&mouse[0],&mouse[1],&mouse[2]);
    b->offset = Vec3(0,0,0);
    // Mark child as selected
    b->is_tip_selected = true;
    b->is_selected = true;
    b->is_down = true;
    // Make this as not selected
    is_tip_selected = false;
    is_selected = false;
    is_down = false;
    return true;
  }
  // Check bone segment
  // Don't check roots and don't check if we just found out that we're in the
  // tip
  if(!is_root() && !in_tip)
  {
    Vec3 s = tail_as_drawn();

    if(!inside_point(x,y,s[0],s[1],s[2],BONE_POINT_RADIUS))
    {
      in_line_seg = inside_line_segment(
        x,y,
        s[0],s[1],s[2],
        d[0],d[1],d[2],
        BONE_DIRECTED_LINE_SEGMENT_WIDTH);
    }
  }

  is_tip_selected= 
    (in_tip ? 
       (control_down ? !is_tip_selected : true) : 
       control_down&&is_tip_selected);
  is_line_segment_selected = 
    (in_line_seg ? 
      (control_down ? !is_line_segment_selected : true) : 
      control_down&&is_line_segment_selected);

  is_selected = is_tip_selected || is_line_segment_selected;
  is_down = in_tip || in_line_seg;
  return is_down;
}

bool Bone::up(
  int /*x*/,
  int /*y*/, 
  bool /*right_click*/, 
  bool /*shift_down*/, 
  bool /*control_down*/, 
  bool /*meta_down*/)
{
  is_down = false;
  dragging_rotation = false;
  return false;
}

bool Bone::drag(
  int x,
  int y, 
  bool right_click, 
  bool /*shift_down*/, 
  bool /*control_down*/, 
  bool meta_down)
{
  if(is_selected)
  {
    if(right_click)
    {
      dragging_rotation = true;
      if(is_tip_selected)
      {
        Vec3 axis;
        view_axis(&axis[0],&axis[1],&axis[2]);
        switch(skel->view_vector)
        {
          case VIEW_VECTOR:
            view_axis(&axis[0],&axis[1],&axis[2]);
            break;
          case UP_VECTOR:
            up_axis(&axis[0],&axis[1],&axis[2]);
            break;
          case RIGHT_VECTOR:
            right_axis(&axis[0],&axis[1],&axis[2]);
            break;
        }
        // Normalize the axis
        axis.normalize();
        double angle = -2.0*PI*360.0*( (x-last_x)/ 400.0)/360.0;
        // Get change in rotation as quaternion
        Quat delta_rot = Quat(Eigen::AngleAxis<double>(angle,axis)) ;
        // Offset *ALL* rotations so that rotations are about the parent (self in
        // case of roots)
        Vec3 tail = offset;
        if(!is_root())
        {
          tail = parent->rest_tip();
        }
        if(skel->get_editing())
        {
          Tform3 A(Tform3::Identity());
          A.rotate(delta_rot);
          if(!is_root())
          {
            // rotate my offset, if I'm not a root
            offset = A*offset;
          }
          // rotate offsets of descendents
          apply_affine_to_kin(A);
        }else
        {
          // old translation
          Vec3 ot = (rotation*(translation + tail))-tail;
          // update rotation
          //rotation = rotation * Quat(Eigen::AngleAxis<double>(angle,axis));
          rotation = delta_rot*rotation;
          // center rotations around offset point for roots
          // update translation
          translation = ((rotation.inverse() * (tail+ot)) - (tail));
        }
      }else if(is_line_segment_selected)
      {
        // twist about bone
        assert(!is_root());
        Vec3 d = rest_tip();
        Vec3 s = parent->rest_tip();
        // bone axis
        Vec3 axis = s-d;
        // Normalize the axis
        axis.normalize();
        double angle = -2.0*PI*360.0*( (x-last_x)/ 400.0)/360.0;
        // Offset *ALL* rotations so that rotations are about the parent (self in
        // case of roots)
        Vec3 tail = offset;
        Quat delta_rot = Quat(Eigen::AngleAxis<double>(angle,axis));
        if(!is_root())
        {
          tail = parent->rest_tip();
        }
        if(skel->get_editing())
        {
          // rotate offsets of descendents
          Tform3 A(Tform3::Identity());
          A.rotate(delta_rot);
          if(!is_root())
          {
            // rotate my offset, if I'm not a root
            offset = A*offset;
          }
          // rotate offsets of descendents
          apply_affine_to_kin(A);
        }else
        {
          // old translation
          Vec3 ot = (rotation*(translation + tail))-tail;
          // update rotation
          rotation = rotation * Quat(Eigen::AngleAxis<double>(angle,axis));
          // center rotations around offset point for roots
          // update translation
          translation = ((rotation.inverse() * (tail+ot)) - (tail));
        }
      }else
      {
        assert(false);
      }
    }else
    {
      // Get position of tip
      Vec3 t = tip_as_drawn();
      Vec3 tc = t;
      // Get screen position of tip
      Vec3 s_t;
      project(t[0],t[1],t[2],&s_t[0],&s_t[1],&s_t[2]);
      // Move tip in screen space according to mouse move since last
      s_t[0] += (x-last_x);
      s_t[1] += (y-last_y);
      // unproject back to 3d
      unproject(s_t[0],s_t[1],s_t[2],&t[0],&t[1],&t[2]);
      // affective translation, this is the translation we want to show up in
      // affine()
      Vec3 at = (t-tc);
      Vec3 p_at(0,0,0);
      if(parent != NULL)
      {
        p_at = parent->affine().translation();
      }
      //Vec3 trans_update = (rotation.inverse() * orientation()).inverse() * at;
      Vec3 trans_update = orientation().inverse() * at;
      if(skel->get_editing())
      {
        if(is_line_segment_selected)
        {
          // also move parent (which moves me)
          parent->offset += trans_update;
        }else
        {
          // translate my offset
          offset += trans_update;
        }
        if(!meta_down)
        {
          // but untranslate my children
          Tform3 A(Tform3::Identity());
          A.translate(-trans_update);
          apply_affine_to_kin(A);
        }
      }else
      {
        // set translation to put rest tip at new tip position
        translation += trans_update;
      }
    }
  }
  //// New tip
  //Vec3 nt = tip_as_drawn();
  // keep track of last mouse position
  last_x = x;
  last_y = y;
  return is_selected;
}

bool Bone::move(
  int x,
  int y, 
  bool /*shift_down*/, 
  bool /*control_down*/, 
  bool /*meta_down*/)
{
  assert(!is_down);
  // reset to false on each mouse move
  is_tip_hover = false;
  is_line_segment_hover = false;

  // Check endpoint
  Vec3 d = tip_as_drawn();
  bool in_tip = inside_point(x,y,d[0],d[1],d[2],BONE_POINT_RADIUS);
  is_tip_hover = in_tip;

  // Check bone segment
  bool in_line_seg = false;
  if(!is_root() && !in_tip)
  {
    Vec3 s = tail_as_drawn();

    if(!inside_point(x,y,s[0],s[1],s[2],BONE_POINT_RADIUS))
    {
      in_line_seg = inside_line_segment(
        x,y,
        s[0],s[1],s[2],
        d[0],d[1],d[2],
        BONE_DIRECTED_LINE_SEGMENT_WIDTH);
    }
  }
  is_line_segment_hover = in_line_seg;

  is_hover = is_tip_hover || is_line_segment_hover;

  return false;
}

bool Bone::inside(
  int x,
  int y)
{
  // Check tip
  // get tip
  Vec3 d = tip_as_drawn();
  bool in_tip = inside_point(x,y,d[0],d[1],d[2],BONE_POINT_RADIUS);
  // Check bone segment
  bool in_line_seg = false;
  if(!is_root() && !in_tip)
  {
    Vec3 s = tail_as_drawn();

    if(!inside_point(x,y,s[0],s[1],s[2],BONE_POINT_RADIUS))
    {
      in_line_seg = inside_line_segment(
        x,y,
        s[0],s[1],s[2],
        d[0],d[1],d[2],
        BONE_DIRECTED_LINE_SEGMENT_WIDTH);
    }
  }
  return in_tip || in_line_seg;
}

void Bone::draw()
{
  if(parent != NULL)
  {
    Vec3 d = tip_as_drawn();
    Vec3 s = tail_as_drawn();

    double lcolor[3];
    line_segment_color(lcolor);

    // Draw directed line segment "bone"
    glColor3d(lcolor[0],lcolor[1],lcolor[2]);
    draw_directed_line_segment(
      s[0],s[1],s[2],
      d[0],d[1],d[2],
      BONE_DIRECTED_LINE_SEGMENT_WIDTH);
    // Draw tail (might not be at same position as parent's tip)
    double pcolor[3];
    this->parent->tip_color(pcolor);
    // Increase brightness and average with gray
    pcolor[0] = 0.8*(1.0-0.2*(1.0-pcolor[0]))+0.2*0.5;
    pcolor[1] = 0.8*(1.0-0.2*(1.0-pcolor[1]))+0.2*0.5;
    pcolor[2] = 0.8*(1.0-0.2*(1.0-pcolor[2]))+0.2*0.5;
    glColor3d(pcolor[0],pcolor[1],pcolor[2]);
    igl::opengl2::draw_point(s[0],s[1],s[2],BONE_POINT_RADIUS,false);
    // Draw parent's tip again to make sure it shows up on top
    this->parent->draw_tip();
  }

  this->draw_tip();

  // If user is dragging out a rotation then draw a red line from the mouse
  // down location to the current mouse location
  if(skel->draw_drag_rotations && dragging_rotation)
  {
    draw_drag_line();
  }
}

void Bone::draw_tip() const
{ 
  Vec3 d = tip_as_drawn();
  if(skel->average_children_tails_to_draw_non_weighted_roots||wi>=0)
  {
    // get foreground color for points
    double pcolor[3];
    tip_color(pcolor);
    glColor3d(pcolor[0],pcolor[1],pcolor[2]);
    igl::opengl2::draw_point(d[0],d[1],d[2],BONE_POINT_RADIUS,is_tip_selected);
  }
}

void Bone::tip_color(double pcolor[3]) const
{
  switch(tip_dof_type)
  {
    case DOF_TYPE_FIXED_ALL:
      copy(FIXED_POINT_COLOR,FIXED_POINT_COLOR+3,pcolor);
      break;
    case DOF_TYPE_FREE:
      copy(FREE_POINT_COLOR,FREE_POINT_COLOR+3,pcolor);
      break;
    case DOF_TYPE_FIXED_LINEAR:
      copy(LINEAR_POINT_COLOR,LINEAR_POINT_COLOR+3,pcolor);
      break;
    case DOF_TYPE_FIXED_POSITION:
    default:
      copy(POINT_COLOR,POINT_COLOR+3,pcolor);
  }
  if(is_tip_hover && !is_down)
  {
    pcolor[0]+=HOVER_COLOR_OFFSET[0];
    pcolor[1]+=HOVER_COLOR_OFFSET[1];
    pcolor[2]+=HOVER_COLOR_OFFSET[2];
  }else if(is_tip_selected)
  {
    //copy(SELECTED_POINT_COLOR,SELECTED_POINT_COLOR+3,pcolor);
  }
}

void Bone::line_segment_color(double lcolor[3]) const
{
  // Get line segment color
  copy(DIRECTED_LINE_SEGMENT_COLOR,DIRECTED_LINE_SEGMENT_COLOR+3,lcolor);
  if(is_line_segment_hover && !is_down)
  {
    lcolor[0]+=HOVER_COLOR_OFFSET[0];
    lcolor[1]+=HOVER_COLOR_OFFSET[1];
    lcolor[2]+=HOVER_COLOR_OFFSET[2];
  }else if(is_line_segment_selected)
  {
    copy(
      SELECTED_DIRECTED_LINE_SEGMENT_COLOR,
      SELECTED_DIRECTED_LINE_SEGMENT_COLOR+3,
      lcolor);
  }
}

void Bone::draw_drag_line() const
{
  GLboolean old_lighting;
  glGetBooleanv(GL_LIGHTING,&old_lighting);
  GLboolean old_line_stipple;
  glGetBooleanv(GL_LINE_STIPPLE,&old_line_stipple);

  glDisable(GL_LIGHTING);
  glDisable(GL_LINE_STIPPLE);

  // Draw a line from mouse down to mouse last
  // Set offset to mouse position minus parent
  // unproject back to 3d
  // Get screen position of tip
  Vec3 d = tip_as_drawn();
  Vec3 s_d;
  project(d[0],d[1],d[2],&s_d[0],&s_d[1],&s_d[2]);
  // Last mouse location unprojected to 3d
  Vec3 last;
  unproject(last_x,last_y,s_d[2],&last[0],&last[1],&last[2]);
  // Down mouse location unprojected to 3d
  Vec3 down;
  unproject(down_x,down_y,s_d[2],&down[0],&down[1],&down[2]);
  // (last_x,down_y) unprojected to 3d
  Vec3 last_x_only;
  unproject(last_x,down_y,s_d[2],&last_x_only[0],&last_x_only[1],&last_x_only[2]);

  glLineWidth(1);
  glEnable(GL_LINE_STIPPLE);
  glLineStipple(1,0xAAAA);
  glColor3dv(DRAG_LINE_GUIDE_COLOR);
  glBegin(GL_LINES);
  glVertex3d(down[0],down[1],down[2]);
  glVertex3d(last[0],last[1],last[2]);
  glVertex3d(last[0],last[1],last[2]);
  glVertex3d(last_x_only[0],last_x_only[1],last_x_only[2]);
  glEnd();

  glLineWidth(2);
  glColor3dv(DRAG_LINE_COLOR);
  glDisable(GL_LINE_STIPPLE);
  glBegin(GL_LINES);
  glVertex3d(down[0],down[1],down[2]);
  glVertex3d(last_x_only[0],last_x_only[1],last_x_only[2]);
  glEnd();

  if(old_line_stipple) glEnable(GL_LINE_STIPPLE);
  if(old_lighting) glEnable(GL_LIGHTING);
}

Quat Bone::orientation() const
{
  assert(stretch == 1);
  assert(twist == 0);
  Quat p_orientation;
  if(parent == NULL)
  {
    // Root base case
    // Identity rotation
    p_orientation = Quat(1,0,0,0);
  }else
  {
    p_orientation = parent->orientation();
  }
  // Compose with this rotation
  return p_orientation * rotation;
}

Vec3 Bone::tip(
  const bool according_to_last_T,
  const bool average_children_tails) const
{
  assert(stretch == 1);
  assert(twist == 0);
  if(average_children_tails && children.size() > 0)
  {
    Vec3 t(0,0,0);
    vector<Bone*> children = get_children();
    for(
      vector<Bone*>::iterator cit = children.begin(); 
      cit != children.end(); 
      cit++)
    {
      assert((*cit)->parent == this);
      t += (*cit)->tail(according_to_last_T);
    }
    t /= children.size();
    return t;
  }else
  {
    if(according_to_last_T)
    {
      return last_T * rest_tip();
    }else
    {
      return affine() * rest_tip();
    }
  }
}

Vec3 Bone::tip_as_drawn() const
{
  bool according_to_last_T = skel->draw_according_to_last_T;
  bool average_children_tails = 
     skel->average_children_tails_to_draw_non_weighted_roots && 
     is_root() &&
     wi<0;
   return tip(according_to_last_T,average_children_tails);
}

Vec3 Bone::tail(const bool according_to_last_T) const
{
  assert(stretch == 1);
  assert(twist == 0);
  assert(parent!=NULL);
  if(according_to_last_T)
  {
    return last_T * parent->rest_tip();
  }else
  {
    return affine() * parent->rest_tip();
  }
}

Vec3 Bone::tail_as_drawn() const
{
  if(skel->draw_connected_to_parent)
  {
    return parent->tip_as_drawn();
  }
  return tail(skel->draw_according_to_last_T);
}

Vec3 Bone::rest_tip() const
{
  Vec3 p_rest_tip;
  if(parent == NULL)
  {
    // Root base case
    p_rest_tip = Vec3(0,0,0);
  }else
  {
    p_rest_tip = parent->rest_tip();
  }
  return p_rest_tip + offset;
}

Vec3 Bone::rest_tail() const
{
  assert(parent!=NULL);
  return parent->rest_tip();
}

Tform3 Bone::affine() const
{
  assert(stretch == 1);
  assert(twist == 0);
  Tform3 p_affine;
  if(parent == NULL)
  {
    p_affine.setIdentity();
  }else
  {
    p_affine = parent->affine();
  }
  return p_affine.rotate(rotation).translate(translation);
}

Quat Bone::rotated_frame() const
{
  assert(twist == 0);
  Quat q(1,0,0,0);
  if(parent != NULL)
  {
    q = parent->rotated_frame();
  }
  return q*rotation;
}

void Bone::apply_affine_to_kin(const Tform3 & A)
{
  vector<Bone*> children = get_children();
  Tform3 B = A;
  B.translation() = Vec3(0,0,0);
  for(
    vector<Bone*>::iterator cit = children.begin(); 
    cit != children.end(); 
    cit++)
  {
    (*cit)->offset = A * (*cit)->offset;
    // recursive call
    (*cit)->apply_affine_to_kin(B);
  }
}

Bone * Bone::set_is_tip_selected(const bool v)
{
  is_tip_selected = v;
  is_selected = is_tip_selected || is_line_segment_selected;
  return this;
}

Bone * Bone::set_is_line_segment_selected(const bool v)
{
  // roots don't have line segments
  assert(!v||!is_root());
  is_line_segment_selected = v;
  is_selected = is_tip_selected || is_line_segment_selected;
  return this;
}

Bone * Bone::reset_is_selected()
{
  is_tip_selected = was_tip_selected;
  assert(!was_line_segment_selected||!is_root());
  is_line_segment_selected = was_line_segment_selected;
  is_selected = is_tip_selected || is_line_segment_selected;
  return this;
}

void Bone::print() const
{
  printf("offset: %g %g %g\n",offset[0],offset[1],offset[2]);
  printf("translation: %g %g %g\n",translation[0],translation[1],translation[2]);
  printf("rotation: %g %g %g %g\n",rotation.w(),rotation.x(),rotation.y(),rotation.z());
  printf("stretch: %g\n",stretch);
  printf("twist: %g\n",twist);
  printf("\n");
}
