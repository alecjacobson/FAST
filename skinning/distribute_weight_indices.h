#ifndef DISTRIBUTE_WEIGHT_INDICES_H
#define DISTRIBUTE_WEIGHT_INDICES_H

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
inline bool distribute_weight_indices(
  const std::vector<Bone*> & BR);

// Implementation
#include <list>
#include <cstdio>
#include "gather_bones.h"
#include <igl/verbose.h>

inline bool distribute_weight_indices(
  const std::vector<Bone*> & BR)
{
  using namespace std;
  vector<Bone*> B = gather_bones(BR);
  bool overwrite = true;

  // TODO: Validation should also check that weight indices are in order
  //// Validate any existing weight indices
  //bool has_dupe = false;
  //map<int,int> counter;
  //for(std::vector<Bone*>::iterator bit = B.begin();bit != B.end();bit++)
  //{
  //  int wi = (*bit)->get_wi();
  //  if(wi >= 0)
  //  {
  //    map<int,int>::iterator cit = counter.find(wi);
  //    if(cit == counter.end())
  //    {
  //      has_dupe = true;
  //    }else
  //    {
  //      counter[wi] = 1;
  //    }
  //  }
  //}
  //if(has_dupe)
  //{
  //  overwrite = true;
  //  verbose("^%s: wi contain duplicates: overwriting wis\n",__FUNCTION__);
  //}

  
  // Weight indices are distributed to:
  //  roots with no children
  //  non-roots
  int wi = 0;
  // loop over bones
  for(std::vector<Bone*>::iterator bit = B.begin();bit != B.end();bit++)
  {
    Bone * b = (*bit);
    if( (!b->is_root()) || (b->get_children().size() == 0))
    {
      if(overwrite || b->get_wi() < 0)
      {
        b->set_wi(wi);
        wi++;
      }
    }
  }

  return true;
}

#endif

