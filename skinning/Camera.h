// This file is part of libigl, a simple c++ geometry processing library.
// 
// Copyright (C) 2013 Alec Jacobson <alecjacobson@gmail.com>
// 
// This Source Code Form is subject to the terms of the Mozilla Public License 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef CAMERA_H
#define CAMERA_H

// Simple (Legacy) Camera class
class Camera
{
  // Public Fields
  public:
    // Rotation as quaternion (0,0,0,1) is identity
    double rotation[4];
    // Translation of origin
    double pan[3];
    // Zoom scale
    double zoom;
    // Perspective angle
    double angle;
  // Public functions
  public:
    Camera();
    // Copy constructor
    // 
    // Inputs:
    //   that  other Camera to be copied
    Camera(const Camera & that);
    ~Camera(){}
};
#endif
