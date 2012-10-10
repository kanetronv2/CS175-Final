#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "cvec.h"
#include "surface.h"
#include "scene.h"

using namespace std;
using namespace std::tr1;

void parseSceneFile(const char *filename, Camera& camera, Scene& scene) {
  ifstream is(filename, ios::binary);

  vector<shared_ptr<Surface> > surfaces;
  vector<Light> lights;

  while(!is.eof()) {
    string tag;
    is >> ws >> tag >> ws;
    if (tag == string("TRIANGLE")) {
      Cvec3 data[6];
      string subtag;
      int exponent;
      double mirror;
	  int marble;

      for (int i=0; i<6; i++) {
        is >> ws >> subtag >> ws >>
        data[i][0] >> ws >> data[i][1] >> ws >> data[i][2] >> ws;
      }
      is >> ws >> subtag >> ws >> exponent >> ws;
      is >> ws >> subtag >> ws >> mirror >> ws;
	  is >> ws >> subtag >> ws >> marble >> ws;
      surfaces.push_back(shared_ptr<Surface>(new Triangle(data[0], data[1], data[2], data[3], data[4], data[5], exponent, mirror, marble)));
    }
    else if (tag == string("PLANE")) {
      Cvec3 data[5];
      string subtag;
      int exponent;
      double mirror;
	  int marble;

      for (int i=0; i<5; i++) {
        is >> ws >> subtag >> ws >>
        data[i][0] >> ws >> data[i][1] >> ws >> data[i][2] >> ws;
      }
      is >> ws >> subtag >> ws >> exponent >> ws;
      is >> ws >> subtag >> ws >> mirror >> ws;
	  is >> ws >> subtag >> ws >> marble >> ws;
      surfaces.push_back(shared_ptr<Surface>(new Plane(data[0], data[1], data[2], data[3], data[4], exponent, mirror, marble)));
    }
    else if (tag == string("SPHERE")) {
      Cvec3 center;
      Cvec3 data[3];
      string subtag;
      float radius;
      int exponent;
      double mirror;
	  int marble;

      is >> ws >> subtag >> ws >>
      center[0] >> ws >> center[1] >> ws >> center[2] >> ws;
      is >> ws >> subtag >> ws >> radius >> ws;
      for (int i=0; i<3; i++) {
        is >> ws >> subtag >> ws >>
        data[i][0] >> ws >> data[i][1] >> ws >> data[i][2] >> ws;
      }
      is >> ws >> subtag >> ws >> exponent >> ws;
      is >> ws >> subtag >> ws >> mirror >> ws;
	  is >> ws >> subtag >> ws >> marble >> ws;
      surfaces.push_back(shared_ptr<Surface>(new Sphere(center, radius, data[0], data[1], data[2], exponent, mirror, marble)));
    }
    else if (tag == string("LIGHT")) {
      Cvec3 data[2];
      string subtag;

      for (int i=0; i<2; i++) {
        is >> ws >> subtag >> ws >>
        data[i][0] >> ws >> data[i][1] >> ws >> data[i][2] >> ws;
      }
      lights.push_back(Light(data[0], data[1]));
    }
    else if (tag == string("CAMERA")) {
      string subtag;
      is >> ws >> subtag >> ws >> camera.fovY >> ws;
      is >> ws >> subtag >> ws >> camera.width >> ws;
      is >> ws >> subtag >> ws >> camera.height >> ws;
      is >> ws >> subtag >> ws >> camera.samples >> ws;
	  is >> ws >> subtag >> ws >> camera.focallength >> ws;
	  is >> ws >> subtag >> ws >> camera.aperture >> ws;
	  is >> ws >> subtag >> ws >> camera.focaldepth >> ws;
	  is >> ws >> subtag >> ws >> camera.coc >> ws;
    }
    else {
      // do nothing
    }
  }

  scene.lights.swap(lights); // do a swap to avoid copying
  scene.surfaces.swap(surfaces); // do a swap to avoid copying
}
