#ifndef TFORM_H
#define TFORM_H
#include <Eigen/Geometry>

// typedef for transforms so it's easier to switch between float and double if
// need be
typedef Eigen::Transform<double,3,Eigen::Affine> Tform3;

#endif

