#include "Bone.h"
#include <vector>

// Gather rest positions from bone forest roots
// Inputs:
//   BR  list of bone tree roots
//   C  #handles by #dim list of rest positions 
// Output:
//   C  #handles by #dim list of rest positions 
// Returns true on success, false on error
inline bool gather_rest_positions(
  const std::vector<Bone*> & BR, 
  Eigen::MatrixXd & C);

// Implementation
#include "Tform.h"
#include <list>
#include "gather_bones.h"

inline bool gather_rest_positions(
  const std::vector<Bone*> & BR, 
  Eigen::MatrixXd & C)
{
  int num_handles = C.rows();

  std::vector<Bone*> B = gather_bones(BR);
  // loop over bones
  for(std::vector<Bone*>::iterator bit = B.begin();bit != B.end();bit++)
  {
    Bone * b = (*bit);
    if(b->wi_is_set())
    {
      int wi = b->get_wi();
      if(wi >= num_handles)
      {
        fprintf(
          stderr,
          "Error: gather_rest_positions() max weight index of bones (%d)"
          " >= number of handles (%d)\n",
          wi,
          num_handles);
        return false;
      }
      if(wi >= 0)
      { 
        C.row(b->get_wi()) = b->offset;
      }
    }
  }

  return true;
}
