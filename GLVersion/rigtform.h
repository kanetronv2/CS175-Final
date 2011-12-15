#ifndef RIGTFORM_H
#define RIGTFORM_H

#include <iostream>
#include <cassert>

#include "matrix4.h"
#include "quat.h"

class RigTForm {
  Cvec3 t_; // translation component
  Quat r_;  // rotation component represented as a quaternion

public:
  RigTForm() : t_(0) {
    assert(norm2(Quat(1,0,0,0) - r_) < CS175_EPS2);
  }

  RigTForm(const Cvec3& t, const Quat& r) {
    t_ = t;
	r_ = r;
  }

  RigTForm(const Cvec3& t) {
    t_ = t;
	r_ = Quat();
  }

  RigTForm(const Quat& r) {
    t_ = Cvec3(0,0,0);
	r_ = r;
  }

  Cvec3 getTranslation() const {
    return t_;
  }

  Quat getRotation() const {
    return r_;
  }

  RigTForm& setTranslation(const Cvec3& t) {
    t_ = t;
    return *this;
  }

  RigTForm& setRotation(const Quat& r) {
    r_ = r;
    return *this;
  }

  Cvec4 operator * (const Cvec4& a) const {
	return ((r_ * a) + Cvec4(t_,0) * a[3]);
  }

  RigTForm operator * (const RigTForm&	a) const {
	return RigTForm(Cvec3(Cvec4(t_) + r_ * Cvec4(a.getTranslation(),0)), r_ * a.getRotation());
  }
};

inline RigTForm inv(const RigTForm& tform) {
  return RigTForm(Cvec3(inv(tform.getRotation()) * Cvec4(tform.getTranslation(),0) * -1), inv(tform.getRotation()));
}

inline RigTForm transFact(const RigTForm& tform) {
  return RigTForm(tform.getTranslation());
}

inline RigTForm linFact(const RigTForm& tform) {
  return RigTForm(tform.getRotation());
}

inline Matrix4 rigTFormToMatrix(const RigTForm& tform) {
  Matrix4 T=Matrix4(Matrix4::makeTranslation(tform.getTranslation()));
  Matrix4 R=quatToMatrix(tform.getRotation());
  return T * R;
}

#endif;