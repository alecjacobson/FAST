#ifndef PROJECT_CONNECT_H
#define PROJECT_CONNECT_H

//#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
//#include <Eigen/Sparse>
#include <map>
#include <utility>

// PROJECT_CONNECT automatically connect control points using skinning weights
// defined at each control point, rather than trying to determine whether both
// weights significantly influence some point. Just check that all pairwise
// projects are positive
//
// [A] = project_connect(W)
//
// Templates:
//   Mat  should be a matrix class that supports Mat(i,j) subscript look up,
//   .rows(), .cols(), .row(i).array()
//   T  should be a eigen sparse matrix primitive type like int or double
// Inputs:
//  W  weights, #weights by #dimensions matrix of weight space positions of
//    seeds
// Outputs:
//  A  #weights by #weights sparse adjacency matrix
//
template <typename Mat, typename T>
inline void project_connect(
  const Mat & W, 
  std::map<std::pair<int,int>, T> & A);

template <typename Mat, typename T>
inline void project_connect(
  const Mat & W, 
  std::map<std::pair<int,int>, T> & A)
{
  // number of weights/seeds
  int m = W.rows();

  A.clear();
  // consider whether weight seeds i and j are neighbors
  for(int i = 0;i<m;i++)
  {
    for(int j = 0;j<m;j++)
    {
      if(i != j)
      {
        bool valid = true;
        // check that j is not "behind" line segment from k to i
        for(int k = 0;k<m;k++)
        {
          if(i != k && j != k)
          {
            // vector from k to i dot vector from k to j
            double dot = 
              (double)(
                ((W.row(i)-W.row(k)).array()*
                (W.row(j)-W.row(k)).array()).sum());
            if(dot < 0)
            {
              valid = false;
              break;
            }
          }
        }
        if(valid)
        {
          //A.coeffRef(i,j) = 1;
          A[std::pair<int,int>(i,j)] = 1;
        }
      }
    }
  }
}

#endif
