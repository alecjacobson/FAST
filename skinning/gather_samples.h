#ifndef GATHER_SAMPLES_H
#define GATHER_SAMPLES_H

#include "Bone.h"
#include <vector>

// Gather samples from bone forest roots using the current draw tips
// For each root we add 1 sample. For each non-root add k-1 samples starting at
// the tip working back toward the tail.
//
// Inputs:
//   BR  list of bone tree roots
//   k number of samples
// Output:
//   S  #samples by #dim 
// Returns true on success, false on error
inline bool gather_samples(
  const std::vector<Bone*> & BR, 
  const int k,
  Eigen::MatrixXd & S);



// Implementation

#include "Vec.h"
#include "gather_bones.h"
#include "gather_positions_and_connectivity.h"

#include <igl/sample_edges.h>
#include <Eigen/Dense>

#include <list>
#include <cstdio>

inline bool gather_samples(
  const std::vector<Bone*> & BR, 
  const int k,
  Eigen::MatrixXd & S)
{
  using namespace Eigen;
  using namespace igl;

  MatrixXd V;
  VectorXi P;
  MatrixXi BE;
  VectorXi WI;
  gather_positions_and_connectivity(BR,V,P,BE,WI);
  sample_edges(V,BE,k,S);

  //std::vector<Bone*> B = gather_bones(BR);
  //S.resize(BR.size() + (B.size()-BR.size())*(k-1),3);
  //int j =0;
  //// loop over bones
  //for(std::vector<Bone*>::iterator bit = B.begin();bit != B.end();bit++)
  //{
  //  Bone * b = (*bit);
  //  //Vec3 tip = b->tip_as_drawn();
  //  Vec3 tip = b->rest_tip();
  //  if(!b->is_root())
  //  {
  //    // Get k-1 samples toward tail
  //    //Vec3 tail = b->tail_as_drawn();
  //    Vec3 tail = b->rest_tail();
  //    for(int i=1;i<(k-1);i++)
  //    {
  //      double f = ((double)i)/((double)k-1.0);
  //      //S.push_back(f*tail + (1.0-f)*tip);
  //      S.row(j) = f*tail + (1.0-f)*tip;
  //      j++;
  //    }
  //  }
  //  //S.push_back(tip);
  //  S.row(j) = tip;
  //  j++;
  //}

  return true;
}

#endif
