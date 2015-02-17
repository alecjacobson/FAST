// Write a bones forest to a file 
// Input:
//   file_name  path to .bf bones tree file
//   BR  list of bone roots
// Returns true on success, false on errors
inline bool write_BF(const char * file_name,std::vector<Bone*> & BR);

// Implementation
#include "gather_bones.h"
#include <cstdio>

inline bool write_BF(const char * file_name,std::vector<Bone*> & BR)
{

  FILE * fp = fopen(file_name,"w");
  if(NULL==fp)
  {
    printf("IOError: write_BF could not open %s",file_name);
    return false;  
  }
  std::vector<Bone *> B = gather_bones(BR);
  // Build map from bone pointers to index in B
  std::map<const Bone *,int> B2I;
  // Map NULL to -1
  B2I[NULL] = -1;
  int i = 0;
  for(std::vector<Bone*>::iterator bit = B.begin();bit != B.end();bit++)
  {
    B2I[*bit] = i;
    i++;
  }
  // Print line for each bone
  for(std::vector<Bone*>::iterator bit = B.begin();bit != B.end();bit++)
  {
    Bone * b = (*bit);
    Tform3 T = b->last_T;
    int dof_type = 0;
    switch(b->tip_dof_type)
    {
      case DOF_TYPE_FIXED_POSITION:
        dof_type = 0;
        break;
      case DOF_TYPE_FREE:
        dof_type = 1;
        break;
      case DOF_TYPE_FIXED_ALL:
        dof_type = 2;
        break;
      case DOF_TYPE_FIXED_LINEAR:
        dof_type = 3;
        break;
      default:
        assert(false);
    }
    fprintf(fp,
      "%d %d "
      "%lg %lg %lg "
      "%lg %lg %lg %lg "
      "%lg %lg %lg "
      "%lg %lg "
      "%lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg "
      "%d "
      "\n",
      b->get_wi(),B2I[b->get_parent()],
      b->offset[0],b->offset[1],b->offset[2],
      b->rotation.w(), b->rotation.x(), b->rotation.y(), b->rotation.z(),
      b->translation.x(), b->translation.y(), b->translation.z(),
      b->stretch, b->twist,
      T.affine()(0,0), T.affine()(0,1), T.affine()(0,2), T.affine()(0,3),
      T.affine()(1,0), T.affine()(1,1), T.affine()(1,2), T.affine()(1,3),
      T.affine()(2,0), T.affine()(2,1), T.affine()(2,2), T.affine()(2,3),
      dof_type
      );
  }
  fclose(fp);
  return true;
}
