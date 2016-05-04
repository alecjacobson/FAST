#ifndef BONE_H
#define BONE_H

#include <vector>
#include "Widget.h"
#include "Vec.h"
#include "Quat.h"
#include "Tform.h"
#include "DegreeOfFreedomType.h"

#include "Skeleton.h"
// Bone class
// A bone represents a handle controlling a rigid line segment. It is usually
// part of a "skeleton" or tree of bones. In this implementation a bone is
// either a special root or anchor or it is a child of some other bone. In
// that way only non-root bones actually control rigid line segments. The line
// segments are defined as line segment to this bone's destination from this
// bone's parent's destination. Root bones have a destination but no parent.
// Destinations are found by traversing bone offsets starting at a bone's root.

class Bone : public Widget
{
  ///////////////////////////////////////////////////////////////////////////
  // Public fields
  ///////////////////////////////////////////////////////////////////////////
  public:
    // Positional offset from parent bone, if parent is NULL then offset is
    // taken from origin
    Vec3 offset;
    // Current rotation applied to this bone
    Quat rotation;
    // Current translation applied to this bone, usually for non-roots this
    // will be (0,0,0)
    Vec3 translation;
    // Current stretch applied to this bone, scalar value where 1.0 means
    // identity stretch
    double stretch;
    // Current twist angle applied to this bone, for special use with STBS, in
    // an LBS rig this will always be 0 and twisting must be "handled" by
    // rotation, scalar radian angle where 0 means now twist
    double twist;
    // Keep track if line segment or tip is currently selected or hovered over
    // Keep track of what used to be selected before processing click
    bool is_line_segment_selected;
    bool is_line_segment_hover;
    bool was_line_segment_selected;
    bool is_tip_selected;
    bool is_tip_hover;
    bool was_tip_selected;
    DegreeOfFreedomType tip_dof_type;
    // Last transformation passed from skinning program (what was actually used
    // in skinning regardless of the transformation stored above), used for
    // drawing and picking
    Tform3 last_T;
    // Container class parent
    Skeleton<Bone> * skel;

  ///////////////////////////////////////////////////////////////////////////
  // Private fields
  ///////////////////////////////////////////////////////////////////////////
  private:
    // Parent Bone, NULL only if this bone is a root
    Bone * parent;
    // Index in weights associated with this rig
    int wi;
    // Children bones, may be empty
    std::vector<Bone*> children;
    // Whether bone is currently draggin out a rotation
    bool dragging_rotation;

  ///////////////////////////////////////////////////////////////////////////
  // Public functions
  ///////////////////////////////////////////////////////////////////////////
  public:
    ///////////////////////////////////////////////////////////////////////////
    // Initialization
    ///////////////////////////////////////////////////////////////////////////
    // Constructor
    // Inputs:
    //   parent_  parent bone {NULL}
    //   offset_  offset from parent bone {(0,0,0)}
    Bone(
      Skeleton<Bone> * skel,
      Bone * parent_,
      const Vec3 & offset_);
    // Safe copy constructor, copy transformations etc but not heirarchy
    // information.
    // Inputs:
    //   that  other bone
    Bone(const Bone * that);
    ~Bone();
    // Sets the weight index of this bone. Results in error if bone is root
    // or weight is less than 0
    // Inputs:
    //   wi  new weight index for this bone
    // Returns this
    Bone * set_wi(int wi);
    // Gets the weights index of this bone. Results in error if wi has not yet
    // been set, if this is a root, or if wi is non-negative
    // Returns this->wi
    int get_wi() const;
    // Returns true only if wi has been set
    bool wi_is_set() const;
    // Sets the parent of this bone
    // Returns this
    Bone * set_parent(Bone * parent);
    // Returns pointer to parent, NULL for roots
    const Bone * get_parent() const;
    // Get list of children bones
    const std::vector<Bone*> & get_children() const;
    // Returns true if is root (aka parent == null)
    bool is_root() const;
    // Reset transformation to identity
    void reset();
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
    // Drawing
    ///////////////////////////////////////////////////////////////////////////
    void draw_tip() const;
    // Sets color according to status
    // Output:
    //   pcolor  double[3] color of tip
    void tip_color(double pcolor[3]) const;
    // Sets color accordint to line segment status
    // Output:
    //   lcolor  double[3] color of line
    void line_segment_color(double lcolor[3]) const;
    // Draw a line from (down_x,down_y) to (last_x,last_y)
    void draw_drag_line() const;
    ///////////////////////////////////////////////////////////////////////////
    // Forward kinematics
    ///////////////////////////////////////////////////////////////////////////
    // Computes the current orientation via Forward kinematics, i.e composing
    // orientations starting from the root. 
    // Returns quaternion representation of this bones orientation in 3d
    Quat orientation() const;
    // Inputs:
    //  according_to_last_T  whether to use last_T or bone's transformation
    //  average_children_tails  whether to use an average of what this bone's
    //    children think it is or use this bone's position
    // Returns the current position of the tip of this bone, the point shared
    // by this bone and its children if any
    Vec3 tip(
      const bool according_to_last_T,
      const bool average_children_tails) const;
    // Compute tip as where it is being drawn
    Vec3 tip_as_drawn() const;
    // Inputs:
    //  according_to_last_T  whether to use last_T or bone's transformation
    // Returns the current position of the tail of this non-root bone, the
    // point shared by this bone and its parent.
    // Error if this is a root bone
    Vec3 tail(const bool according_to_last_T) const;
    // Compute tail as where it is being drawn
    Vec3 tail_as_drawn() const;
    // Returns the rest position of the tip of this bone.
    Vec3 rest_tip() const;
    // Returns the rest position of the ail of this non-root bone.
    Vec3 rest_tail() const;
    // Returns the current affine transformation via Forward Kinematics of this
    // bone
    Tform3 affine() const;
    // Returns the current rotated frame via Forward Kinematics of this
    // bone
    Quat rotated_frame() const;
    // Apply affine transformation to offsets of descendents 
    // Inputs:
    //    A  affine transformation
    //
    void apply_affine_to_kin(const Tform3 & A);

    ///////////////////////////////////////////////////////////////////////////
    // Selection
    ///////////////////////////////////////////////////////////////////////////
    Bone * set_is_tip_selected(const bool v);
    Bone * set_is_line_segment_selected(const bool v);
    // Restores is_*_selected flags according to was_*_selected flags
    // Returns this
    Bone * reset_is_selected();
    ///////////////////////////////////////////////////////////////////////////
    // DEBUG
    ///////////////////////////////////////////////////////////////////////////
    void print() const;
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

#endif
