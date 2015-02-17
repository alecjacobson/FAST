#include "naive_collide_with_floor.h"
#include <igl/print_ijv.h>
#include <igl/repdiag.h>

#include <iostream>
template <typename LbsMatrixType,typename Derivedfext>
void naive_collide_with_floor(
  const Eigen::SparseMatrix<double> & Mass,
  const Eigen::VectorXd & Tcol,
  const LbsMatrixType & M,
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
  const int n = M.rows()/3;
  // Compute deformed mesh U
  MatrixXd U3 = M*Tcol;
  MatrixXd U(n,3);
  U.col(0) = U3.block(0,0,n,1);
  U.col(1) = U3.block(n,0,n,1);
  U.col(2) = U3.block(2*n,0,n,1);

  // Initial force is zero
  MatrixXd fextV = MatrixXd::Zero(n*3,1);

  //// Total mass below real floor
  //VectorXd below(VectorXd::Zero(n));
  //for(int i = 0;i<n;i++)
  //{
  //  if(U(i,1) < floor_depth)
  //  {
  //    below(i) = 1;
  //  }
  //}
  const double m = 1;//(Mass * below).eval().sum();

  const double eff_floor_depth = floor_depth+m*collisions_param;

  // loop over vertices
  for(int i = 0;i<n;i++)
  {
    //if(U3(i+1*n,0) < floor_depth)
    //{ 			
    //  fextV(i+1*n,0) = K*pow(fabs(U3(i+1*n,0)-floor_depth),p);
    //}
    if(U(i,1) < eff_floor_depth)
    {
      fextV(i+1*n,0) = K*pow(fabs(U(i,1)-eff_floor_depth),p);
    }
  }

  SparseMatrix<double> Mass_rep;
  repdiag(Mass,3,Mass_rep);
  //fext = (fextV.transpose() *  M).transpose().eval();
  MatrixXd fextd = (fextV.transpose() * Mass_rep *  M).transpose().eval();
  fext.resize(fextd.rows(),fextd.cols());
  fext = fextd.cast<typename Derivedfext::Scalar>();
  //cout<<"fext["<<endl<<fext<<endl<<"];"<<endl;
}

// Explicit template instanciation
template void naive_collide_with_floor<Eigen::Matrix<double, -1, -1, 0, -1, -1>, Eigen::Matrix<float, -1, -1, 0, -1, -1> >(Eigen::SparseMatrix<double, 0, int> const&, Eigen::Matrix<double, -1, 1, 0, -1, 1> const&, Eigen::Matrix<double, -1, -1, 0, -1, -1> const&, double, double, double, double, Eigen::PlainObjectBase<Eigen::Matrix<float, -1, -1, 0, -1, -1> >&);
