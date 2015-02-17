#include <Eigen/Dense>
// Optimize the indices in a mesh's face list F to reduce cache misses when
// displaying triangles in order
//
// Inputs:
//   F  original face list to be rearranged
//   silent  do not print statistics
// Outputs:
//   OF output face list (can be same as input)
void optimize_index_buffer(
  const Eigen::MatrixXi& F, 
  const bool silent,
  Eigen::MatrixXi& OF);
