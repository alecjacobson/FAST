#ifndef PROJECT_EXTRA_COMPACT_H
#define PROJECT_EXTRA_COMPACT_H

#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Sparse>
#include <Eigen/Dense>
// project_EXTRA_COMPACT use initial weights as embedding and there
// compute smooth voronoi weights for each handle (with the original handles
// and offset versions of all other new handles weights as zeros)
// 
// EW = project_extra_compact(W,k,push)
//
// Inputs:
//  W  weights for existing handles, #V by #W matrix of weights
//  k  number of extra weights
//  push  factor by which extra weight seeds are push away when setting
//    zeros (pairwise voronoi weights), {1} 
// Outputs:
//   #V by k matrix of extra weights
// 
void project_extra_compact(
  const Eigen::MatrixXd & W,
  const int k,
  const double push,
  Eigen::MatrixXd & EW);

// Implementation

// STL
#include <map>
#include <utility>

// IGL
#include <igl/colon.h>
#include <igl/slice.h>
#include <igl/project_to_line.h>
#include <igl/partition.h>

#include "project_connect.h"

void project_extra_compact(
  const Eigen::MatrixXd & W,
  const int k,
  const double push,
  Eigen::MatrixXd & EW)
{
  assert(k > 0);
  // number of domain vertices
  int n = W.rows();
  // number of original weights
  int nc = W.cols();
  // Get seeds in weight space for originals and new weights
  Eigen::Matrix<int,Eigen::Dynamic,1> G;
  Eigen::Matrix<int,Eigen::Dynamic,1> S;
  Eigen::Matrix<double,Eigen::Dynamic,1> GD;
  igl::partition(W,k+nc,G,S,GD);

  // Get weights of seeds
  Eigen::MatrixXd WS;
  igl::slice(W,S,igl::colon<int>(0,W.cols()-1),WS);

  // Build adjacency matrix of neighbors determined by projection
  //Eigen::DynamicSparseMatrix<double, Eigen::RowMajor> A;
  std::map<std::pair<int,int>, bool> A;
  project_connect(WS,A);

  // resize output
  EW.setOnes(n,k);

  // loop over new weights
  for(int fi = 0; fi < k; fi++)
  {
    // Original weights at this extra weight seed
    Eigen::VectorXd Wfi = W.row(S(nc+fi));
#ifdef EXTREME_VERBOSE
    std::cout<<"Wfi = ["<<Wfi.transpose()<<"];"<<std::endl;
#endif
    // loop over all other seeds (originals and other extras)
    for(int oi = 0; oi < (k+nc); oi++)
    {
      // skip self
      if(oi == (fi+nc))
      {
        continue;
      }
      // only consider neighbors
      std::map<std::pair<int,int>, bool>::iterator found = 
        A.find(std::pair<int,int>(fi+nc,oi));
      if(found != A.end())
      {
        // Weights at other seed
        Eigen::VectorXd Woi = W.row(S(oi));
        if(oi >= nc)
        {
          Woi = push*Woi - Wfi;
        }
        Eigen::VectorXd t;
        Eigen::VectorXd sqr_d;
#ifdef EXTREME_VERBOSE
        std::cout<<"  Woi = ["<<Woi.transpose()<<"];"<<std::endl;
#endif
        igl::project_to_line(W,Woi,Wfi,t,sqr_d);
        // Clamp to [0,1]
        t = (t.array() < 0).select(Eigen::VectorXd::Zero(t.rows(),t.cols()),t);
        t = (t.array() > 1).select(Eigen::VectorXd::Ones(t.rows(),t.cols()),t);
        // smooth
        t = (-2*(t.array().pow(3))+3*(t.array().pow(2))).matrix();
        // multiply into running product
        EW.col(fi) = (EW.col(fi).array() < t.array()).select(EW.col(fi),t);
        // No NaNs
        assert(EW.col(fi) == EW.col(fi));
      }
    }
  }
}

#endif
