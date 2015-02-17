#include <AntTweakBar.h>
class Skinning;

// This contains a number of callbacks which an instance of the Skinning class
// as the expected clientData argument. This is useful to "piggyback" into an
// instance of a class from a "static" method
namespace SkinningCallback
{
  // Utilties
  // Static cast a void * to a Skinning *
  inline Skinning * scast(void *clientData);

  // Callbacks
  // No-op setter, does nothing
  void TW_CALL no_op(const void *value, void *clientData);
  // No-op getter, does nothing
  void TW_CALL no_op(void *value, void *clientData);

  // Snap clientData->object_rotation to XY plane quaternion
  void TW_CALL view_xy_plane(void *clientData);
  // Snap clientData->object_rotation to XZ plane quaternion
  void TW_CALL view_xz_plane(void *clientData);
  // Snap clientData->object_rotation to YZ plane quaternion
  void TW_CALL view_yz_plane(void *clientData);
  // Set pan to zero
  void TW_CALL zero_pan(void *clientData);
  // Check zoom lock before seting new zoom value
  void TW_CALL set_zoom(const void *value, void *clientData);
  // Get current zoom value
  void TW_CALL get_zoom(void *value, void *clientData);
  // Check sangle lock before seting new angle value
  void TW_CALL set_angle(const void *value, void *clientData);
  // Get current angle value
  void TW_CALL get_angle(void *value, void *clientData);
  // Get current number of vertices
  void TW_CALL get_num_vertices(void * value, void *clientData);
  // Get current number of faces
  void TW_CALL get_num_faces(void * value, void *clientData);
  // Restart spin timer
  void TW_CALL set_spinning_about_up_axis(const void *value, void *clientData);
  // Get current spinning_about_up_axis value
  void TW_CALL get_spinning_about_up_axis(void *value, void *clientData);
  // Snap current rotation to canonical view quaternion
  void TW_CALL snap_rotation_to_canonical_view_quat(void *clientData);
  // Clear the screen
  void TW_CALL clear(void *clientData);
  // Get weight index of selected bone
  void TW_CALL get_selected_bone_wi(void *value, void *clientData);
  // Set and get the selected bone's rotation
  void TW_CALL set_selected_bone_rotation(const void *value, void *clientData);
  void TW_CALL get_selected_bone_rotation(void *value, void *clientData);
  // Set and get the selected bone's translation
  void TW_CALL set_selected_bone_translation_x(const void *value, void *clientData);
  void TW_CALL get_selected_bone_translation_x(void *value, void *clientData);
  void TW_CALL set_selected_bone_translation_y(const void *value, void *clientData);
  void TW_CALL get_selected_bone_translation_y(void *value, void *clientData);
  void TW_CALL set_selected_bone_translation_z(const void *value, void *clientData);
  void TW_CALL get_selected_bone_translation_z(void *value, void *clientData);
  void TW_CALL set_selected_bone_dof_type(const void *value, void *clientData);
  void TW_CALL get_selected_bone_dof_type(void *value, void *clientData);
  void TW_CALL set_selected_bone_last_T(const void *value, void *clientData);
  void TW_CALL get_selected_bone_last_T(void *value, void *clientData);
  // Snap current selection bone's rotation to canonical view quaternion
  void TW_CALL snap_selected_bone_rotation_to_canonical_view_quat(void *clientData);
  void TW_CALL select_all(void *clientData);
  // Reset selected bone transformations to identity
  void TW_CALL reset_selected_bone_transformation(void *clientData);
  // Reset extra transformations
  void TW_CALL reset_extra_transformations(void *clientData);
  // Reset all bone transformations to identity
  void TW_CALL reset_all_bone_transformations(void *clientData);
  void TW_CALL set_auto_dof(const void *value, void *clientData);
  void TW_CALL get_auto_dof(void *value, void *clientData);
  // Prints the variable T holding the current transformations
  void TW_CALL print_transformations(void *clientData);
  // Clears any extra weights
  void TW_CALL clear_extra_weights(void *clientData);
  void TW_CALL set_animate(const void *value, void *clientData);
  void TW_CALL get_animate(void *value, void *clientData);
  void TW_CALL get_animation_size(void *value, void *clientData);
  void TW_CALL push_keyframe(void *clientData);
  void TW_CALL pop_keyframe(void *clientData);
  void TW_CALL set_color_option(const void *value, void *clientData);
  void TW_CALL get_color_option(void *value, void *clientData);
  void TW_CALL set_bypass_auto_dof(const void *value, void *clientData);
  void TW_CALL get_bypass_auto_dof(void *value, void *clientData);
  void TW_CALL set_corner_threshold(const void *value, void *clientData);
  void TW_CALL get_corner_threshold(void *value, void *clientData);
  void TW_CALL set_normal_type(const void *value, void *clientData);
  void TW_CALL get_normal_type(void *value, void *clientData);
  void TW_CALL set_use_texture_mapping(const void *value, void *clientData);
  void TW_CALL get_use_texture_mapping(void *value, void *clientData);
  void TW_CALL clear_bone_roots(void *clientData);
  void TW_CALL set_editing(const void *value, void *clientData);
  void TW_CALL get_editing(void *value, void *clientData);
  void TW_CALL delete_selected_bones(void *clientData);
  void TW_CALL compute_weights(void *clientData);
  void TW_CALL set_dial_in_each_T(const void *value, void *clientData);
  void TW_CALL get_dial_in_each_T(void *value, void *clientData);
  void TW_CALL set_arap_dof_h(const void *value, void *clientData);
  void TW_CALL get_arap_dof_h(void *value, void *clientData);
  void TW_CALL set_with_dynamics(const void *value, void *clientData);
  void TW_CALL get_with_dynamics(void *value, void *clientData);
  // Send reset to puppet
  void TW_CALL reset_puppet(void * clientData);
    
    void TW_CALL compute_spheres(void *clientData);
};

