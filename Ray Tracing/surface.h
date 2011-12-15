#ifndef SURFACE_H
#define SURFACE_H

#include "cvec.h"
#include "ray.h"
#include <iostream>
using namespace std;

class Surface {
  Cvec3 ka_;
  Cvec3 kd_;
  Cvec3 ks_;
  int exponentSpecular_;
  double mirrorCoef_;

public:
  Surface()
    : ka_(0.1,0.1,0.1), kd_(0.8,0.8,0.8), ks_(0,0,0), exponentSpecular_(32), mirrorCoef_(0)
  {}

  Surface(const Cvec3& ka, const Cvec3& kd, const Cvec3& ks, const int exponentSpecular, const double mirrorCoef)
    : ka_(ka), kd_(kd), ks_(ks), exponentSpecular_(exponentSpecular), mirrorCoef_(mirrorCoef)
  {}

  virtual ~Surface() {}

  virtual double intersect(const Ray& ray) const = 0;

  // returns  a *normalized* normal vector
  virtual Cvec3 computeNormal(const Cvec3& p) const = 0;

  const Cvec3& getAmbientCoef() const {
    return ka_;
  }
  const Cvec3& getDiffuseCoef() const {
    return kd_;
  }
  const Cvec3& getSpecularCoef() const {
    return ks_;
  }
  int getExponentSpecular() const {
    return exponentSpecular_;
  }
  double getMirrorCoef() const {
    return mirrorCoef_;
  }
};



class Sphere : public Surface {
  Cvec3 center_;
  double radius_;

public:
  Sphere(const Cvec3& center, const double radius,
         const Cvec3& ka, const Cvec3& kd, const Cvec3& ks, const int exponentSpecular, const double mirrorCoef)
    : Surface(ka, kd, ks, exponentSpecular, mirrorCoef), center_(center), radius_(radius)
  {}

  Sphere(const Cvec3& center, const double radius)
    : Surface(), center_(center), radius_(radius)
  {}

  double intersect(const Ray& ray) const {
	  double a = pow(ray.direction(0),2) + pow(ray.direction(1),2) + pow(ray.direction(2),2);
	  double b = 2*ray.direction(0)*(ray.point(0)-center_(0)) + 2*ray.direction(1)*(ray.point(1)-center_(1)) + 2*ray.direction(2)*(ray.point(2)-center_(2));
	  double c = pow(ray.point(0)-center_(0),2) + pow(ray.point(1) - center_(1),2) + pow(ray.point(2)-center_(2),2) - pow(radius_,2);

	  double det = pow(b,2) - 4 * a * c;

	  double lambda1;
	  double lambda2;

	  if(det < 0)
		  return -1;
	  else if (det==0){
		  lambda1 = -1 * b/(2 * a);
		  if(lambda1 < 0)
			  return -1;
		  else
			return lambda1;
	  }
	  else{
		  lambda1 = (-1*b + sqrt(det))/(2*a);
		  lambda2 = (-1*b - sqrt(det))/(2*a);
		  if(lambda1<0){
			  if(lambda2<0)
				  return -1;
			  else
				  return lambda2;
		  }
		  if(lambda2<0){
			  if(lambda1<0)
				  return -1;
			  else
				  return lambda1;
		  }
		  if(lambda1>lambda2)
			return lambda2;
		  else
			return lambda1;
	  }
  }

  Cvec3 computeNormal(const Cvec3& p) const {
	  return normalize(((p - center_) * 2));
  }
};


class Triangle : public Surface {
  Cvec3 point[3];

public:
  Triangle(const Cvec3& p0, const Cvec3& p1, const Cvec3& p2,
           const Cvec3& ka, const Cvec3& kd, const Cvec3& ks, const int exponentSpecular, const double mirrorCoef)
    : Surface(ka, kd, ks, exponentSpecular, mirrorCoef) {
    point[0] = p0;
    point[1] = p1;
    point[2] = p2;
  }

  Triangle(const Cvec3& p0, const Cvec3& p1, const Cvec3& p2)
    : Surface() {
    point[0] = p0;
    point[1] = p1;
    point[2] = p2;
  }

  double intersect(const Ray& ray) const {
	Cvec3 vector1 = point[1]-point[0];
	Cvec3 vector2 = point[2]-point[0];

	Cvec3 normal;
	normal = normalize(cross(vector1, vector2));

	double d = -1 * (normal(0) * point[0](0) + normal(1) * point[0](1) + normal(2) * point[0](2));
	double denominator = normal(0) * ray.direction(0) + normal(1) * ray.direction(1) + normal(2) * ray.direction(2);
	double lambda;  

	if(denominator==0)
		return -1;
	else{
		lambda = (-normal(0) * ray.point(0) - normal(1) * ray.point(1) - normal(2) *ray.point(2) - d)/denominator;
		if(lambda<0)
			return -1;
	}

	Cvec3 insidePoint = ray.point + ray.direction * lambda;

	Cvec3 d1 = cross(point[0]-insidePoint, point[1]-insidePoint);
	Cvec3 d2 = cross(point[1]-insidePoint, point[2]-insidePoint);
	Cvec3 d3 = cross(point[2]-insidePoint, point[0]-insidePoint);

	if(dot(d1,d2)<0 || dot(d1,d3)<0 || dot(d2,d3)<0)
		return -1;

	return lambda;
  }

  Cvec3 computeNormal(const Cvec3& p) const{
	Cvec3 vector1 = point[1]-point[0];
	Cvec3 vector2 = point[2]-point[0];

	Cvec3 normal;
	normal = normalize(cross(vector1, vector2));
	return normal;
  }
};


class Plane : public Surface {
  Cvec3 point;
  Cvec3 normal_;

public:
  Plane(const Cvec3& point, const Cvec3& normal,
        const Cvec3& ka, const Cvec3& kd, const Cvec3& ks, const int exponentSpecular, const double mirrorCoef)
    : Surface(ka, kd, ks, exponentSpecular, mirrorCoef), point(point), normal_(normal)
  {}

  Plane(const Cvec3& point, const Cvec3& normal)
    : point(point), normal_(normal)
  {}

  double intersect(const Ray& ray) const {
	  double d = -1.0 * (normal_(0) * point(0) + normal_(1) * point(1) + normal_(2) * point(2));
	  double denominator = normal_(0) * ray.direction(0) + normal_(1) * ray.direction(1) + normal_(2) * ray.direction(2);
	  
	  if(denominator == 0)
		  return -1;
	  else{
		double lambda = (-normal_(0) * ray.point(0) - normal_(1) * ray.point(1) - normal_(2) *ray.point(2) - d)/denominator;
		if(lambda<0)
			return -1;
		else
			return lambda;
	  }
  }

  Cvec3 computeNormal(const Cvec3& p) const {
    return normalize(normal_);
  }
};



#endif
