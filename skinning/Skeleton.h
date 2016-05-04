#ifndef SKELETON_H
#define SKELETON_H

#include <vector>
#include <functional>
 
// A Skeleton object is simple a collection of forests of "Bone" classes. Each
// tree designated by its respective root
#include "ViewVector.h"
#include "Widget.h"
#include <Eigen/Core>

template <typename BoneType>
class Skeleton : public Widget
{
  ///////////////////////////////////////////////////////////////////////////
  // Public fields
  ///////////////////////////////////////////////////////////////////////////
  public:
    std::vector<BoneType*> roots;
    // Draw bones (and also respond to UI) using last_T to define positions
    // rather than transformations stored in bones
    bool draw_according_to_last_T;
    // Draw bone segments such that they *always* connect this bone to parent,
    // regardless of stored transformations (i.e. non-trivial translations)
    bool draw_connected_to_parent;
    // Draw non-weighted roots as average of children's tails, otherwise do not
    // draw them
    bool average_children_tails_to_draw_non_weighted_roots;
    // Vector about which to rotate when right-click dragging
    ViewVector view_vector;
    // Draw handles on top of everything else in the scene
    bool bones_on_top;
    // Type definition for a callback that will receive a const reference to
    // this skeleton as input and returns a void pointer
    // Method 
    typedef std::function<void (const Skeleton<BoneType> &)> SkeletonCallback;
    // Callback called after setting editing field
    SkeletonCallback after_set_editing;
    bool draw_drag_rotations;
    // Don't allow editing with the mouse
    bool lock_mouse_new;


  ///////////////////////////////////////////////////////////////////////////
  // Private fields
  ///////////////////////////////////////////////////////////////////////////
  private:
    // Editting rest positon
    bool editing;
  ///////////////////////////////////////////////////////////////////////////
  // Public functions
  ///////////////////////////////////////////////////////////////////////////
  public:
    ///////////////////////////////////////////////////////////////////////////
    // Initialization
    ///////////////////////////////////////////////////////////////////////////
    Skeleton();
    ~Skeleton();
    ///////////////////////////////////////////////////////////////////////////
    // Implemented Widget functions
    ///////////////////////////////////////////////////////////////////////////
    // See Widget.h
    virtual bool down(
      int x, int y, 
      bool right_click, bool shift_down, bool control_down, bool meta_down);
    virtual bool up(  
      int x, int y, 
      bool right_click, bool shift_down, bool control_down, bool meta_down);
    virtual bool drag(
      int x, int y, 
      bool right_click, bool shift_down, bool control_down, bool meta_down);
    virtual bool move(
      int x, int y, 
      bool shift_down, bool control_down, bool meta_down);
    virtual bool inside(int x,int y);
    virtual void draw();
    ///////////////////////////////////////////////////////////////////////////
    // Selection
    ///////////////////////////////////////////////////////////////////////////
    // Inputs:
    //   deselect_others deselect any other selected bones
    // Return pointer to first bone found to be selected
    // Returns NULL if no bones are selected
    BoneType * find_selected(const bool deselect_others = false);
    // Return a list of pointers to bones found to be selected
    std::vector<BoneType*> find_all_selected();
    ///////////////////////////////////////////////////////////////////////////
    // Accessors
    ///////////////////////////////////////////////////////////////////////////
    bool set_editing(const bool v);
    bool get_editing() const;
    void print() const;
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};
#endif
