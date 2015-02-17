#ifndef LOAD_TGF_AND_DMAT_PAIR_AS_BONE_ROOTS_AND_WEIGHTS_H
#define LOAD_TGF_AND_DMAT_PAIR_AS_BONE_ROOTS_AND_WEIGHTS_H
#include <Eigen/Core>
#include "Skeleton.h"

// Load a skeleton from a .tgf and weights from a .dmat file (as output by
// lbs.app) and convert them to a bone roots and weights representation
//
// Inputs:
//   tgf_filename  path to .tgf file containing #P point handles and #BE bone
//     edge handles
//   dmat_filename  path to .dmat file, containing #V by #C+#BE weights
//   skel Skeleton object container 
// Outputs:
//    BR  list of bone roots
//    W  #V by #P+#BE weights matrix
bool load_tgf_and_dmat_pair_as_bone_roots_and_weights(
  const std::string tgf_filename,
  const std::string dmat_filename,
  Skeleton<Bone> * skel,
  std::vector<Bone* > & BR,
  Eigen::MatrixXd & W);

// Implementation
#include <igl/readTGF.h>
#include <igl/readDMAT.h>

bool load_tgf_and_dmat_pair_as_bone_roots_and_weights(
  const std::string tgf_filename,
  const std::string dmat_filename,
  Skeleton<Bone> * skel,
  std::vector<Bone* > & BR,
  Eigen::MatrixXd & W)
{
  using namespace igl;
  using namespace Eigen;
  using namespace std;
  vector<vector<double> > C;
  vector<vector<int> > E,BE,CE,PE;
  vector<int> P;
  // read TGF as C,P,BE
  bool tgf_success = readTGF(tgf_filename,C,E,P,BE,CE,PE);
  if(!tgf_success)
  {
    return false;
  }
  // Sort BE
  cout<<"BE: "<<endl;
  for(int i = 0; i<(int)BE.size();i++)
  {
    for(int j = 0; j<(int)BE[i].size();j++)
    {
      cout<<BE[i][j]<<" ";
    }
    cout<<endl;
  }
  cout<<endl;
  sort(BE.begin(),BE.end());
  cout<<"sort(BE): "<<endl;
  for(int i = 0; i<(int)BE.size();i++)
  {
    for(int j = 0; j<(int)BE[i].size();j++)
    {
      cout<<BE[i][j]<<" ";
    }
    cout<<endl;
  }
  cout<<endl;
  // read WCBE from dmat_filename
  MatrixXd WCBE;
  bool dmat_success = readDMAT(dmat_filename,WCBE);
  if(!dmat_success)
  {
    return false;
  }
  // build W from WCBE and C,P,BE sizes
  W.resize(WCBE.rows(),P.size() + BE.size());
  // loop over point handles
  for(int i = 0;i<(int)P.size();i++)
  {
    if(P[i] >= WCBE.cols() || P[i] < 0)
    {
      fprintf(stderr,
        "Error: load_tgf_and_dmat_pair_as_bone_roots_and_weights: "
        " point handle index in .tgf file %d > number of weights "
        " in .dmat file %d\n",
        P[i],(int)WCBE.cols());
      W.resize(0,0);
      return false;
    }
    W.col(i) = WCBE.col(P[i]);
  }
  // Loop over sorted bone edges
  for(int i = 0;i<(int)BE.size();i++)
  {
    int wcbei = C.size()+i;
    if(wcbei >= WCBE.cols() || wcbei < 0)
    {
      fprintf(stderr,
        "Error: load_tgf_and_dmat_pair_as_bone_roots_and_weights: "
        " point handle index in .tgf file %d > number of weights "
        " in .dmat file %d\n",
        wcbei,(int)WCBE.cols());
      W.resize(0,0);
      return false;
    }
    W.col(P.size() + i) = WCBE.col(wcbei);
  }

  // Normalize each row
  for(int i = 0;i<(int)W.rows();i++)
  {
    double sum = W.row(i).sum();
    W.row(i) /= sum;
  }

  // construct BR from P,sorted BE, and C
  // Clear bone roots list
  for(vector<Bone*>::iterator bit = BR.begin();
      bit != BR.end(); bit++)
  {
    delete *bit;
  }
  BR.clear();
  // map from indices to bone pointers
  map<int,Bone*> index2bone;

  // loop over point handles
  for(int i = 0;i<(int)P.size();i++)
  {
    Vec3 offset;
    offset(0) = C[P[i]][0];
    offset(1) = C[P[i]][1];
    offset(2) = C[P[i]][2];
    Bone * b = *BR.insert(BR.end(),new Bone(skel,NULL,offset));
    b->set_wi(i);
  }

  // Loop over sorted bone edges
  for(int i = 0;i<(int)BE.size();i++)
  {
    int pi = BE[i][0];
    Vec3 pi_offset;
    pi_offset(0) = C[BE[i][0]][0];
    pi_offset(1) = C[BE[i][0]][1];
    pi_offset(2) = C[BE[i][0]][2];
    int ci = BE[i][1];
    Vec3 ci_offset;
    ci_offset(0) = C[BE[i][1]][0];
    ci_offset(1) = C[BE[i][1]][1];
    ci_offset(2) = C[BE[i][1]][2];
    // offset is really from parent
    ci_offset -= pi_offset;
    map<int,Bone*>::iterator pit = index2bone.find(pi);
    Bone * parent;
    // check if new parent
    if(pit == index2bone.end())
    {
      // new root
      parent = *BR.insert(BR.end(),new Bone(skel,NULL,pi_offset));
      // parents don't get weights
      parent->set_wi(-1);
      // add to index2bone map
      index2bone[pi] = parent;
    }else
    { 
      parent = pit->second;
    }
    map<int,Bone*>::iterator cit = index2bone.find(ci);
    // destination should always be new
    if(cit!=index2bone.end())
    {
      fprintf(stderr,
        "Error: load_tgf_and_dmat_pair_as_bone_roots_and_weights: "
        "  bones must form a forest\n");
      return false;
    }
    Bone * c = new Bone(skel,parent,ci_offset);
    // add to index2bone map
    index2bone[ci] = c;
    c->set_wi(P.size() + i);
  }

  return true;
}
#endif
