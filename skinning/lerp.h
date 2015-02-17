#ifndef LERP_H
#define LERP_H

#include "BoneBoneCopyMap.h"
#include "igl/normalize_quat.h"
#include "Camera.h"

// Linearly interpolate the transformations of each bone in the values of A and
// B using the factor f and store the result in the transformations of the key
// (A and B should have the same keys)
// 
// Inputs:
//   A  map of bone pointers to bone copies
//   B  map of bone pointers to bone copies
//   f  lerp factor ==0 results in using transformations in A, ==1 those in B,
//     and in between is interpolated
// Returns false on error
inline bool lerp(BoneBoneCopyMap & A, BoneBoneCopyMap & B, const double f);

// Linearly interpolate the transformations of bones A and B using the factor f
// and store the result in the transformation of C
// 
// Inputs:
//   A  pointer to bone when f ==0
//   B  pointer to bone when f ==1
//   f  lerp factor 
//   C  pointer to bone who recieves result of lerp
// Returns false on error
inline void lerp(const Bone * A, const Bone * B, const double f, Bone * C);


// Linearly interpolate the rotations A and B using the factor f
// and store the result in C
// 
// Inputs:
//   A  rotation when f ==0
//   B  rotation when f ==1
//   f  lerp factor 
// Returns interpolated rotation
inline Quat lerp(const Quat & A, const Quat & B, const double f);

// Linearly interpolate the vectors A and B using the factor f
// and store the result in C
// 
// Inputs:
//   A  vectors when f ==0
//   B  vectors when f ==1
//   f  lerp factor 
// Returns interpolated vector
inline Vec3 lerp(const Vec3 & A, const Vec3 & B, const double f);

// Linearly interpolate the scalars A and B using the factor f
// and store the result in C
// 
// Inputs:
//   A  scalar when f ==0
//   B  scalar when f ==1
//   f  lerp factor 
// Returns interpolated scalar 
template <typename Scalar>
inline Scalar lerp(const Scalar & A, const Scalar & B, const double f);

// Implementation

inline bool lerp(BoneBoneCopyMap & A, BoneBoneCopyMap & B, const double f)
{
  assert(A.size() == B.size());
  // loop over keys in A
  for(BBCMIterator ait = A.begin(); ait != A.end(); ait++)
  {
    Bone * key = ait->first;
    // B must contain key as a key
    assert(B.find(key) != B.end());
    Bone * a = &(ait->second);
    Bone * b = &(B.find(key)->second);
    lerp(a,b,f,key);
  }
  return true;
}

inline void lerp(const Bone * A, const Bone * B, const double f, Bone * C)
{
  C->rotation = lerp(A->rotation,B->rotation,f);

  Vec3 tail = C->offset;
  if(!C->is_root())
  {
    tail = C->get_parent()->rest_tip();
  }
  // Lerp translations minus rotational part
  Vec3 At( (A->rotation*(A->translation + tail))-tail);
  Vec3 Bt( (B->rotation*(B->translation + tail))-tail);
  Vec3 Ct = lerp(At,Bt,f);
  C->last_T.matrix() = lerp(A->last_T.matrix(),B->last_T.matrix(),f);
  C->translation = ((C->rotation.inverse() * (tail+Ct))) - tail;
  // Lerp dof type, because lerp is also used to copy
  C->tip_dof_type = (f<0.5? A->tip_dof_type : B->tip_dof_type);
}

inline Quat lerp(const Quat & A, const Quat & B, const double f)
{
  Quat q(
    lerp(A.w(),B.w(),f),
    lerp(A.x(),B.x(),f),
    lerp(A.y(),B.y(),f),
    lerp(A.z(),B.z(),f));
  q.normalize();
  return q;
}

inline Vec3 lerp(const Vec3 & A, const Vec3 & B, const double f)
{
  return (1-f)*A + f*B;
}

template <typename Scalar>
inline Scalar lerp(const Scalar & A, const Scalar & B, const double f)
{
  return A*(1-f) + B*f;
}

inline Camera lerp(const Camera & A, const Camera & B, const double f)
{
  Camera C;
  for(int i = 0;i<3;i++)
  {
    C.pan[i] = lerp(A.pan[i],B.pan[i],f);
  }
  for(int i = 0;i<4;i++)
  {
    C.rotation[i] = lerp(A.rotation[i],B.rotation[i],f);
  }
  igl::normalize_quat<double>(C.rotation,C.rotation);
  C.zoom = lerp(A.zoom,B.zoom,f);
  C.angle = lerp(A.angle,B.angle,f);
  return C;
}


#endif
