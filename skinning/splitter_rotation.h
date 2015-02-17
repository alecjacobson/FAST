#ifndef SPLITTER_ROTATION_H
#define SPLITTER_ROTATION_H
#ifndef NO_PUPPET

#include <PuppetParserRQ.h>
#include "Quat.h"

// Get Rotation based on type of parent and index of child
// Inputs:
//   p  parent node (splitter)
//   child_i  number of child (index in p.offspring)
// Returns rotation that transforms parent's frame into child i's
Quat splitter_rotation(
  const Puppet::NodeRecord & p,
  const int child_i);
#endif
#endif
