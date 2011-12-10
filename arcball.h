#ifndef ARCBALL_H
#define ARCBALL_H

#include "cvec.h"
#include "matrix4.h"
#include <iostream>
using namespace std;

inline bool getScreenSpaceCircle(const Cvec3& center, double radius,            // camera/eye coordinate info for sphere
                                 const Matrix4& projection,                     // projection matrix
                                 const double frustNear, const double frustFovY,
                                 const int screenWidth, const int screenHeight, // viewport size
                                 Cvec2& outCenter, double& outRadius) {         // output data in screen coordinates
  // returns false if the arcball is behind the viewer
  // get post projection canonical coordinates
  Cvec3 postproj = Cvec3(projection * Cvec4(center, 1));
  double w = projection(3, 0) * center[0] + projection(3,1)
             * center[1] + projection(3, 2) * center[2] + projection(3, 3) * 1.0;
  double winv = 0.0;
  if (w != 0.0)
    winv = 1.0 / w;
  postproj *= winv;

  // convert to screen pixel space
  outCenter[0] = postproj[0] * (double)screenWidth/2.0 + ((double)screenWidth-1.0)/2.0;
  outCenter[1] = postproj[1] * (double)screenHeight/2.0 + ((double)screenHeight-1.0)/2.0;

  // determine some overall radius
  double dist = center[2];
  if (dist < frustNear) {
    outRadius = -radius/(dist * tan(frustFovY * CS175_PI/360.0));
  }
  else {
    outRadius = 1.0;
    return false;
  }

  outRadius *= screenHeight * 0.5;
  return true;
}

#endif

