#include "Skeleton.h"
#include "gather_bones.h"

// IGL
#include <igl/unproject_to_zero_plane.h>

// OpenGL related
#ifdef __APPLE__
#  include <OpenGL/gl.h>
// For functions like gluLookAt
#  include <OpenGL/glu.h>
#else //__APPLE__
#  include <GL/glu.h>
#  ifdef _WIN32
#    define NOMINMAX
#    include <Windows.h>
#    undef NOMINMAX
#  endif
#endif

// STL
#include <algorithm>
#include <cstdio>
#include <iostream>

// Predicate to return bone is_selected field
template <typename BoneType>
static bool is_bone_selected(BoneType * b)
{
  return b->is_selected;
}

// No-op default 
template <typename BoneType>
static void no_op(const Skeleton<BoneType> &)
{
  return;
}

template <typename BoneType>
Skeleton<BoneType>::Skeleton(): 
  bones_on_top(true),
  after_set_editing(no_op<BoneType>), 
  draw_drag_rotations(true), 
  lock_mouse_new(false)
{
}

template <typename BoneType>
Skeleton<BoneType>::~Skeleton()
{
}

template <typename BoneType>
bool Skeleton<BoneType>::down(
  int x,
  int y, 
  bool right_click, 
  bool shift_down, 
  bool control_down, 
  bool meta_down)
{
  bool bone_down = false;
  bool reset_selection = true;
  using namespace std;
  using namespace igl;
  // get bones
  vector<BoneType *> B = gather_bones(roots);

  // check bones
  for(typename vector<BoneType *>::iterator bit = B.begin();bit != B.end();bit++)
  {
    BoneType * b = (*bit);
    if(b->down(x,y,right_click,shift_down,control_down,meta_down))
    {
      bone_down = true;
      reset_selection &=
        ((b->was_tip_selected && b->is_tip_selected) || 
        (b->was_line_segment_selected && b->is_line_segment_selected));
    }
  }

  // If we're editing bones and meta is not down then place a new bone
  if(!bone_down && editing && !meta_down)
  {
    if(lock_mouse_new)
    {
      cerr<<"Warning: Mouse editing locked"<<endl;
    }else
    {
      Vec3 off;
      unproject_to_zero_plane(x,y, &off[0], &off[1], &off[2]);
      // Add as root
      using namespace std;
      cout<<"Adding new bone"<<endl;
      BoneType * b = *roots.insert(roots.end(),new BoneType(this,NULL,off));
      // Treat as if we just click on it
      if(b->down(x,y,right_click,shift_down,control_down,meta_down))
      {
        bone_down = true;
        // Always trash the current the selection in the case of adding new bone
        reset_selection = false;
      }else
      {
        assert(false);
      }
    }
  }

  if(bone_down && reset_selection)
  {
    // Regather bones since the list may have changed
    B = gather_bones(roots);
    for_each(B.begin(),B.end(),mem_fun(&BoneType::reset_is_selected));
  }
  

  return bone_down;
}

template <typename BoneType>
bool Skeleton<BoneType>::up(
  int x,
  int y, 
  bool right_click, 
  bool shift_down, 
  bool control_down, 
  bool meta_down)
{
  using namespace std;
  bool bone_up = false;
  // get bones
  vector<BoneType *> B = gather_bones(roots);
  // check bones
  for(typename vector<BoneType *>::iterator bit = B.begin();bit != B.end();bit++)
  {
    BoneType * b = (*bit);
    if(b->up(x,y,right_click,shift_down,control_down,meta_down))
    {
      bone_up= true;
    }
  }
  return bone_up;
}

template <typename BoneType>
bool Skeleton<BoneType>::drag(
  int x,
  int y, 
  bool right_click, 
  bool shift_down, 
  bool control_down, 
  bool meta_down)
{
  using namespace std;
  bool bone_drag = false;
  // get bones
  vector<BoneType *> B = gather_bones(roots);
  // check bones
  for(typename vector<BoneType *>::iterator bit = B.begin();bit != B.end();bit++)
  {
    BoneType * b = (*bit);
    if(b->drag(x,y,right_click,shift_down,control_down,meta_down))
    {
      bone_drag = true;
    }
  }
  return bone_drag;
}

template <typename BoneType>
bool Skeleton<BoneType>::move(
  int x,
  int y, 
  bool shift_down, 
  bool control_down, 
  bool meta_down)
{
  using namespace std;
  bool bone_move = false;
  // get bones
  vector<BoneType *> B = gather_bones(roots);
  // check bones
  for(typename vector<BoneType *>::iterator bit = B.begin();bit != B.end();bit++)
  {
    BoneType * b = (*bit);
    if(b->move(x,y,shift_down,control_down,meta_down))
    {
      bone_move = true;
    }
  }
  return bone_move;
}
template <typename BoneType>
bool Skeleton<BoneType>::inside(
  int /*x*/,
  int /*y*/)
{
  return false;
}

template <typename BoneType>
void Skeleton<BoneType>::draw()
{
  using namespace std;
  glEnable(GL_POINT_SMOOTH);
  // get bones
  vector<BoneType *> B = gather_bones(roots);
  if(bones_on_top)
  {
    // First draw bones without depth ordering (so all appear on top)
    glDisable(GL_DEPTH_TEST);
    for_each(B.begin(),B.end(),mem_fun(&BoneType::draw));
  }
  // Then draw again with depth ordering (so ordering comes out according to
  // depth)
  glEnable(GL_DEPTH_TEST);
  for_each(B.begin(),B.end(),mem_fun(&BoneType::draw));
}

template <typename BoneType>
BoneType * Skeleton<BoneType>::find_selected(const bool deselect_others)
{
  using namespace std;
  // Get selected bone
  vector<BoneType*> B = gather_bones(roots);
  typename vector<BoneType *>::iterator bi = find_if(B.begin(),B.end(),is_bone_selected<BoneType>);
  if(bi == B.end())
  {
    return NULL;
  }
  // Deselect all others
  if(deselect_others)
  {
    for_each((bi+1),B.end(),
        bind2nd(mem_fun(&BoneType::set_is_tip_selected),false));
    for_each((bi+1),B.end(),
      bind2nd(mem_fun(&BoneType::set_is_line_segment_selected),false));
  }
  return *bi;
}

template <typename BoneType>
std::vector<BoneType*> Skeleton<BoneType>::find_all_selected()
{
  using namespace std;
  vector<BoneType*> sel;
  vector<BoneType*> B = gather_bones(roots);
  typename vector<BoneType *>::iterator bi = B.begin()-1;
  do
  {
    bi = find_if(bi+1,B.end(),is_bone_selected<BoneType>);
    if(bi == B.end())
    {
      break;
    }
    sel.push_back(*bi);
  }while(true);
  return sel;
}

template <typename BoneType>
bool Skeleton<BoneType>::set_editing(const bool v)
{
  using namespace std;
  editing = v;
  if(editing)
  {
    vector<BoneType*> B = gather_bones(roots);
    // call reset() for each bone
    for_each(B.begin(),B.end(),mem_fun(&BoneType::reset));
    after_set_editing(*this);
  }
  return editing;
}

template <typename BoneType>
bool Skeleton<BoneType>::get_editing() const
{
  return editing;
}

template <typename BoneType>
void Skeleton<BoneType>::print() const
{
  using namespace std;
  // get bones
  vector<BoneType *> B = gather_bones(roots);
  // check bones
  for(typename vector<BoneType *>::iterator bit = B.begin();bit != B.end();bit++)
  {
    BoneType * b = (*bit);
    cout<<"is root: "<<b->is_root()<<endl;
    cout<<"offset: "<<
      b->offset[0]<<" "<<
      b->offset[1]<<" "<<
      b->offset[2]<<" "<<endl;
  }
}
