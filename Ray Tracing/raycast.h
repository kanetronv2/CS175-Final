#ifndef RAYCAST_H
#define RAYCAST_H

#include <vector>
#include "cvec.h"
#include "ray.h"
#include "scene.h"

struct Intersection {
  double lambda;      // lambda == -1 means "no intersection was found"
  int surfaceId;
};

Intersection rayCast(const Scene& scene, const Ray& ray);

Cvec3 shadeLight(const Scene& scene, const Light& light, const Surface& surface, const Cvec3& viewDirection, const Cvec3& point, const Cvec3& normal, const int level);

Cvec3 shade(const Scene& scene, const int surfaceId, const Ray& ray, const Intersection& in, const int level);

Cvec3 rayTrace(const Scene& scene, const Ray& ray, const int level = 0);


#endif
