#include "Bone.h"
#include <vector>

// Gather displacements from bone forest roots
// Inputs:
//   BR  list of bone tree roots
//   D  #handles by #dim list of displacements
// Output:
//   D  #handles by #dim list of displacements
// Returns true on success, false on error
inline bool gather_displacements(
  const std::vector<Bone*> & BR, 
  Eigen::MatrixXd & D);

// Implementation
#include "Tform.h"
#include <list>
#include "gather_bones.h"

inline bool gather_displacements(
  const std::vector<Bone*> & BR, 
  Eigen::MatrixXd & D)
{
  int num_handles = D.rows();
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
          "Error: gather_displacements() max weight index of bones (%d)"
          " >= number of handles (%d)\n",
          wi,
          num_handles);
        return false;
      }
      if(wi >= 0)
      { 
        D.row(b->get_wi()) = b->tip(false,false) - b->rest_tip();
      }
    }
  }

  return true;
}
