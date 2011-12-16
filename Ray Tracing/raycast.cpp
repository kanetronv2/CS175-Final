#include <cmath>
#include <cassert>

#include "surface.h"
#include "scene.h"
#include "raycast.h"
#include "perlin.h"

using namespace std;

inline Cvec3 getBounce(const Cvec3& v, const Cvec3& n) {
	Cvec3 normal = normalize(n);

	Cvec3 bounce = normal * dot(v, normal) * 2.0 - v;

	return bounce;
}

// This is the function that simply goes through all the geometry and
// finds the closest intersection in the positive side of the ray
Intersection rayCast(const Scene& scene, const Ray& ray) {
  Intersection in;
  in.lambda = -1;
  for (size_t i = 0; i < scene.surfaces.size(); ++i) {
    const double lambda = scene.surfaces[i]->intersect(ray);

    if (lambda > 0 && (in.lambda < 0 || lambda < in.lambda)) {
      in.lambda = lambda;
      in.surfaceId = i;
    }
  }
  return in;
}


Cvec3 shadeLight(const Scene& scene, const Light& light, const Surface& surface, const Cvec3& viewDirection, const Cvec3& point, const Cvec3& normal, const int level) {

	const Cvec3& lightDirection = (light.position-point).normalize();
	const Cvec3& lightIntensity = light.intensity;
	const Cvec3 bounceDirection = getBounce(lightDirection, normal);

	const Cvec3& kd = surface.getDiffuseCoef();
	const Cvec3& ks = surface.getSpecularCoef();
	const int n = surface.getExponentSpecular();

	const Cvec3 diffuse = mulElemWise(kd, lightIntensity) * max(0., dot(normal, lightDirection));
	const Cvec3 specular = mulElemWise(ks, lightIntensity) * pow(max(0., dot(bounceDirection, viewDirection)), n);
	
	Ray shadowRay(point + lightDirection * CS175_EPS,lightDirection);
	Intersection in = rayCast(scene, shadowRay);                // compute the intersection poin

	if (in.lambda > 0)
		return Cvec3(0,0,0);    

	Cvec3 hitPoint = point + lightDirection * CS175_EPS + lightDirection * in.lambda;
	Cvec3 output(0);
	float noiseCoef = 0.0f;
	if(surface.getMarbleToggle() != 0){
		for(int level = 1; level < 10; level++){
			noiseCoef += (1.0f/level) * fabsf(float(noise(level * .05 * hitPoint(0),
				                                          level * .05 * hitPoint(1),
														  level * .05 * hitPoint(2))));
		}
		output = (diffuse * noiseCoef) + (Cvec3(.7, .8, .9) * (1 - noiseCoef));
		return output;
	}

	return diffuse + specular;
}

Cvec3 shade(const Scene& scene, const int surfaceId, const Ray& ray, const Intersection& in, const int level) {
  const Surface& surface = *scene.surfaces[surfaceId];
  const Cvec3 point = ray.point + ray.direction * in.lambda;       // the intersection point
  const Cvec3 normal = surface.computeNormal(point);              // the normal at the intersection point
  const Cvec3 viewDirection = (ray.point - point).normalize();       // the view direction goes from the point to the beginning of the ray (normalized)
  const Cvec3& ka = surface.getAmbientCoef();
  
  Cvec3 mirrorColor = (0,0,0);

  assert(abs(dot(normal, normal) - 1) < CS175_EPS);    // we check that the normal is pre-normalized


  Cvec3 outputColor(0,0,0);
  for (size_t i = 0; i < scene.lights.size(); ++i) {
    outputColor += mulElemWise(ka, scene.lights[i].intensity);
    if (dot(scene.lights[i].position - point, normal) > 0) {
      outputColor += shadeLight(scene, scene.lights[i], surface, viewDirection, point, normal, level);
    }
  }

  if(surface.getMirrorCoef()!=0){
	Cvec3 bounceDir = getBounce(viewDirection, normal);
	Cvec3 newPoint =  point + bounceDir * CS175_EPS;
	mirrorColor = rayTrace(scene, Ray(newPoint, bounceDir), level+1);
  }

  return outputColor  * (1-surface.getMirrorCoef()) + mirrorColor * (surface.getMirrorCoef());
}


Cvec3 rayTrace(const Scene& scene, const Ray& ray, const int level) {

  const Intersection in = rayCast(scene, ray);                // compute the intersection point

  if (in.lambda < 0 || level == 5)
    return Cvec3(.7,.7,.7);                    // if no intersection => return black (background color)

  return shade(scene, in.surfaceId, ray, in, level);         // ..otherwise compute shade and return that color
}
