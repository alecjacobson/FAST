#ifndef LOGPUPPET_H
#define LOGPUPPET_H

#include <PuppetParserRQ.h>
#include "Tform.h"
#include "Quat.h"
#include "Vec.h"

#include <string>
#include <stdlib.h>
#include <map>
#include <cassert>
#include "BoneData.h"

class Skeleton;
class Bone;

class LogPuppet
{
  private:
    // File pointer to current file
    FILE * fp;
    // Skeleton object this LogPuppet is attached to (aka writing into)
    Skeleton * skel;
    // Puppet parser class
    Puppet::PuppetParserRQ pp;
    typedef std::map<uint8_t,BoneData> NodeID2Bone;
    NodeID2Bone n2b;
  
  public:
    LogPuppet(Skeleton * skel_);
    ~LogPuppet();
    ////////////////////////////////////////////////////////////////////////
    // Input
    ////////////////////////////////////////////////////////////////////////
    // Start a new session
    // Returns true on success, false on error
    bool open(const std::string & filename);
    // Close an existing session
    // Returns true on success, false on error
    bool close();
    // Returns true if more lines exist, false on erro
    bool has_next();
    // Get, parse and process line
    // Returns true on success, false on error
    bool process_next();
  private:
    ////////////////////////////////////////////////////////////////////////
    // Processing
    ////////////////////////////////////////////////////////////////////////
    // Append any new subtrees to the b2n map
    // Inputs:
    //   n  id of this node
    //   p  id of parent (0 --> master/root)
    ////   offset  to place new bone n (if created) with respect to p)
    //   T  frame offset transformation
    void append_new_subtrees(
      const uint8_t n,
      const uint8_t p,
      const Quat & frame);
    // When topology of the puppet has changed we need to update the skeleton
    // accordingly
    void on_topology_change();
    // Update offsets of bones based on current measurements
    void update_offsets();
};
#endif
