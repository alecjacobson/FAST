#include <Eigen/Dense>
#include "Bone.h"

// Compute boundary conditions for automatic weights computation
// Inputs:
//   V  #V by dim list of domain vertices
//   Ele  #Ele by simplex-size list of simplex indices
// Outputs:
//   b  #b list of boundary indices (indices into V of vertices which have
//     known, fixed values)
//   bc #b by #weights list of known/fixed values for boundary vertices (notice
//     the #b != #weights in general because #b will include all the
//     intermediary samples along each bone, etc.
// Returns true if boundary conditions make sense
bool boundary_conditions(
  const Eigen::MatrixXd & V,
  const Eigen::MatrixXi & Ele,
  const std::vector<Bone*> & BR, 
  Eigen::VectorXi & b,
  Eigen::MatrixXd & bc);
