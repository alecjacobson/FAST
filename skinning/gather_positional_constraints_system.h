#ifndef GATHER_POSITIONAL_CONSTRAINTS_SYSTEM_H
#define GATHER_POSITIONAL_CONSTRAINTS_SYSTEM_H

#include "Bone.h"
#include <vector>
#include <iostream>

#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>

// Gather positional constraints system entries implied by bone forest
//
// Inputs:
//   BR  list of bone tree roots
//   m  number of handles/weight functions (might be more than the number of
//     weight influencing bones)
//   dim  number of dimensions for constraints, 2 or 3
// Output:
//   Aeq  dim*#constraint_points by m*dim*(dim+1)  matrix of linear equality
//     constraint coefficients. Each row corresponds to a linear constraint, so
//     that Aeq * L = Beq says that the linear transformation entries in the
//     column L should produce the user supplied positional constraints for
//     each handle in Beq. The row Aeq(i*dim+d) corresponds to the constrain on
//     coordinate d of position i
// Returns true on success, false on error
bool gather_positional_constraints_system(
  const std::vector<Bone*> & BR, 
  const int m,
  const int dim,
  Eigen::SparseMatrix<double> & A_eq);

// Append constraints for a given position and influencing weight function (one
// constraint for each dimension)
//
// Inputs:
//   p  >dim position vector
//   m  number of handles/weight functions (might be more than the number of
//     weight influencing bones)
//   dim  number of dimensions for constraints, 2 or 3, if less than dimension
//     of p then first dim coordinates are used
//   A_eq_IJV current A_eq entries (see above) with only num_pc non-zeros rows
//     at the top, and at least dim rows free after those
//   num_pc  current number of non-zero rows at the top of A_eq_IJV
// Outputs:
//   A_eq_IJV  updated A_eq entries
//   num_pc  updated number of non-zero rows in A_eq_IJV
//     (num_pc += dim)
void append_to_constraints_system(
  const Vec3 & p,
  const int wi,
  const int m,
  const int dim,
  std::vector<Eigen::Triplet<double> > & A_eq_IJV,
  int & num_pc);

// creates 'A_fix_eq matrix' corresponding to one-variable constraints
// specified by fixed transformations 'fixed':
void gather_fixed_constraints_system(
  const Eigen::Matrix<int,Eigen::Dynamic,1> &fixed_dim, 
  int dim, 
  int numBones, 
  Eigen::SparseMatrix<double> & A_fix_eq);

// creates 'A_fix_eq matrix' corresponding to one-variable constraints
// specified by fixed transformations 'fixed':
void join_constraints_systems(
  const Eigen::SparseMatrix<double> & A1, 
  const Eigen::SparseMatrix<double> & A2, 
  Eigen::SparseMatrix<double> & Ajoin);

#endif

