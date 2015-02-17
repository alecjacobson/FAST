#ifndef NO_PUPPET
#include "puppet_angles_to_quat.h"
#include "Vec.h"
#include "Tform.h"
#include <igl/PI.h>

Quat puppet_angles_to_quat(
  const Puppet::NodeType & t,
  const std::vector<float> & angles_rad)
{
  using namespace Puppet;
  using namespace std;
  using namespace igl;
  // Start with identity
  switch(t)
  {
    case NODE_JOINT_TRT:
    {
      //assert(angles_rad.size() == 3);
      //Vec3 axis0(1,0,0);
      //Quat q0(Eigen::AngleAxis<double>(angles_rad[0]-PI,axis0));
      //Vec3 axis1 = Tform3(q0)*Vec3(0,0,1);
      //Quat q1(Eigen::AngleAxis<double>(angles_rad[1]-PI,axis1));
      //Vec3 axis2 = Tform3(q1)*Tform3(q0)*Vec3(1,0,0);
      //Quat q2(Eigen::AngleAxis<double>(angles_rad[2]-PI,axis2));
      //return q2*q1*q0;
      Quat q0(Eigen::AngleAxis<double>(angles_rad[0]-PI,Vec3(1,0,0)));
      Quat q1(Eigen::AngleAxis<double>(angles_rad[1]-PI,Vec3(0,0,1)));
      Quat q2(Eigen::AngleAxis<double>(angles_rad[2]-PI,Vec3(1,0,0)));
      return q0*q1*q2;
    }
    default:
      // not supported
      assert(false);
  }
  return Quat(1,0,0,0);
}

#endif
