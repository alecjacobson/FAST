#ifndef DEGREEOFFREEDOMTYPE_H
#define DEGREEOFFREEDOMTYPE_H

enum DegreeOfFreedomType
{
  // readBF, writeBF already expect this order
  DOF_TYPE_FIXED_POSITION = 0,
  DOF_TYPE_FREE = 1,
  DOF_TYPE_FIXED_ALL = 2,
  DOF_TYPE_FIXED_LINEAR = 3,
  NUM_DOF_TYPES = 4
};

#endif
