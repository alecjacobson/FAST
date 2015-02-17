#ifndef NO_PUPPET
#include "splitter_rotation.h"
#include <igl/PI.h>
#include "Vec.h"
#include "BoneData.h"
#include <iostream>
#include <PuppetParserRQ.h>

Quat splitter_rotation(
  const Puppet::NodeRecord & p,
  const int child_i)
{
  using namespace igl;
  using namespace std;
  using namespace Puppet;
        cout<<"child_i: "<<child_i<<endl;

  switch(p.type)
  {
    // Non splitters
    case NODE_COLLECTOR:
    case NODE_JOINT_TRT:
    case NODE_JOINT_T:
    case NODE_JOINT_R:
      //assert(false);
      return Quat(1,0,0,0);
    case NODE_SPLITTER_2:
    case NODE_SPLITTER_3:
    case NODE_SPLITTER_5:
      //
      //
      //  parent ---> Split ---> 0
      //                |
      //                v
      //                1 
      switch(child_i)
      {
        case 0:
          return Quat(1,0,0,0);
        case 1:
          return
            Quat(Eigen::AngleAxis<double>(-PI/2,Vec3(1,0,0)));
            //Quat(Eigen::AngleAxis<double>(-PI/2,Vec3(1,0,0))) * 
            //Quat(Eigen::AngleAxis<double>(-PI/2,Vec3(0,0,1)));
        default:
          return Quat(1,0,0,0);
      }
    default:
      return Quat(1,0,0,0);
  }

  //// Spiral joint (XY plane)
  //// Valence of joint p (including its incoming bone)
  //double valence = p.offspring.size() + 1;
  //double theta = (((double)child_i+1)/valence)*2.0*PI;
  //Vec3 axis(0,0,1);
  //return Quat(Eigen::AngleAxis<double>(PI-theta,axis));
}
#endif
