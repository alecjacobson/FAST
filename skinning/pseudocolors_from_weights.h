#ifndef PSEUDOCOLORS_FROM_WEIGHTS_H
#define PSEUDOCOLORS_FROM_WEIGHTS_H
// Returns a coloring for each vertex of a mesh as a weighted combination of
// pseudocolors assigned to each weigh function
// Inputs:
//   OW  #V by #W list of original weights
// Outputs
//   C  #V by 3 list of RGB colors
inline void pseudocolors_from_weights(
  const Eigen::MatrixXd & OW,
  Eigen::MatrixXd & C);

// Implementation

inline void pseudocolors_from_weights(
  const Eigen::MatrixXd & OW,
  Eigen::MatrixXd & C)
{
  Eigen::MatrixXd colors(12,3);
  colors <<
    1.0,0.0,0.0,
    0.0,1.0,0.0,
    0.0,0.0,1.0,
    0.0,1.0,1.0,
    1.0,0.0,1.0,
    1.0,1.0,0.0,
    1.0,1.0,1.0,
    0.1,0.1,0.1,
    0.7,0.7,0.7,
    1.0,0.5,0.5,
    0.5,1.0,0.5,
    0.5,0.5,1.0;

  // resize output
  C.resize(OW.rows(),3);

  for(int j = 0;j < OW.rows(); j++)
  {
    C.row(j) = Eigen::MatrixXd::Zero(1,3);
    for(int i = 0;i < OW.cols(); i++)
    {
      C.row(j) += colors.row(i%colors.rows()) * OW(j,i);
    }
  }
}
#endif
