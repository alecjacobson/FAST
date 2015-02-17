#include "boundary_conditions.h"
#include "gather_bones.h"
#include "gather_positions_and_connectivity.h"

#include <igl/project_to_line.h>
#include <igl/EPS.h>
#include <igl/verbose.h>
#include <igl/slice_into.h>
#include <igl/colon.h>
#include <igl/boundary_conditions.h>

#include <vector>
#include <iostream>

bool boundary_conditions(
  const Eigen::MatrixXd & V,
  const Eigen::MatrixXi & Ele,
  const std::vector<Bone*> & BR, 
  Eigen::VectorXi & b,
  Eigen::MatrixXd & bc)
{
  using namespace std;
  using namespace Eigen;
  using namespace igl;

  MatrixXd C;
  VectorXi P;
  MatrixXi BE;
  MatrixXi CE;
  VectorXi WI;
  gather_positions_and_connectivity(BR,C,P,BE,WI);

  // Compute boundary conditions in C,P,BE,CE style
  Eigen::MatrixXd bc_temp;
  bool ret = boundary_conditions(V,Ele,C,P,BE,CE,b,bc_temp);

  if(!ret)
  {
    return false;
  }
  // But now columns are ordered according to [P,BE]
  bc.resize(bc_temp.rows(),bc_temp.cols());
  slice_into(bc_temp,colon<int>(0,bc.rows()-1),WI,bc);
  return ret;

  //vector<int> bci;
  //vector<int> bcj;
  //vector<int> bcv;

  //vector<Bone*> B = gather_bones(BR);
  //int max_wi = -1;
  //// loop over bones
  //for(vector<Bone*>::iterator bit = B.begin();bit != B.end();bit++)
  //{
  //  Bone * bone = (*bit);
  //  //Vec3 tip = bone->tip_as_drawn();
  //  Vec3 tip = bone->rest_tip();
  //  int wi = bone->get_wi();
  //  max_wi = (wi>max_wi?wi:max_wi);
  //  if(wi >= 0)
  //  {
  //    // loop over domain vertices
  //    for(int i = 0;i<V.rows();i++)
  //    {
  //      if(bone->is_root())
  //      {
  //        // Find samples just on tip
  //        Vec3 vi(V(i,0),V(i,1),V(i,2));
  //        double sqrd = (vi-tip).array().pow(2).sum();
  //        if(sqrd <= FLOAT_EPS)
  //        {
  //          bci.push_back(i);
  //          bcj.push_back(wi);
  //          bcv.push_back(1.0);
  //        }
  //      }else
  //      {
  //        // Find samples from tip up to tail
  //        //Vec3 tail = bone->tail_as_drawn();
  //        Vec3 tail = bone->rest_tail();
  //        // Compute parameter along bone and squared distance
  //        double t,sqrd;
  //        project_to_line(
  //          V(i,0),V(i,1),V(i,2),
  //          tip(0),tip(1),tip(2),
  //          tail(0),tail(1),tail(2),
  //          t,sqrd);
  //        if(t>=-FLOAT_EPS && t<=(1.0f+FLOAT_EPS) && sqrd<=FLOAT_EPS)
  //        {
  //          bci.push_back(i);
  //          bcj.push_back(wi);
  //          bcv.push_back(1.0);
  //        }
  //      }
  //    }
  //  }
  //}

  //if(max_wi < 0)
  //{
  //  verbose("^%s: Error: no weight indices found\n",__FUNCTION__);
  //  return false;
  //}

  //// find unique boundary indices
  //vector<int> vb = bci;
  //sort(vb.begin(),vb.end());
  //vb.erase(unique(vb.begin(), vb.end()), vb.end());

  //b.resize(vb.size());
  //bc = MatrixXd::Zero(vb.size(),max_wi+1);
  //// Map from boundary index to index in boundary
  //map<int,int> bim;
  //int i = 0;
  //// Also fill in b
  //for(vector<int>::iterator bit = vb.begin();bit != vb.end();bit++)
  //{
  //  b(i) = *bit;
  //  bim[*bit] = i;
  //  i++;
  //}

  //// Build BC
  //for(i = 0;i < (int)bci.size();i++)
  //{
  //  assert(bim.find(bci[i]) != bim.end());
  //  bc(bim[bci[i]],bcj[i]) = bcv[i];
  //}

////#ifdef EXTREME_VERBOSE
////#endif

  //// Normalize accross rows so that conditions sum to one
  //for(i = 0;i<bc.rows();i++)
  //{
  //  double sum = bc.row(i).sum();
  //  assert(sum != 0);
  //  bc.row(i).array() /= sum;
  //}

  //// Check that every Weight function has at least one boundary value of 1 and
  //// one value of 0
  //for(i = 0;i<bc.cols();i++)
  //{
  //  double min_abs_c = bc.col(i).array().abs().minCoeff();
  //  double max_c = bc.col(i).maxCoeff();
  //  if(min_abs_c > FLOAT_EPS)
  //  {
  //    verbose("^%s: Error: handle %d does not receive 0 weight\n",__FUNCTION__,i);
  //    return false;
  //  }
  //  if(max_c< (1-FLOAT_EPS))
  //  {
  //    verbose("^%s: Error: handle %d does not receive 1 weight\n",__FUNCTION__,i);
  //    return false;
  //  }
  //}

  //return true;
}
