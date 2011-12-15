#ifndef RAY_H
#define RAY_H

#include "cvec.h"

struct Ray {
  Cvec3 point;
  Cvec3 direction;

  Ray(const Cvec3& point_, const Cvec3& direction_)
    : point(point_), direction(direction_) {}
};

#endif