#ifndef ISO_EXTRA_H
#define ISO_EXTRA_H

#include <Eigen/Dense>
#include "Ease.h"

// ISO_EXTRA Use initial weights as embedding for isotropic "bump" weights
//
// EW = iso_extra(W,k)
// EW = iso_extra(W,k,'ParameterName',ParameterValue)
// 
// Inputs:
//  W  weights for existing handles, #V by #W matrix of weights
//  F  #F by 3 indices of triangles
//  k  number of extra weights
//  push  factor by which extra weight seeds are push away when setting
//    zeros (pairwise voronoi weights), {1} 
//  ease  type of ease filter to use to make bumps, see Ease.h
// Outputs:
//  EW  #V by k matrix of extra weights
// 
void iso_extra(
  const Eigen::MatrixXd & W,
  const Eigen::MatrixXi & F,
  const int k,
  const double push,
  Ease ease,
  Eigen::MatrixXd & EW);

// Implementation 
#include <igl/colon.h>
#include <igl/slice.h>
#include <functional>
#include <igl/partition.h>
#include <igl/uniformly_sample_two_manifold.h>

void iso_extra(
  const Eigen::MatrixXd & W,
  const Eigen::MatrixXi & F,
  const int k,
  const double push,
  Ease ease,
  Eigen::MatrixXd & EW)
{
  using namespace Eigen;
  using namespace igl;
  using namespace std;
  assert(k > 0);
  // number of domain vertices
  int n = W.rows();

  // Get weights of seeds
  MatrixXd WS;
  uniformly_sample_two_manifold(W,F,k,push,WS);

  assert(WS.rows() == k);

#ifdef EXTREME_VERBOSE
  cout<<"WS=["<<endl<<WS<<endl<<"];"<<endl;
#endif

  // Get weights of originals (corners of weight space)
  MatrixXd I = MatrixXd::Identity(W.cols(),W.cols());

  // resize output
  EW.resize(n,k);
  // loop over new handles
  int added_i = 0;
  for(int i = 0;i < k;i++)
  { 
    Matrix<double,1,Dynamic> sw = WS.row(i);
#ifdef EXTREME_VERBOSE
    cout<<"sw=["<<endl<<sw<<endl<<"];"<<endl;
#endif
    // find closest original seed and use distance as potential radius
    double r =
      sqrt((I.rowwise()-sw).array().pow(2).rowwise().sum().minCoeff());
#ifdef EXTREME_VERBOSE
    cout<<"r=["<<endl<<r<<endl<<"];"<<endl;
#endif
    // Get weights of other new seeds
    MatrixXd Wother(k-1,W.cols());
    for(int j = 0;j<i;j++)
    {
      Wother.row(j) = WS.row(j);
    }
    for(int j = i+1;j<k;j++)
    {
      Wother.row(j-1) = WS.row(j);
    }
#ifdef EXTREME_VERBOSE
    cout<<"Wother=["<<endl<<Wother<<endl<<"];"<<endl;
#endif
    if(Wother.size() > 0)
    {
      // find closest new seed and use distance as potential radius
      double r_new =
        sqrt((Wother.rowwise()-sw).array().pow(2).rowwise().sum().minCoeff());
#ifdef EXTREME_VERBOSE
      cout<<"r_new=["<<endl<<r_new<<endl<<"];"<<endl;
#endif
      // radius is minimum of pushed closest new and closest original
      if(r<r_new*push)
      {
        //continue;
        verbose("r_original (%d) < (%d)*push\n",r,r_new);
      }
      r = (r<r_new*push?r:r_new*push);
    }
#ifdef EXTREME_VERBOSE
    cout<<"r=["<<endl<<r<<endl<<"];"<<endl;
#endif
    assert(r>0);
    for(int j = 0;j<n;j++)
    {
      // get distance to this seed
      EW(j,added_i) = sqrt((W.row(j)-sw).array().pow(2).sum());
      // clamp to r
      EW(j,added_i) = EW(j,added_i) > r ? r : EW(j,added_i);
      // divide by r, reverse and run through ease filter
      EW(j,added_i) = ::ease(ease,1.0-EW(j,added_i)/r);
    }
    added_i++;
  }
  EW.conservativeResize(EW.rows(),added_i);
}

#endif
