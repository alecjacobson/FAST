#ifndef NO_PUPPET

#include "LogPuppet.h"
#include "Skeleton.h"
#include "destroy_bone_roots.h"
#include "puppet_angles_to_quat.h"
#include "BoneData.h"
#include "splitter_rotation.h"

#define VERBOSE
#include <igl/verbose.h>
#include <igl/PI.h>
#include <cassert>
#include <iostream>

LogPuppet::LogPuppet(Skeleton * skel_):
  // Private:
  skel(skel_)
{
  assert(skel != NULL);
}

LogPuppet::~LogPuppet()
{
  // Try to clean up
  if(fp != NULL)
  {
    close();
  }
}

bool LogPuppet::open(const std::string & filename)
{
  using namespace std;
  using namespace igl;
  verbose("LogPuppet::open()\n");
  fp = fopen(filename.c_str(),"r");
  if(fp == NULL)
  {
    cerr<<"^LogPuppet::open: Error: Can't open: '"<<filename<<"'"<<endl;
    return false;
  }
  return true;
}

bool LogPuppet::close()
{
  using namespace std;
  using namespace igl;
  verbose("LogPuppet::close()\n");
  int success = fclose(fp);
  if(success == EOF)
  {
    cerr<<"^LogPuppet::close: Error: Can't close file"<<endl;
    return false;
  }
  return true;
}

bool LogPuppet::has_next()
{
  using namespace std;
  using namespace igl;
  if(fp == NULL)
  {
    cerr<<"^LogPuppet::has_next: Error: file is NULL"<<endl;
    return false;
  }
  return !feof(fp);
}

#define BUFFER_LENGTH 1024
char buffer[BUFFER_LENGTH];
bool LogPuppet::process_next()
{
  assert(skel != NULL);
  using namespace std;
  using namespace igl;
  char * r = fgets(buffer,BUFFER_LENGTH,fp);
  if(!r)
  {
    cerr<<"^LogPuppet::process_next: Error: fgets() result is NULL"<<endl;
    return false;
  }

  //cout<<buffer<<endl;

  // Send line to puppetparser
  std::string line(buffer);
  bool success = pp.parseLine(line);
  if(!success)
  {
    cerr<<"^LogPuppet::process_next: Error: could not parse '"<<line<<"'"<<endl;
    return false;
  }

  if(!pp.measurementsAreReady())
  {
    // just continue when reading next line
    return true;
  }

  // if topology has changed then switch to edit mode
  if(pp.topologyHasChanged())
  {
    pp.printTopology(true,stdout);
    on_topology_change();
    // re-read entire skeleton
    // (but ideally remember offsets of bones that have remained)
  }else
  {
    pp.printTopology(true,stdout);
    if(skel->get_editing())
    {
      // update offsets (and kin)
      update_offsets();
    }else
    {
      // update rotations
    }
  }

  return true;
}

void LogPuppet::append_new_subtrees(
  const uint8_t nid, 
  const uint8_t pid, 
  const Quat & frame)
{
  using namespace std;
  using namespace Puppet;
  printf("^append_new_subtrees: %u %u\n",nid,pid);
  cout<<"  frame: "
    <<frame.w()<<" "
    <<frame.x()<<" "
    <<frame.y()<<" "
    <<frame.z()<<" "
    <<endl;
  // Perhaps you have called this on a leaf's child (you shouldn't have)
  assert(nid != 0);

  NodeDB::const_iterator nit = pp.getTopology().find(nid);
  assert(nit != pp.getTopology().end());
  const NodeRecord & n = nit->second;

  bool splitter = false;
  int effective_parent_id = nid;
  if((n.type & NODE_SPLITTER_MASK) == NODE_SPLITTER_GENERIC)
  {
    printf("  SPLITTER\n");
    splitter = true;
    // Don't add a bone for splitters
    // Children of splitter should think of splitter's parent as parent
    effective_parent_id = pid;
  }else
  {
    // Only add bones for non splitters
    NodeID2Bone::iterator n2bit = n2b.find(nid);
    // Try to find match to n
    if(n2bit  == n2b.end())
    {
      printf("  Not found in n2b\n");
      Bone * nb;
      // not found in n2b (no associated bone to this id)
      if(0 == pid)
      {
        // Found new root
        printf("    New root\n");
        nb = *skel->roots.insert( 
          skel->roots.end(),new
          Bone(skel,NULL,DEFAULT_ROOT_OFFSET));
      }else
      {
        // Parent should have associated bone
        NodeID2Bone::iterator pit = n2b.find(pid);
        // Parent should have associated bone
        assert(pit != n2b.end());
        Bone * pb = pit->second.b;
        // Skeleton container should be the same
        assert(pb->skel == skel);
        printf("    New child\n");
        Vec3 offset = Tform3(frame)*Vec3(1,0,0);
        cout<<"    offset: "<<offset<<endl;
        nb = new Bone(skel,pb,offset);
      }
      // Add to n2b map
      n2b.insert(pair<uint8_t,BoneData>(nid,BoneData(nb)));
    }else
    {
      // found in n2b 
      printf("  already in n2b\n");
      // check that parent also makes sense if it is also in the puppet
      if(0 != pid)
      {
        NodeID2Bone::iterator p2bit = n2b.find(pid);
        // Parent should have associated bone
        assert(p2bit != n2b.end());
        // bone associated to parent node should be same as parent bone of this
        // node
        assert(p2bit->second.b == n2bit->second.b->get_parent());
      }
    }
  }

  // Recurse on children
  for (int i=0;i<(int)n.offspring.size();i++)
  {
    uint8_t cid = n.offspring[i];
    // Only recurse on real children
    if(cid != 0)
    {
      Quat split_rot = splitter_rotation(n,i);
      Quat frame_i = split_rot*frame;
      append_new_subtrees(cid,effective_parent_id,frame_i);
    }
  }
}

void LogPuppet::on_topology_change()
{
  using namespace Puppet;
  using namespace igl;
  using namespace std;
  // assumes that pp.topologyHasChanged() just returned true

  // Topology changes are only allowed in edit mode
  skel->set_editing(true);
  
  const NodeDB nodedb = pp.getTopology();

  // Remove any bones (and their subtrees) that were linked to nodes that are
  // no longer alive
  // http://stackoverflow.com/a/180772/148668
  for(NodeID2Bone::iterator n2bit=n2b.begin();n2bit!=n2b.end();)
  {
    cout<<"id: "<<n2bit->first<<endl;
    // Try to find node with this id in nodedb
    if(nodedb.find(n2bit->first) == nodedb.end())
    {
      // Delete this bone and its descendents
      delete n2bit->second.b;
      // Delete this from n2b map
      n2b.erase(n2bit++);
    }else
    {
      ++n2bit;
    }
  }

  // At this point all the bones in b2n should point to valid noderecords

  // In other words, any existing bones are either not linked to nodes or are
  // linked to valid nodes
  
  NodeDB::const_iterator it = nodedb.find(1); // Find master
  if(it == nodedb.end())
  {
    verbose("^LogPuppet::on_topology_change No Nodes (or no master)\n");
    return;
  }

  // traverse puppet's tree until we find where skeleton doesn't match
  // append new subtrees according to puppet
  append_new_subtrees(it->first,0,Quat(1,0,0,0));
}

void LogPuppet::update_offsets()
{
  using namespace std;
  using namespace Puppet;
  assert(skel->get_editing());
  
  const NodeDB nodedb = pp.getTopology();
  
  if(nodedb.size() == 0)
  {
    cerr<< ("^LogPuppet::update_offsets No Nodes (or no master)\n");
    return;
  }

  // Loop over nodes
  for(NodeDB::const_iterator nit = nodedb.begin();nit!=nodedb.end();nit++)
  {
    printf("%u:\n",nit->first);
    const NodeRecord & n = nit->second;
    if((n.type & NODE_SPLITTER_MASK) == NODE_SPLITTER_GENERIC)
    {
      printf("  SPLITTER skipped\n");
      continue;
    }
    MeasurementMap::const_iterator im = pp.getMeasurements().find(nit->first);
    if (im == pp.getMeasurements().end())
    {
      printf(" -no data- skipped\n");
      continue;
    }
    if (im->second.data.size()<=0)
    {
      printf(" -empty data- skipped\n");
      continue;
    }

    // TODO: SHOULD BE JUST PASSING THIS TO BUFFER


    NodeID2Bone::iterator n2bit = n2b.find(nit->first);
    // Should be taken as invariant that n2b is up to date.
    assert(n2bit != n2b.end());

    // TODO: ANGLE == 0 MEANS NO DATA, OR MORE PRECISELY WHEN THE INTEGER DATA
    // TERM == 0 THERE IS NO DATA
    int zeros = count(im->second.data.begin(),im->second.data.end(),0);
    if(zeros > 0)
    {
      printf("  %d 0%s in data. skipped\n",zeros,(zeros==1?"":"s"));
      continue;
    }
    vector<float> angles_rad = im->second.angles_rad;

    // Raw rotation
    Quat q = puppet_angles_to_quat(nit->second.type,angles_rad);
    Bone * b = n2bit->second.b;
    // Set bones rotation to raw rotation conjugated with frame
    b->rotation = n2bit->second.frame.inverse() * q * n2bit->second.frame;
    Vec3 tail = b->offset;
    if(!b->is_root())
    {
      tail = b->get_parent()->rest_tip();
    }
    b->translation = b->rotation.inverse() * tail - tail;

  }

}
#endif
