#ifndef NO_PUPPET

#include "PuppetInterface.h"
#include <boost/thread/thread.hpp>
#include "Skeleton.h"
#include "splitter_rotation.h"
#include "puppet_angles_to_quat.h"

#include <iostream>

// Static
void PuppetInterface::handler(void * arg, const std::string &s)
{
  using namespace std;
  PuppetInterface * pi = (PuppetInterface*)arg;
  pi->parse_line(s);
}

PuppetInterface::PuppetInterface(const std::string &devname, Skeleton * _skel):
  // Set up default values and pass on devname to initialize serial
  skel(_skel),
  verbose(true),
  looping(false),
  scale(1.0),
  serial(devname.c_str(),handler,this),
  topology_changed_since_last_sync(false),
  print_angles(false)
{
}

PuppetInterface::~PuppetInterface()
{
  bool stopped = stop_looping();
  close();
}

bool PuppetInterface::open(unsigned int speed)
{
  // Depending on the local set up serial.Open(...) will fail because
  // permissions on the device are not set correctly. To test issue something
  // like:
  //   stty -echo -F /dev/ttyUSB0
  // If you get:
  //   stty: /dev/ttyUSB0: Permission denied
  // Then try:
  //   sudo stty -echo -F /dev/ttyUSB0
  // If this produces no error then you simply need to add rw privileges to the
  // device file:
  //   sudo chmod o+rw /dev/ttyUSB0
  // Now you should see no error issuing again:
  //   stty -echo -F /dev/ttyUSB0
  // 
  // https://groups.google.com/forum/?fromgroups#!topic/fhem-users/2nUjXv16vlc
  return serial.Open(speed);
}

bool PuppetInterface::close()
{
  bool stopped = stop_looping();
  assert(!looping);
  return serial.Close();
}

void PuppetInterface::reset()
{
  using namespace std;
  boost::mutex::scoped_lock l(m_mutex);
  serial.Send("b\n");
  serial.EmptyBuffers();
  serial.reset();
  serial.Send("i\n");
  if(verbose)
  {
    cerr<<__FUNCTION__<<": Reset sent"<<endl;
  }
}

void PuppetInterface::fork_and_loop()
{
  using namespace std;
  stop_looping();
  assert(!loop_thread.joinable());
  assert(!looping);
  loop_thread = boost::thread(mem_fun(&PuppetInterface::loop),this);
  if(verbose)
  {
    cerr<<__FUNCTION__<<": forked"<<endl;
  }
}

void PuppetInterface::sync()
{
  using namespace std;
  // Lock for this scope
  boost::mutex::scoped_lock l(m_mutex);

  //cout<<"n2b.size(): "<<n2b.size()<<endl;
  //cout<<"NodeDB.size(): "<<topology.size()<<endl;

  // If topology has changed then need to update
  if(topology_changed_since_last_sync)
  {
    // re-read entire skeleton
    // (but ideally remember offsets of bones that have remained)
    on_topology_change();
    topology_changed_since_last_sync = false;
  }
  // update offsets (and kin)
  update_offsets();
}

void PuppetInterface::loop()
{
  using namespace std;
  if(verbose)
  {
    cerr<<__FUNCTION__<<": entering main loop"<<endl;
  }
  reset();

  looping = true;
  while(looping)
  {
    {
      boost::mutex::scoped_lock l(m_mutex);
      serial.update();
      // Now serial.line is updated
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(1));
  }
  serial.Close();
  cout<<"loop() ended"<<endl;
}

void PuppetInterface::parse_line(const std::string & s)
{
  using namespace std;
  if (!pp.parseLine(s))
  {
    cerr<<__FUNCTION__<<": failed to parse line '"<<endl<<s<<endl<<"'"<<endl;
    return;
  }

  // Remember that topology has changed since last sync
  if(pp.topologyHasChanged())
  {
    topology_changed_since_last_sync = true;
    // Save new topology
    topology = pp.getTopology();
  }
  if(pp.measurementsAreReady())
  {
    // Save new measurements
    for(
      Puppet::MeasurementMap::const_iterator mit = pp.getMeasurements().begin();
      mit != pp.getMeasurements().end();
      mit++)
    {
      measurements[mit->first] = mit->second;
    }
  }
}

bool PuppetInterface::stop_looping()
{
  using namespace std;
  if(!loop_thread.joinable())
  {
    return false;
  }
  cerr<<__FUNCTION__<<": loop_thread is already running"<<endl;
  cerr<<"  Waiting for it to finish"<<endl;
  looping = false;
  loop_thread.join();
  return true;
}

void PuppetInterface::on_topology_change()
{
  using namespace Puppet;
  using namespace std;
  cout<<"on_topology_change()"<<endl;
  // assumes that pp.topologyHasChanged() returned true since last sync

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
    cerr<<"^PuppetInterface::on_topology_change No Nodes (or no master)"<<endl;
    return;
  }

  // traverse puppet's tree until we find where skeleton doesn't match
  // append new subtrees according to puppet
  append_new_subtrees(it->first,0,Quat(1,0,0,0));
}

void PuppetInterface::append_new_subtrees(
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
        Vec3 offset = Tform3(frame)*Vec3(scale,0,0);
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

void PuppetInterface::update_offsets()
{
  using namespace std;
  using namespace Puppet;
  assert(skel->get_editing());
  
  const NodeDB nodedb = pp.getTopology();
  
  if(nodedb.size() == 0)
  {
    cerr<< "^PuppetInterface::update_offsets No Nodes (or no master)"<<endl;
    return;
  }

  // Loop over nodes
  for(NodeDB::const_iterator nit = nodedb.begin();nit!=nodedb.end();nit++)
  {
    //printf("%u:\n",nit->first);
    const NodeRecord & n = nit->second;
    if((n.type & NODE_SPLITTER_MASK) == NODE_SPLITTER_GENERIC)
    {
      //printf("  SPLITTER skipped\n");
      continue;
    }
    MeasurementMap::const_iterator im = measurements.find(nit->first);
    if (im == measurements.end())
    {
      //printf(" -no data- skipped\n");
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

    if(print_angles)
    {
      cout<<"angles_rad: "<<endl<<
        angles_rad[0]<<" "<<
        angles_rad[1]<<" "<<
        angles_rad[2]<<" "<<endl;
    }

    // Raw rotation
    Quat q = puppet_angles_to_quat(nit->second.type,angles_rad);
    Bone * b = n2bit->second.b;
    // Remember this q
    n2bit->second.last_q = q;
    // Set bones rotation to raw rotation conjugated with frame
    if(skel->get_editing())
    {
      b->rotation = q;
    }else
    {
      //cout<<"frame: "<<
      //  n2bit->second.frame.x()<<" "<<
      //  n2bit->second.frame.y()<<" "<<
      //  n2bit->second.frame.z()<<" "<<
      //  n2bit->second.frame.w()<<" "<<endl;
      b->rotation = 
        n2bit->second.pframe*
        q*
        n2bit->second.rotation_at_set.inverse()*
        n2bit->second.pframe.inverse();
      //b->rotation = Quat(1,0,0,0);
    }
    Vec3 tail = b->offset;
    if(!b->is_root())
    {
      tail = b->get_parent()->rest_tip();
    }
    b->translation = b->rotation.inverse() * tail - tail;
  }
}

void PuppetInterface::set_frames()
{
  skel->print();
  using namespace std;
  cout<<"set_frames()"<<endl;
  // Loop over connected bones
  for(
    NodeID2Bone::iterator n2bit = n2b.begin();
    n2bit != n2b.end();
    n2bit++
    )
  {
    BoneData & bd = n2bit->second;
    bd.offset_at_set = bd.b->offset;
    bd.frame = bd.b->rotated_frame();
    bd.rotation_at_set = bd.b->rotation;
    bd.pframe = Quat(1,0,0,0);
    if(!bd.b->is_root())
    {
      bd.pframe = bd.b->get_parent()->rotated_frame();
    }
    // Compute and store bone edge vector. Root gets its current position
    Vec3 tail = Vec3(0,0,0);
    if(!bd.b->is_root())
    {
      tail = bd.b->tail_as_drawn();
    }
    bd.bone_vector_at_set = bd.b->tip_as_drawn() - tail;
  }

  // Loop over connected bones
  for(
    NodeID2Bone::iterator n2bit = n2b.begin();
    n2bit != n2b.end();
    n2bit++
    )
  {
    BoneData & bd = n2bit->second;
    bd.b->rotation = Quat(1,0,0,0);
    bd.b->translation = Vec3(0,0,0);
    // Restor bone vector as offset
    bd.b->offset = bd.bone_vector_at_set; 
  }

  //// Loop over connected bones
  //for(
  //  NodeID2Bone::iterator n2bit = n2b.begin();
  //  n2bit != n2b.end();
  //  n2bit++
  //  )
  //{
  //  cout<<(int)n2bit->first<<": before"<<endl;
  //  skel->print();
  //  BoneData & bd = n2bit->second;
  //  bd.frame = bd.last_q;
  //  // remember offset
  //  bd.offset_at_set = bd.b->offset;
  //  bd.translation_at_set = bd.b->translation;
  //  // Unrotate each bone so that offsets end up where they are with identity
  //  // rotations
  //  Tform3 A(Tform3::Identity());
  //  Vec3 tail = bd.b->offset;
  //  if(!bd.b->is_root())
  //  {
  //    tail = bd.b->tail_as_drawn();
  //  }
  //  A.translate(-tail);
  //  A.rotate(bd.b->rotation);
  //  A.translate(tail);
  //  A.translate(bd.b->translation);
  //  if(!bd.b->is_root())
  //  {
  //    // rotate my offset, if I'm not a root
  //    bd.b->offset = A*bd.b->offset;
  //  }
  //  bd.b->translation = Vec3(0,0,0);
  //  bd.b->apply_affine_to_kin(A);
  //  cout<<"after:"<<endl;
  //  skel->print();
  //  cout<<endl;
  //}
}

void PuppetInterface::unset_frames()
{
  using namespace std;
  cout<<"unset_frames()"<<endl;
  // Loop over connected bones
  for(
    NodeID2Bone::iterator n2bit = n2b.begin();
    n2bit != n2b.end();
    n2bit++
    )
  {
    BoneData & bd = n2bit->second;
    bd.frame = Quat(1,0,0,0);
    // remember offset
    bd.b->offset = bd.offset_at_set;
  }
}

#endif
