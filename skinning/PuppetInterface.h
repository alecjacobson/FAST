#ifndef NO_PUPPET
#ifndef PUPPETINTERFACE_H
#define PUPPETINTERFACE_H

// Puppet API
#include "PuppetParserRQ.h"
#include "SerialLineHandler.h"
#include <boost/thread.hpp>
class Skeleton;

#include "BoneData.h"

// Class for connecting a puppet to a skeleton
class PuppetInterface
{
  public:
    Skeleton * skel;
    bool verbose;
    bool looping;
    double scale;
    bool print_angles;
  private:
    // Mutex used around changes to measurements
    boost::mutex m_mutex;
    SerialLineHandler::SerialLineHandler serial;
    Puppet::PuppetParserRQ pp;
    typedef std::map<uint8_t,BoneData> NodeID2Bone;
    NodeID2Bone n2b;
    boost::thread loop_thread;
    // State of puppet
    bool topology_changed_since_last_sync;
    Puppet::NodeDB topology;
    // Last measurements
    Puppet::MeasurementMap measurements;

  // Static handle method
  static void handler(void * arg, const std::string &s);
  public:
    // Instanciate interface with a path to the puppet device location
    // Inputs:
    //   devname path to device location (e.g. /dev/tty.Puppet)
    PuppetInterface(const std::string &devname, Skeleton * _skel);
    // Destroy and cleanup
    ~PuppetInterface();
    // Open the serial port
    // Inputs:
    //   speed  serial device speed
    //
    // e.g. on the linux machine I use speed=115200
    bool open(unsigned int speed);
    // Closes the serial port
    bool close();
    // Resets the serial device
    void reset();
    // Spawn a worker thread and tell him to entire main loop
    void fork_and_loop();
    // Synchronize skeleton with last saved measurements and topology
    void sync();
    // Freezes frames of current joints (called after switching from edit to
    // deform mode)
    void set_frames();
    // Unfreezes frames and restores skeleton as it was before deforming
    void unset_frames();
  private:
    // Main loop, loops until looping is set to false. Should be
    // called by a worker thread. Always closes serial at finish
    void loop();
    // Parse a new line from the serial
    // Inputs:
    //   s line read from serial
    void parse_line(const std::string & s);
    // Set looping to false and wait for loop thread to finish
    // Returns true only if thread had been running and was succesfully stopped
    bool stop_looping();
    // Called when syncing after topology change 
    void on_topology_change();
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
    // Update offsets of bones based on current measurements
    void update_offsets();
};

#endif

