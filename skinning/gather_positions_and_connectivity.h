#include "Bone.h"
#include <vector>
#include <Eigen/Dense>

// Gather the positions of each bone and the connectivity as matlab style
// matrices (V,E), or like a graph
//
// Inputs:
//   BR  list of bone tree roots
// Outputs:
//   V  vertices over which edges are defined, # vertices by dim
//   P  list of roots with weights
//   BE  edge list, # edges by 2
//   WI  #P+#BE  list of weight indices
inline void gather_positions_and_connectivity(
  const std::vector<Bone*> & BR, 
  Eigen::MatrixXd & V,
  Eigen::VectorXi & P,
  Eigen::MatrixXi & BE,
  Eigen::VectorXi & WI);


// Implementation
#include <map>

inline void gather_positions_and_connectivity(
  const std::vector<Bone*> & BR, 
  Eigen::MatrixXd & V,
  Eigen::VectorXi & P,
  Eigen::MatrixXi & BE,
  Eigen::VectorXi & WI)
{
  using namespace std;
  vector<Bone *> B = gather_bones(BR);
  // Build map from bone pointers to index in B
  map<const Bone *,int> B2I;
  // Map NULL to -1
  B2I[NULL] = -1;
  int i = 0;
  for(vector<Bone*>::iterator bit = B.begin();bit != B.end();bit++)
  {
    B2I[*bit] = i;
    i++;
  }

  // count weighted roots
  int weighted_roots = 0;
  for(vector<Bone*>::iterator bit = B.begin();bit != B.end();bit++)
  {
    if((*bit)->is_root() && (*bit)->get_wi()>=0)
    {
      weighted_roots++;
    }
  }

  // Resize list of vertices, one for each "Bone" including roots
  // Resize list of edges, one for each bone segment, so excluding roots
  V.resize(B.size(),3);
  BE.resize(B.size() - BR.size(),2);
  // Only consider point handles at weighted roots
  P.resize(weighted_roots);
  WI.resize(weighted_roots+BE.rows());
  int e = 0;
  int p = 0;
  // loop over bones
  for(vector<Bone*>::iterator bit = B.begin();bit != B.end();bit++)
  {
    // Store position
    V.row(B2I[*bit]) = (*bit)->rest_tip();
    // If not root, then store connectivity
    if(!(*bit)->is_root())
    {
      // Bone edge
      BE(e,0) = B2I[(*bit)->get_parent()];
      BE(e,1) = B2I[*bit];
      // Bone edges are expected to have weight indices
      assert((*bit)->get_wi()>=0);
      WI(P.size()+e) = (*bit)->get_wi();
      e++;
    }else if((*bit)->get_wi()>=0)
    {
      // Point handle
      P(p) = B2I[*bit];
      WI(p) = (*bit)->get_wi();
      p++;
    }
  }

}
