#define VERBOSE
#include "unused.h"
#include "SkinningCallback.h"
#include "Skinning.h"
#include "gather_bones.h"
#include "Bone.h"
#include "Quat.h"
#include "matrix_from_string.h"
#include "ColorOption.h"
#include "gather_transformations.h"
#include "destroy_bone_roots.h"
#include "gather_samples.h"
#include "distribute_weight_indices.h"
#include "boundary_conditions.h"

// Standard library
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <cstdio>

// IGL library
#include <igl/canonical_quaternions.h>
#include <igl/snap_to_canonical_view_quat.h>
#include <igl/get_seconds.h>
#include <igl/transpose_blocks.h>
#include <igl/per_corner_normals.h>
#include <igl/cat.h>
#include <igl/writeMESH.h>
#include <igl/launch_medit.h>
#include <igl/faces_first.h>
//#include <igl/ReAntTweakBar.h>
#include <igl/ONE.h>
#include <igl/ZERO.h>
#include <igl/columnize.h>
#include <igl/normalize_row_sums.h>

#ifndef NO_TETGEN
  // TETGEN
  // Must define this to ensure header knows we're using it as a library and not
  // building the tetgen command line app
  // Define this at the top to be sure that if others include tetgen.h we get the
  // library version
#  define TETLIBRARY
#  include <igl/tetgen/tetrahedralize.h>
#endif
#ifndef NO_MOSEK
#  include <igl/bbw/bbw.h>
#endif

#include <Eigen/Dense>

using namespace std;
using namespace igl;
using namespace Eigen;

inline Skinning * SkinningCallback::scast(void *clientData)
{
  return static_cast<Skinning *>(clientData);
}

void TW_CALL SkinningCallback::no_op(const void * /*value*/, void * /*clientData*/)
{
  // do nothing
}

void TW_CALL SkinningCallback::no_op(void * /*value*/, void * /*clientData*/)
{
  // do nothing
}

void TW_CALL SkinningCallback::view_xy_plane(void *clientData)
{
  Skinning * skinning = scast(clientData);
  copy(XY_PLANE_QUAT_F,XY_PLANE_QUAT_F+4,skinning->camera.rotation);
}

void TW_CALL SkinningCallback::view_xz_plane(void *clientData)
{
  Skinning * skinning = scast(clientData);
  copy(XZ_PLANE_QUAT_F,XZ_PLANE_QUAT_F+4,skinning->camera.rotation);
}

void TW_CALL SkinningCallback::view_yz_plane(void *clientData)
{
  Skinning * skinning = scast(clientData);
  copy(YZ_PLANE_QUAT_F,YZ_PLANE_QUAT_F+4,skinning->camera.rotation);
}

void TW_CALL SkinningCallback::zero_pan(void *clientData)
{
  Skinning * skinning = scast(clientData);
  skinning->camera.pan[0] = 0;
  skinning->camera.pan[1] = 0;
  skinning->camera.pan[2] = 0;
}

// Check zoom lock before seting new zoom value
void TW_CALL SkinningCallback::set_zoom(const void *value, void *clientData)
{
  Skinning * skinning = scast(clientData);
  if(!skinning->zoom_locked)
  {
    skinning->camera.zoom = *(const double*)(value);
  }
}
// Get current zoom value
void TW_CALL SkinningCallback::get_zoom(void *value, void *clientData)
{
  Skinning * skinning = scast(clientData);
  *(double *)(value) = skinning->camera.zoom;
}

// Check angle lock before seting new angle value
void TW_CALL SkinningCallback::set_angle(const void *value, void *clientData)
{
  Skinning * skinning = scast(clientData);
  if(!skinning->angle_locked)
  {
    skinning->camera.angle = *(const double*)(value);
  }
}
// Get current angle value
void TW_CALL SkinningCallback::get_angle(void *value, void *clientData)
{
  Skinning * skinning = scast(clientData);
  *(double *)(value) = skinning->camera.angle;
}

// Get current number of vertices
void TW_CALL SkinningCallback::get_num_vertices(void * value, void *clientData)
{
  Skinning * skinning = scast(clientData);
  *(int *)(value) = skinning->V.rows();
}
// Get current number of faces
void TW_CALL SkinningCallback::get_num_faces(void * value, void *clientData)
{
  Skinning * skinning = scast(clientData);
  *(int *)(value) = skinning->F.rows();
}

void TW_CALL SkinningCallback::set_spinning_about_up_axis(
  const void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  skinning->spinning_about_up_axis = *(const bool*)(value);
  if(skinning->spinning_about_up_axis)
  {
    // If just started spinning then reset spin timer
    skinning->spin_start_seconds = get_seconds();
    copy(skinning->camera.rotation,skinning->camera.rotation+4,
      skinning->spin_start_rotation);
  }
}
void TW_CALL SkinningCallback::get_spinning_about_up_axis(void *value, void *clientData)
{
  Skinning * skinning = scast(clientData);
  *(bool *)(value) = skinning->spinning_about_up_axis;
}

void TW_CALL SkinningCallback::snap_rotation_to_canonical_view_quat(void *clientData)
{
  Skinning * skinning = scast(clientData);
  snap_to_canonical_view_quat<double>(
    skinning->camera.rotation,
    1.0,
    skinning->camera.rotation);
}

void TW_CALL SkinningCallback::clear(void *clientData)
{
  Skinning * skinning = scast(clientData);
  skinning->clear();
}

// Get weight index of selected bone
void TW_CALL SkinningCallback::get_selected_bone_wi(
  void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  // Get selected bone
  Skeleton<Bone> * skel = skinning->skel;
  Bone* b = skel->find_selected();
  if(b != NULL)
  {
    *(int *)(value) = b->get_wi();
  }
}

// Set and get the selected bone's rotation
void TW_CALL SkinningCallback::set_selected_bone_rotation(
  const void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  // Get selected bone
  Bone* b = skinning->skel->find_selected(true);
  if(b != NULL)
  {
    // case current value to double
    const double * quat = (const double *)(value);
    // Remove object rotation by left multiplying stored quaternion by object
    // rotation's inverse
    Quat obj_rot(
      skinning->camera.rotation[3],
      skinning->camera.rotation[0],
      skinning->camera.rotation[1],
      skinning->camera.rotation[2]);
    // Offset *ALL* rotations so that rotations are about the parent (self in
    // case of roots)
    Vec3 tail = b->offset;
    if(!b->is_root())
    {
      tail = b->get_parent()->rest_tip();
    }
    // Keep track of old translation
    Vec3 ot = (b->rotation*(b->translation + tail))-tail;
    // Adjust rotation
    b->rotation = obj_rot.inverse() * Quat(quat[3],quat[0],quat[1],quat[2]);
    //if(skinning->center_root_rotations && b->is_root())
    b->translation = 
      ((b->rotation.inverse() * (tail+ot)) - tail);
  }
}

void TW_CALL SkinningCallback::get_selected_bone_rotation(
  void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  // Get selected bone
  Bone* b = skinning->skel->find_selected();
  if(b != NULL)
  {
    // case current value to double
    double * quat = (double *)(value);
    // Left multiply by object rotation
    Quat obj_rot(
      skinning->camera.rotation[3],
      skinning->camera.rotation[0],
      skinning->camera.rotation[1],
      skinning->camera.rotation[2]);
    Quat rot = obj_rot * b->rotation;
    quat[0] = rot.x();
    quat[1] = rot.y();
    quat[2] = rot.z();
    quat[3] = rot.w();
  }
}

// Set and get the selected bone's translation
void TW_CALL SkinningCallback::set_selected_bone_translation_x(
  const void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  // Get selected bone
  Bone * b = skinning->skel->find_selected(true);
  if(b != NULL)
  {
    // case current value to double
    const double * t = (const double *)(value);
    b->translation[0] = t[0];
  }
}
void TW_CALL SkinningCallback::get_selected_bone_translation_x(
  void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  // Get selected bone
  Bone * b = skinning->skel->find_selected();
  if(b != NULL)
  {
    // case current value to double
    double * t = (double *)(value);
    t[0] = b->translation[0];
  }
}

// Set and get the selected bone's translation
void TW_CALL SkinningCallback::set_selected_bone_translation_y(
  const void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  // Get selected bone
  Bone * b = skinning->skel->find_selected(true);
  if(b != NULL)
  {
    // case current value to double
    const double * t = (const double *)(value);
    b->translation[1] = t[0];
  }
}
void TW_CALL SkinningCallback::get_selected_bone_translation_y(
  void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  // Get selected bone
  Bone * b = skinning->skel->find_selected();
  if(b != NULL)
  {
    // case current value to double
    double * t = (double *)(value);
    t[0] = b->translation[1];
  }
}

// Set and get the selected bone's translation
void TW_CALL SkinningCallback::set_selected_bone_translation_z(
  const void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  // Get selected bone
  Bone * b = skinning->skel->find_selected(true);
  if(b != NULL)
  {
    // case current value to double
    const double * t = (const double *)(value);
    b->translation[2] = t[0];
  }
}
void TW_CALL SkinningCallback::get_selected_bone_translation_z(
  void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  // Get selected bone
  Bone * b = skinning->skel->find_selected();
  if(b != NULL)
  {
    // case current value to double
    double * t = (double *)(value);
    t[0] = b->translation[2];
  }
}

void TW_CALL SkinningCallback::set_selected_bone_dof_type(
  const void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  bool changed = false;
  vector<Bone*> sel = skinning->skel->find_all_selected();
  for(vector<Bone*>::iterator bi = sel.begin();bi!=sel.end();bi++)
  {
    // case current value to bool
    (*bi)->tip_dof_type = *(const DegreeOfFreedomType *)(value);
    changed = true;
  }
  // trigger reinitialization of arap dof
  if(changed && skinning->auto_dof)
  {
    skinning->reinitialize_auto_dof();
  }
}

void TW_CALL SkinningCallback::get_selected_bone_dof_type(
  void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  // Get selected bone
  Bone * b = skinning->skel->find_selected();
  if(b != NULL)
  {
    *(DegreeOfFreedomType *)(value) = b->tip_dof_type;
  }
}

void TW_CALL SkinningCallback::set_selected_bone_last_T(
  const void *value, void *clientData)
{
  // Set: copy the value of s from AntTweakBar
  const std::string *srcPtr = static_cast<const std::string *>(value);
  string T_str = *srcPtr;

  MatrixXd T;
  T.resize(3,4);
  // Parse string into transform
  bool success = matrix_from_string(3,4,T_str,T);

  // return early if transform didn't make any sense
  if(!success)
  {
    return;
  }

  Skinning * skinning = scast(clientData);
  // Get selected bones
  vector<Bone*> sel = skinning->skel->find_all_selected();
  for(vector<Bone*>::iterator bi = sel.begin();bi!=sel.end();bi++)
  {
    (*bi)->last_T.affine() = T;
  }
}

void TW_CALL SkinningCallback::get_selected_bone_last_T(
  void *value, 
  void *clientData)
{
  using namespace std;
  Skinning * skinning = scast(clientData);
  // Get selected bone
  Bone * b = skinning->skel->find_selected();
  stringstream T_stream;
  if(b != NULL)
  {
    Tform3 T = b->last_T;
    // convert T into a string
    T_stream << 
      T(0,0) << "," << T(0,1) << "," << T(0,2) << "," << T(0,3) << ";" <<
      T(1,0) << "," << T(1,1) << "," << T(1,2) << "," << T(1,3) << ";" <<
      T(2,0) << "," << T(2,1) << "," << T(2,2) << "," << T(2,3);
  }

  // Get: copy the value of s to AntTweakBar
  std::string *destPtr = static_cast<std::string *>(value);
  // the use of TwCopyStdStringToLibrary is required here
  TwCopyStdStringToLibrary(*destPtr, T_stream.str());
}
// Snap current selection bone's rotation to canonical view quaternion
void TW_CALL 
  SkinningCallback::snap_selected_bone_rotation_to_canonical_view_quat(
  void *clientData)
{

  Skinning * skinning = scast(clientData);
  // Get selected bones
  vector<Bone*> sel = skinning->skel->find_all_selected();
  for(vector<Bone*>::iterator bi = sel.begin();bi!=sel.end();bi++)
  {
    double q[4];
    q[0] = (*bi)->rotation.x();
    q[1] = (*bi)->rotation.y();
    q[2] = (*bi)->rotation.z();
    q[3] = (*bi)->rotation.w();
    snap_to_canonical_view_quat<double>( q, 1.0, q);
    Quat new_rot(q[3],q[0],q[1],q[2]);

    // Offset *ALL* rotations so that rotations are about the parent (self in
    // case of roots)
    Vec3 tail = (*bi)->offset;
    if(!(*bi)->is_root())
    {
      tail = (*bi)->get_parent()->rest_tip();
    }
    // Keep track of old translation
    Vec3 ot = ((*bi)->rotation*((*bi)->translation + tail))-tail;
    // Adjust rotation
    (*bi)->rotation = new_rot;
    //if(skinning->center_root_rotations && (*bi)->is_root())
    (*bi)->translation = 
      (((*bi)->rotation.inverse() * (tail+ot)) - tail);
  }

}

void TW_CALL SkinningCallback::select_all(void *clientData)
{
  Skinning * skinning = scast(clientData);
  // Get selected bone
  vector<Bone*> B = gather_bones(skinning->skel->roots);
  for_each(B.begin(),B.end(),
      bind2nd(mem_fun(&Bone::set_is_tip_selected),true));
}

void TW_CALL SkinningCallback::reset_selected_bone_transformation(void *clientData)
{
  Skinning * skinning = scast(clientData);
  // Get selected bones
  vector<Bone*> sel = skinning->skel->find_all_selected();
  for(vector<Bone*>::iterator bi = sel.begin();bi!=sel.end();bi++)
  {
    (*bi)->reset();
    // increment iterator and keep looking for selected bones
  }
}

void TW_CALL SkinningCallback::reset_extra_transformations(void * clientData)
{
  Skinning * skinning = scast(clientData);
  // Zero out extra transformations, by zeroing out all
  skinning->T.setZero();
  // Re-gather T of non extras 
  bool gather_success = 
    gather_transformations(
      skinning->skel->roots,
      skinning->dial_in_each_T,
      skinning->T);
  if(!gather_success)
  {
    return;
  }
  if(skinning->auto_dof)
  {
    int m = skinning->arap_dof.m;
    if(m != skinning->T.cols()/4)
    {
      fprintf(stderr,
        "Error: transformations()" 
        "number of handles in T (%d) doesn't match that in arap_dof data (%d)\n" 
        "you must reinitialize.\n",
        (int)(skinning->T.cols()/4),
        m);
      skinning->auto_dof = false;
      return;
    }
    MatrixXf Tcol;
    // number of dimensions
    columnize(
      skinning->T.block(
        0,0,skinning->arap_dof.dim,skinning->T.cols()).eval(),m,2,Tcol);
    // Replace last solution with current T
    skinning->L = Tcol.cast<double>().eval();
  }
}

void TW_CALL SkinningCallback::reset_all_bone_transformations(void *clientData)
{
  Skinning * skinning = scast(clientData);
  // Get selected bone
  vector<Bone*> B = gather_bones(skinning->skel->roots);
  // call reset() for each bone
  for_each(B.begin(),B.end(),mem_fun(&Bone::reset));
  if(skinning->auto_dof)
  {
    skinning->initialize_transformations();
  }
}

void TW_CALL SkinningCallback::set_auto_dof(const void *value, void *clientData)
{
  Skinning * skinning = scast(clientData);
  bool old_value = skinning->auto_dof;
  skinning->auto_dof = *(const bool*)(value);
  // Only try to initialize if value changed from false to true
  if(skinning->auto_dof && !old_value)
  {
    skinning->initialize_auto_dof();
  }
}
void TW_CALL SkinningCallback::get_auto_dof(void *value, void *clientData)
{
  Skinning * skinning = scast(clientData);
  *(bool *)(value) = skinning->auto_dof;
}

void TW_CALL SkinningCallback::print_transformations(void *clientData)
{
  Skinning * skinning = scast(clientData);
  int m = skinning->T.cols()/4;
  int rows = skinning->T.rows();
  cout<<"T=["<<endl;
  for(int i = 0;i<m;i++)
  {
    cout<<endl<<skinning->T.block(0,i*4,rows,4)<<endl;
  }
  cout<<"];"<<endl;

  cout<<"d=["<<endl;
  for(int i = 0;i<m;i++)
  {
    cout<<endl<<skinning->T.block(0,i*4,3,3).determinant()<<endl;
  }
  cout<<"];"<<endl;
}

void TW_CALL SkinningCallback::clear_extra_weights(void *clientData)
{
  Skinning * skinning = scast(clientData);
  skinning->EW.resize(skinning->V.rows(),0);
  skinning->initialize_weights();
}

void TW_CALL SkinningCallback::set_animate(const void * value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  if(value && !skinning->animating)
  {
    skinning->start_animating();
  }else if(skinning->animating)
  {
    skinning->stop_animating();
  }
}

void TW_CALL SkinningCallback::get_animate(void * value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  *(bool *)(value) = skinning->animating;
}

void TW_CALL SkinningCallback::get_animation_size(void * value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  *(int *)(value) = skinning->animation.size();
}

void TW_CALL SkinningCallback::push_keyframe(void * clientData)
{
  Skinning * skinning = scast(clientData);
  if(!skinning->animating)
  {
    skinning->animation.push_back(
      KeyFrame<BoneBoneCopyMap>(
        BoneBoneCopyMap(skinning->skel->roots,false),
        skinning->animation_interp_secs,
        skinning->transition_type));
    skinning->camera_animation.push_back(
      KeyFrame<Camera>(
        skinning->camera,
        skinning->animation_interp_secs,
        skinning->transition_type));
  }
}

void TW_CALL SkinningCallback::pop_keyframe(void * clientData)
{
  Skinning * skinning = scast(clientData);
  if(!skinning->animating)
  {
    if(skinning->animation.size() > 0)
    {
      skinning->animation.pop_back();
    }
    if(skinning->camera_animation.size() > 0)
    {
      skinning->camera_animation.pop_back();
    }
  }
}

void TW_CALL SkinningCallback::set_color_option(const void * value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  skinning->color_option =  *(const ColorOption*)(value);
  if(skinning->color_option == COLOR_OPTION_ONE_SETS &&
    skinning->RC.rows()!= skinning->F.rows()*3)
  {
    verbose("RC.rows() %d != F.rows()*3 %d\n",
      skinning->RC.rows(),skinning->F.rows()*3);
    skinning->color_option = COLOR_OPTION_MATCH_DIFFUSE;
  }
  skinning->display_list_damage = true;
}

void TW_CALL SkinningCallback::get_color_option(void * value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  *(ColorOption *)(value) = skinning->color_option;
}

void TW_CALL SkinningCallback::set_bypass_auto_dof(const void * value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  skinning->bypass_auto_dof =  *(const bool*)(value);
  if(skinning->bypass_auto_dof)
  {
    // keep track of current T
    skinning->T_at_bypass = skinning->T;
    // Zero out extra transformations, by zeroing out all
    skinning->T.setZero();
    // Re-gather T of non extras 
    bool gather_success = 
      gather_transformations(
        skinning->skel->roots,
        skinning->dial_in_each_T,
        skinning->T);
    if(!gather_success)
    {
      return;
    }
  }else
  {
    // Restore old transformations
    if(
      skinning->T.rows() == skinning->T_at_bypass.rows() && 
      skinning->T.cols() == skinning->T_at_bypass.cols())
    {
      verbose("Restored old transformations\n");
      skinning->T = skinning->T_at_bypass;
    }else
    {
      verbose("ERROR: Could not restored old transformations\n");
    }
  }
}

void TW_CALL SkinningCallback::get_bypass_auto_dof(void * value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  *(bool *)(value) = skinning->bypass_auto_dof;
}

void TW_CALL SkinningCallback::set_corner_threshold(const void * value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  skinning->corner_threshold =  *(const double*)(value);
  //// Recompute corner normals
  //per_corner_normals(
  //  skinning->V,
  //  skinning->F,
  //  skinning->corner_threshold,
  //  skinning->CN);
  skinning->display_list_damage = true;
}

void TW_CALL SkinningCallback::get_corner_threshold(void * value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  *(double *)(value) = skinning->corner_threshold;
}

void TW_CALL SkinningCallback::set_normal_type(const void * value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  skinning->normal_type =  *(const NormalType*)(value);
  skinning->display_list_damage = true;
}

void TW_CALL SkinningCallback::get_normal_type(void * value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  *(NormalType *)(value) = skinning->normal_type;
}

void TW_CALL SkinningCallback::set_use_texture_mapping(const void * value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  skinning->use_texture_mapping =  *(const bool*)(value);
  skinning->display_list_damage = true;
}

void TW_CALL SkinningCallback::get_use_texture_mapping(void * value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  *(bool *)(value) = skinning->use_texture_mapping;
}

void TW_CALL SkinningCallback::clear_bone_roots(void *clientData)
{
  Skinning * skinning = scast(clientData);
  verbose("Clearing bone roots...\n");
  destroy_bone_roots(skinning->skel->roots);
}

void TW_CALL SkinningCallback::set_editing(
  const void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  bool was_editing = skinning->skel->get_editing();
  bool v = *(const bool*)(value);
  skinning->skel->set_editing(v);
  if(was_editing && !skinning->skel->get_editing())
  {
#ifndef NO_PUPPET
    skinning->pi->set_frames();
#endif
  }else if(!was_editing && skinning->skel->get_editing())
  {
#ifndef NO_PUPPET
    skinning->pi->unset_frames();
#endif
  }
}

void TW_CALL SkinningCallback::get_editing(
  void *value, 
  void *clientData)
{
  Skinning * skinning = scast(clientData);
  *(bool *)(value) = skinning->skel->get_editing();
}

void TW_CALL SkinningCallback::delete_selected_bones(void *clientData)
{
  Skinning * skinning = scast(clientData);
  if(skinning->skel->get_editing())
  {
    assert(!skinning->auto_dof);
    // Delete first selected vertex found until there are no more
    while(true)
    {
      Bone * b = skinning->skel->find_selected();
      if(b == NULL)
      {
        break;
      }
      // Remove from list of roots
      skinning->skel->roots.erase(
        remove(skinning->skel->roots.begin(), skinning->skel->roots.end(), b), 
        skinning->skel->roots.end());
      // Destroying takes care of deleting children
      delete b;
    }
  }else
  {
    verbose(
      "^delete_selected_bones: Warning: Must be in edit mode to delete\n");
  }
}

#if defined(NO_MOSEK) || defined(NO_TETGEN)
void TW_CALL SkinningCallback::compute_weights(void * /*clientData*/)
{
  verbose(
    "^SC:compute_weights: ERROR: compiled with -DNO_MOSEK or -DNO_TETGEN "
    "so unable to compute weights\n");
  return;
}
#else
void TW_CALL SkinningCallback::compute_weights(void * clientData)
{
  Skinning * skinning = scast(clientData);
  skinning->auto_dof = false;
  verbose("Computing BBW weights\n");
  // Determine which Bones have weights
  bool success = distribute_weight_indices(skinning->skel->roots);
  if(!success)
  {
    return;
  }
  // Gather samples along controls
  MatrixXd S;
  gather_samples(skinning->skel->roots,10,S);
  VectorXi IM;
  faces_first(skinning->V,skinning->F,IM);
  skinning->Tets = 
    skinning->Tets.unaryExpr(bind1st(mem_fun( static_cast<VectorXi::Scalar&
    (VectorXi::*)(VectorXi::Index)>(&VectorXi::operator())),
    &IM)).eval();
  // Surface vertices
  MatrixXd SV = 
    skinning->V.block(0,0,skinning->F.maxCoeff()+1,skinning->V.cols());
  // Remesh at control samples
  MatrixXd VS = cat(1,SV,S);
  // Boundary faces
  MatrixXi BF;
  cout<<"tetgen begin()"<<endl;
  int status = 
    tetrahedralize(
      VS,skinning->F,"Ypq100",skinning->V,skinning->Tets,BF);
  cout<<"tetgen end()"<<endl;
  if(BF.rows() != skinning->F.rows())
  {
    //assert(BF.maxCoeff() == skinning->F.maxCoeff());
    verbose("^%s: Warning: boundary faces != orignal faces\n",__FUNCTION__);
  }
  if(status != 0)
  {
    verbose(
        "************************************************************\n"
        "************************************************************\n"
        "************************************************************\n"
        "************************************************************\n"
        "* ^%s: tetgen failed. Just meshing convex hull\n"
        "************************************************************\n"
        "************************************************************\n"
        "************************************************************\n"
        "************************************************************\n"
        ,__FUNCTION__);
    status =
      tetrahedralize(
        VS,skinning->F,"q1.414",skinning->V,skinning->Tets,BF);
    assert(skinning->F.maxCoeff() < skinning->V.rows());
    if(status != 0)
    {
      verbose("^%s: tetgen failed again.\n",__FUNCTION__);
      return;
    }
  }
#ifdef __APPLE__
    launch_medit(skinning->V,skinning->Tets,skinning->F,false);
#endif
  skinning->initialize_mesh();

  // Get boundary conditions
  VectorXi b;
  MatrixXd bc;
  boundary_conditions(skinning->V,skinning->Tets,skinning->skel->roots,b,bc);

  // call BBW 
  BBWData bbw_data;
  bbw_data.active_set_params.max_iter = 10;
  success = bbw(
    skinning->V,
    skinning->Tets,
    b,
    bc,
    bbw_data,
    skinning->OW
    );
  skinning->OW = 
    (skinning->OW.array().colwise() / 
      skinning->OW.rowwise().sum().array()).eval();
  if(!success)
  {
    return;
  }
  // Clear extra weights
  skinning->skel->set_editing(false);
  skinning->EW.resize(skinning->OW.rows(),0);
  skinning->initialize_weights();
}
#endif

void TW_CALL SkinningCallback::set_dial_in_each_T(const void *value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  using namespace igl;
  skinning->dial_in_each_T = *(const bool*)(value);
  if(skinning->dial_in_each_T)
  {
    skinning->set_twbarvar_readonly("sb_last_T",!skinning->dial_in_each_T);
  }
}
void TW_CALL SkinningCallback::get_dial_in_each_T(void *value, void *clientData)
{
  Skinning * skinning = scast(clientData);
  *(bool *)(value) = skinning->dial_in_each_T;
}


void TW_CALL SkinningCallback::set_arap_dof_h(const void *value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  double old_value = skinning->arap_dof.h;
  skinning->arap_dof.h = *(const double*)(value);
  if( (old_value != skinning->arap_dof.h) && skinning->auto_dof)
  {
    skinning->seconds_per_update = max(skinning->seconds_per_update,skinning->arap_dof.h);
    skinning->reinitialize_auto_dof();
  }
}
void TW_CALL SkinningCallback::get_arap_dof_h(void *value, void *clientData)
{
  Skinning * skinning = scast(clientData);
  *(double *)(value) = skinning->arap_dof.h;
}

void TW_CALL SkinningCallback::set_with_dynamics(const void *value, void * clientData)
{
  Skinning * skinning = scast(clientData);
  bool old_value = skinning->arap_dof.with_dynamics;
  skinning->arap_dof.with_dynamics = *(const bool*)(value);
  if( (old_value != skinning->arap_dof.with_dynamics) && skinning->auto_dof)
  {
    skinning->reinitialize_auto_dof();
  }
}

void TW_CALL SkinningCallback::get_with_dynamics(void *value, void *clientData)
{
  Skinning * skinning = scast(clientData);
  *(bool *)(value) = skinning->arap_dof.with_dynamics;
}

void TW_CALL SkinningCallback::reset_puppet(void * clientData)
{
#ifndef NO_PUPPET
  Skinning * skinning = scast(clientData);
  cout<<"Why is this being called"<<endl;
  skinning->pi->reset();
#else
  _unused(clientData);
#endif
}

#ifdef PBS_KAAN
void TW_CALL SkinningCallback::compute_spheres(void * clientData)
{
    Skinning * skinning = static_cast<Skinning *>(clientData);
    skinning->sphere_constructor();
}
#else
void TW_CALL SkinningCallback::compute_spheres(void * /*clientData */){}
#endif
