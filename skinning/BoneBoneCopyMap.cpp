#include "BoneBoneCopyMap.h"
#include "gather_bones.h"
#include "Bone.h"
#include <iostream>

BoneBoneCopyMap::BoneBoneCopyMap(std::vector<Bone*> BR, bool rest)
{
  std::vector<Bone*> B = gather_bones(BR);
  for(std::vector<Bone*>::iterator bit = B.begin(); bit != B.end(); bit++)
  {
    //this->[*bit] = Bone(*bit);
    this->insert(std::make_pair(*bit,Bone(*bit)));
    if(rest)
    {
      assert(this->find(*bit) != this->end());
      this->find(*bit)->second.reset();
    }
  }
  std::cout<<"BoneBoneCopyMap constructed..."<<std::endl;
}

