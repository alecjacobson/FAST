#ifndef PUPPET_ANGLES_TO_QUAT_H
#define PUPPET_ANGLES_TO_QUAT_H
#ifndef NO_PUPPET

#include "PuppetTopology.h"
#include <vector>
#include "Quat.h"

// Here we assume in a TRT joint the
// first angle θ₁ refers to a CCW rotation R₁ around (1,0,0) [x-axis] the
// second angle θ₂ is a CCW rotation R₂ around R₁(0,0,1) [z-axis rotated by
// R₁] the third angle θ₃ is a CCW rotation R₃ around R₂R₁(1,0,0) [x-axis
// rotated by R₁ then by R₂]
//
// Let's define a "rest state" if all the puppet angles are PI then our joint
// lies along the x-axis like:
//
//   y                     θ₁   θ₂   θ₃
//   |                    |^| /```\ |^|
//   .---> x parent --->  | ||  .  || | ---> child
//  /                     |_| \__L/ |_|
// z
//
// Where increasing θ₁ rotates around x-axis (right-hand rule)
//       increasing θ₂ rotates around z-axis (right-hand rule)
//       increasing θ₃ rotates around x-axis (right-hand rule)
//       and L means the little LED
//
//
// Inputs:
//   t  Node type
//   angles_rad  list of angles in radians
// Returns rotation implied by node's angles as a quaternion
Quat puppet_angles_to_quat(
  const Puppet::NodeType & t,
  const std::vector<float> & angles_rad);
#endif
#endif
