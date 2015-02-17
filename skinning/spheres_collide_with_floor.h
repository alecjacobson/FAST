#include "collision_spheres.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
// Collide a mesh with the floor and return external penalty forces
//
// Inputs:
//   Mass #V by #V mass matrix
//   Tcol  #T by 1 list of transformation entries
//   M  #V by #T  lbs matrix (see lbs_matrix)
//   K  penalty force spring constant
//   p  "spring power"
//   floor_depth  y-offest value of floor
// Outputs:
//   fext  #T by 1 list of external forces on T
template <typename Derivedfext>
void spheres_collide_with_floor(
  const SpheresVector & SC,
  const double K,
  const double p,
  const double collisions_param,
  const double floor_depth,
  Eigen::PlainObjectBase<Derivedfext> & fext);
