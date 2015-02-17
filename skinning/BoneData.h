#ifndef BONEDATA_H
#define BONEDATA_H
#include "Bone.h"
#include "Quat.h"
#include <cassert>

const Vec3 DEFAULT_ROOT_OFFSET(0,0,0);

struct BoneData
{
  Bone * b;
  Quat pframe;
  Quat frame;
  Quat rotation_at_set;

  Quat last_q;
  Vec3 offset_at_set;
  Vec3 bone_vector_at_set;
  //std::vector<float> last_angles_rad;
  public:
  BoneData(Bone * b_):b(b_)
  {
    assert(b != NULL);
    // Default is identity
    frame = Quat(1,0,0,0);
    last_q = Quat(1,0,0,0);
  }
  //BoneData(const BoneData & other)
  //{
  //  b = other.b;
  //  last_angles_rad = other.last_angles_rad;
  //}
};

#endif
