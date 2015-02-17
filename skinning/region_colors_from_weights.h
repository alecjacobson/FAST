#ifndef REGION_COLORS_FROM_WEIGHTS_H
#define REGION_COLORS_FROM_WEIGHTS_H
// Returns a coloring for each corner of a mesh as a colored with region_color
// if any weights are exactly (0,...,1,...,0) and with interior color otherwise
// Inputs:
//   OW  #V by #W list of original weights
//   F  #F by 3 list of triangle indices
// Outputs
//   C  #F*3 by 3 list of RGB colors, C(i*3+j,:) gives color of F(i,j)
inline void region_colors_from_weights(
  const Eigen::MatrixXd & OW,
  const Eigen::MatrixXi & F,
  const float region_color[],
  const float interior_color[],
  Eigen::MatrixXd & C);

// Implementation

inline void region_colors_from_weights(
  const Eigen::MatrixXd & OW,
  const Eigen::MatrixXi & F,
  const float region_color[],
  const float interior_color[],
  Eigen::MatrixXd & C)
{
  using namespace Eigen;

  // resize output
  C.resize(F.rows()*3,3);

  for(int i = 0;i < F.rows(); i++)
  {
    bool all_region = true;
    for(int j = 0;j < F.cols(); j++)
    {
      bool found_one = false;
      bool all_zero = true;
      for(int k = 0;k < OW.cols(); k++)
      {
        if(!found_one && OW(F(i,j),k)==1)
        {
          found_one = true;
        }else
        {
          all_zero &= OW(F(i,j),k) == 0;
        }
      }
      all_region &= found_one && all_zero;
    }
    for(int j = 0;j < F.cols(); j++)
    {
      for(int c = 0;c<3;c++)
      {
        C(i*3+j,c) = (all_region?region_color[c]:interior_color[c]);
      }
    }
  }
}
#endif

