#include "Bone.h"
#include <vector>

// Gather transformations from bone forest roots into stack of affine
// transformations
// Inputs:
//   BR  list of bone tree roots
//   use_last_T  use last_T transformation instead of .affine()
//   T  #rows by 4*num_handles horizontal stack of transformations
// Output:
//   T  #rows by 4*num_handles horizontal stack of transformations
// Returns true on success, false on error
inline bool gather_transformations(
  const std::vector<Bone*> & BR, 
  const bool use_last_T,
  Eigen::MatrixXf & T);

// Implementation
#include "Tform.h"
#include <list>
#include "gather_bones.h"

inline bool gather_transformations(
  const std::vector<Bone*> & BR, 
  const bool use_last_T,
  Eigen::MatrixXf & T)
{
  int num_handles = T.cols()/4;
  std::vector<Bone*> B = gather_bones(BR);
  // loop over bones
  for(std::vector<Bone*>::iterator bit = B.begin();bit != B.end();bit++)
  {
    Bone * b = (*bit);
    if(b->wi_is_set())
    {
      int wi = b->get_wi();
      if(wi >= num_handles)
      {
        fprintf(
          stderr,
          "Error: gather_transformations() max weight index of bones (%d)"
          " >= number of handles (%d)\n",
          wi,
          num_handles);
        return false;
      }
      if(wi >= 0)
      {
        // compute current transformation at bone
        Tform3 bT;
        if(use_last_T)
        {
          bT = b->last_T;
        }else
        {
          bT = b->affine();
        }
        // place transform into T stack
        T.block(0,b->get_wi()*4,3,4) = bT.affine().matrix().cast<float>();
      }
    }
  }

  return true;
}
