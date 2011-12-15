#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>
#if __GNUG__
#   include <tr1/memory>
#endif

#include "cvec.h"
#include "surface.h"

struct Light {
  Cvec3 position;
  Cvec3 intensity;

  Light() {}
  Light(const Cvec3& position_, const Cvec3& intensity_)
    : position(position_), intensity(intensity_) {}
};

struct Camera {
  double fovY;
  int width;
  int height;
  int samples;
};

struct Scene {
  std::vector<Light> lights;
  std::vector<std::tr1::shared_ptr<Surface> > surfaces;
};

void parseSceneFile(const char *filename, Camera& camera, Scene& scene);



#endif

