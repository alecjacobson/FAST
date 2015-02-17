#ifndef GATHER_BONES_H
#define GATHER_BONES_H
#include "Bone.h"
#include <vector>
#include <list>
// Gather a list of pointers to all bone roots and their ancestors from a list
// of bones
//
// It's your funeral if your bone roots have closed loops
//
// Inputs:
//   BR  list of bone roots
// Returns list of bone pointers in what should be considered RANDOM ORDER
//
inline std::vector<Bone*> gather_bones(const std::vector<Bone*> & BR);

inline std::vector<Bone*> gather_bones(const std::vector<Bone*> & BR)
{
  // Insert roots into search queue
  std::list<Bone*> Q;
  for(
    std::vector<Bone*>::const_iterator bit = BR.begin();
    bit != BR.end();
    bit++)
  {
    Q.push_back(*bit);
  }
  // Keep track of all bones that get popped 
  std::vector<Bone*> B;
  while(!Q.empty())
  {
    // Read from front because we want to keep order QUEUE
    Bone * b = Q.front();
    Q.pop_front();
    // Add to list
    B.push_back(b);
    // Add children to queue
    std::vector<Bone*> children = b->get_children();
    Q.insert(Q.end(),children.begin(),children.end());
  }
  return B;
}
#endif
