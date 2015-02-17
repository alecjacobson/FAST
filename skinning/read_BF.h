#ifndef READ_BF_H
#define READ_BF_H

#include "Skeleton.h"
// Read a bones forest from a file, returns a list of bone roots
// Input:
//   file_name  path to .bf bones tree file
//   skel  skeleton container
// Output:
//   BR  list of bone roots
// Returns true on success, false on errors
inline bool read_BF(const char * file_name,Skeleton<Bone> * skel, std::vector<Bone*> & BR);

// Implementation

#include <cstdio>
#include "gather_bones.h"
#include "destroy_bone_roots.h"

inline bool read_BF(const char * file_name,Skeleton<Bone> * skel,std::vector<Bone*> & BR)
{

  FILE * fp = fopen(file_name,"r");
  if(NULL==fp)
  {
    printf("IOError: read_BF could not open %s",file_name);
    return false;  
  }
  destroy_bone_roots(BR);

  std::vector<Bone*> bones;
  std::vector<int> parent_indices;

  const int MAX_LINE_LENGTH = 500;
  char line[MAX_LINE_LENGTH];
  int lineno = 1;
  int max_wi = -1;
  // read lines until end of file
  while(fgets(line,MAX_LINE_LENGTH,fp)!=NULL)
  {
    int wi;
    int parent_index;
    Vec3 offset;
    Quat rotation(1,0,0,0);
    Vec3 translation(0,0,0);
    double stretch = 1.0;
    double twist = 0.0;
    Tform3 T(Tform3::Identity());
    // 0 - DOF_TYPE_FIXED_POSITION, 
    // 1 - DOF_TYPE_FREE, 
    // 2 - DOF_TYPE_FIXED_ALL,
    // 3 - DOF_TYPE_FIXED_LINEAR
    int dof_type = -1; 
    // number of variables read
    int numvar;

    numvar = sscanf(line,
      "%d %d "
      "%lg %lg %lg "
      "%lg %lg %lg %lg "
      "%lg %lg %lg "
      "%lg %lg "
      "%lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg "
      "%d",
      &wi,&parent_index,
      &offset(0),&offset(1),&offset(2),
      &rotation.w(),&rotation.x(),&rotation.y(),&rotation.z(),
      &translation(0),&translation(1),&translation(2),
      &stretch,&twist,
      &T.affine()(0,0), &T.affine()(0,1), &T.affine()(0,2), &T.affine()(0,3),
      &T.affine()(1,0), &T.affine()(1,1), &T.affine()(1,2), &T.affine()(1,3),
      &T.affine()(2,0), &T.affine()(2,1), &T.affine()(2,2), &T.affine()(2,3),
      &dof_type
      );
    if(numvar < 26)
    {
      // Legacy
      int numvar = sscanf(line,"%d %d %lg %lg %lg",
        &wi,
        &parent_index,
        &offset(0),&offset(1),&offset(2));
      if(5 != numvar)
      {
        fprintf(stderr,"ERROR: read_BF() bad format on line %d",lineno);
        return false;
      }
    }

    Bone * b;
    if(parent_index == -1)
    {
      // ROOT
      b = *BR.insert(BR.end(),new Bone(skel,NULL,offset));
    }else if(parent_index < 0)
    {
      fprintf(stderr,
        "ERROR: read_BF() bad format on line %d,"
        " parent index (%d) should be -1 or >=0",
        lineno,
        parent_index);
      fclose(fp);
      return false;
    }else
    {
      b = new Bone(skel,NULL,offset);
    }
    //// wi is either no-weights -1, or >0
    //if(wi != -1 && wi<0)
    //{
    //  fprintf(stderr,
    //    "ERROR: read_BF() bad format on line %d,"
    //    " wieght index (%d) should be -1 or >=0",
    //    lineno,
    //    wi);
    //   fclose(fp);
    //   return false;
    //}
    max_wi = std::max(wi,max_wi);
    if(wi >= -1)
    {
      b->set_wi(wi);
    }
    b->rotation = rotation;
    b->translation = translation;
    b->stretch = stretch;
    b->twist = twist;
    b->last_T = T;
    if(numvar >= 27)
    {
      switch(dof_type)
      {
        case 0:
          b->tip_dof_type = DOF_TYPE_FIXED_POSITION;
          break;
        case 1:
          b->tip_dof_type = DOF_TYPE_FREE;
          break;
        case 2:
          b->tip_dof_type = DOF_TYPE_FIXED_ALL;
          break;
        case 3:
          b->tip_dof_type = DOF_TYPE_FIXED_LINEAR;
          break;
        default:
          fprintf(stderr,"Error: read_BF.h: unsupported dof_type %d\n",
            dof_type);
          fclose(fp);
          return false;
      }
    }

    bones.push_back(b);
    parent_indices.push_back(parent_index);
    lineno++;
  }
  printf("bones.size(): %d\n",(int)bones.size());
  fclose(fp);

  std::vector<bool> weight_index_taken;
  if(max_wi > 0)
  {
    weight_index_taken.resize(max_wi+1,false);
  }
  // loop over bones and parent indices
  for(int i = 0;i<(int)bones.size();i++)
  {
    if(parent_indices[i] != -1)
    {
      if(parent_indices[i] >= (int)bones.size())
      {
        fprintf(stderr,
          "ERROR: read_BF() bad format on line %d,"
          " parent index (%d) should be -1 or <#bones (%d)",
          i,
          parent_indices[i],
          (int)bones.size());
        return false;
      }
      if(bones[i]->get_wi() > 0 && weight_index_taken[bones[i]->get_wi()])
      {
        fprintf(stderr,
          "ERROR: read_BF() bad format on line %d,"
          " weight index (%d) already taken",
          i,
          bones[i]->get_wi());
        return false;
      }
      bones[i]->set_parent(bones[parent_indices[i]]);
    }
  }
  return true;
}
#endif
