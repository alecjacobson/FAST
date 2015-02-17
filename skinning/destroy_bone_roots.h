#ifndef DESTROY_BONE_ROOTS_H
#define DESTROY_BONE_ROOTS_H
#include "Bone.h"
#include <vector>
// Properly destroy a bone forest via a list of its roots
// 
// Inputs:
//   BR  list of bone roots
inline void destroy_bone_roots(std::vector<Bone*> & BR);

// Implemention

inline void destroy_bone_roots(std::vector<Bone*> & BR)
{
  // Clear bone roots list
  for(std::vector<Bone*>::iterator bit = BR.begin();
      bit != BR.end(); bit++)
  {
    delete *bit;
  }
  BR.clear();
}

#endif
