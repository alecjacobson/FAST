class Bone;
// Traverse all trees in BR and keep track of the max wi weight index value
// Inputs:
//  BR  list of bone tree roots
// Returns <-1 if BR is empty, -1 if all bones have undefined weight index or
// value >0
inline int max_weight_index(const std::vector<Bone> & BR);

// Implementation
#include "Bone.h"
inline int max_weight_index(const std::vector<Bone> & BR)
{
  int max_wi = -2;
  std::vector<Bone*> B = gather_bones(BR);
  // loop over bones
  for(std::vector<Bone*>::iterator bit = B.begin();bit != B.end();bit++)
  {
    Bone * b = (*bit);
    max_wi = max(b->wi,max_wi);
  }
  return max_wi;
}
