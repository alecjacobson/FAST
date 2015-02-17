#ifdef PBS_KAAN
#include "collision_spheres.h"
#include "spheres_collide_with_floor.h"

#include <igl/repdiag.h>
#include <vector>
#include <iostream>
template <typename Derivedfext>
void spheres_collide_with_floor(
  const SpheresVector & SC,
  const double K,
  const double p,
  const double collisions_param,
  const double floor_depth,
  Eigen::PlainObjectBase<Derivedfext> & fext)
{
  using namespace std;
  using namespace Eigen;
  using namespace igl;


  //const int dim = 3;
  //const int n = M.rows()/3;
  //// Compute deformed mesh U
  //MatrixXd U3 = M*Tcol;
  //MatrixXd U(n,3);
  //U.col(0) = U3.block(0,0,n,1);
  //U.col(1) = U3.block(n,0,n,1);
  //U.col(2) = U3.block(2*n,0,n,1);

  // Initial force is zero
  fext.setZero(fext.rows(),fext.cols());
  //MatrixXd fextV = MatrixXd::Zero(n*3,1);

  const double eff_floor_depth = floor_depth+collisions_param;

  // clear external forces

  //SparseMatrix<double> Mass_rep;
  //repdiag(Mass,3,Mass_rep);
  //MatrixXd MM = Mass_rep * M;
  //MatrixXd MMT = MM.transpose();

  // loop over spheres
  for(int i = 0;i<(int)SC.size();i++)
  {
    //if(SC[i]->deformed_sphere.center.y() < eff_floor_depth)
    //{
      assert(Spheres::M_map.count(SC[i])==1);
      assert(Spheres::vertex_map.count(SC[i])==1);

      double d = fabs(SC[i]->deformed_sphere.center.y()-eff_floor_depth);
      //MatrixXd fextVi = MatrixXd::Zero(n*3,1);
      //std::vector<int> vlist = Spheres::vertex_map[SC[i]];
      //// loop over vertices
      //for(int j = 0;j<vlist.size();j++)
      //{
      //  if(U(vlist[j],1) < eff_floor_depth)
      //  {
      //    ////double d = fabs(U(vlist[j],1)-eff_floor_depth);
      //    //fextV(vlist[j]+1*n,0) = K*pow(d,p);
      //  }
      //  fextVi(vlist[j]+1*n,0) = 1;
      //}
      //fextVi.array() *= K*pow(d,p);
      //fextV.array() += fextVi.array();
      //fext.array() += ((MMT * fextVi).eval().array() * (K * pow(d,p))).eval();
      //const int m = Spheres::M_map[SC[i]].rows();
      const double mag = K * pow(d,p);
      if(mag>10 || mag != mag)
      {
        cout<<"FUCK"<<endl;
      }
      fext.array() += ((Spheres::M_map[SC[i]].col(1).array()) * mag).eval().cast<typename Derivedfext::Scalar>();
      //fextV.array() = fextV.array() + 
      //  (K*pow(fabs(SC[i]->deformed_sphere.center.y()-eff_floor_depth),p)*
      //  fextVi.array()).eval();
      //fext.array() += Spheres::M_map[SC[i]].col(1).array() *
      //  K*pow(fabs(SC[i]->deformed_sphere.center.y()-eff_floor_depth),p);
    //}
  }

  //fext = MM.transpose().eval() * fextV;

  //fext = (fextV.transpose() * Mass_rep *  M).transpose().eval();
  //Eigen::MatrixXd MM = (Mass_rep *  M).transpose().eval();
  //fext = MM * fextV;
  //cout<<"fext["<<endl<<fext<<endl<<"];"<<endl;

}
template void spheres_collide_with_floor<Eigen::Matrix<float, -1, -1, 0, -1, -1> >(std::vector<Spheres*, std::allocator<Spheres*> > const&, double, double, double, double, Eigen::PlainObjectBase<Eigen::Matrix<float, -1, -1, 0, -1, -1> >&);
#endif
