#ifndef ROTATIONCONVERSIONS_H_
#define ROTATIONCONVERSIONS_H_

#include "EulerAngle.h"
#include "Matrix.h"
#include "Quaternion.h"

namespace mathtool {
  //////////////////////////////////////////////////////////////////////////////
  // Conversions to EulerAngles
  //////////////////////////////////////////////////////////////////////////////

  //assignment from a 3x3 rotation matrix. Convert matrix to EulerZYX.
  inline EulerAngle& convertFromMatrix(EulerAngle& _e, const Matrix3x3& _m) {
    _e.m_beta = std::atan2(-_m[2][0], std::sqrt(_m[2][1]*_m[2][1] + _m[2][2]*_m[2][2]));
    if(std::cos(_e.m_beta) > 0) {
      _e.m_alpha = std::atan2(_m[1][0], _m[0][0]);
      _e.m_gamma = std::atan2(_m[2][1], _m[2][2]);
    } else {
      _e.m_alpha = std::atan2(-_m[1][0], -_m[0][0]);
      _e.m_gamma = std::atan2(-_m[2][1], -_m[2][2]);
    }
    return _e;
  }
  //assignment from a quaternion. convert to matrix first then to Euler.
  Matrix3x3& convertFromQuaternion(Matrix3x3&, const Quaternion& _q);
  inline EulerAngle& convertFromQuaternion(EulerAngle& _e, const Quaternion& _q) {
    Matrix3x3 m;
    convertFromQuaternion(m, _q);
    return convertFromMatrix(_e, m);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Conversions to Matrix3x3
  //////////////////////////////////////////////////////////////////////////////

  //assignment from EulerZYX
  inline Matrix3x3& convertFromEuler(Matrix3x3& _m, const EulerAngle& _e) {
    double sa = std::sin(_e.m_alpha);
    double ca = std::cos(_e.m_alpha);
    double sb = std::sin(_e.m_beta);
    double cb = std::cos(_e.m_beta);
    double sg = std::sin(_e.m_gamma);
    double cg = std::cos(_e.m_gamma);
    _m[0][0] = ca*cb;
    _m[0][1] = ca*sb*sg - sa*cg;
    _m[0][2] = ca*sb*cg + sa*sg;
    _m[1][0] = sa*cb;
    _m[1][1] = sa*sb*sg + ca*cg;
    _m[1][2] = sa*sb*cg - ca*sg;
    _m[2][0] = -sb;
    _m[2][1] = cb*sg;
    _m[2][2] = cb*cg;
    return _m;
  }
  //assignment from Quaternion
  inline Matrix3x3& convertFromQuaternion(Matrix3x3& _m, const Quaternion& _q) {
    double  w = _q.m_s;
    double  x = _q.m_v[0];
    double  y = _q.m_v[1];
    double  z = _q.m_v[2];

    _m[0][0] = 1.0 - 2.0*(y*y + z*z);
    _m[0][1] = 2.0*(x*y - w*z);
    _m[0][2] = 2.0*(x*z + w*y);

    _m[1][0] = 2.0*(x*y + w*z);
    _m[1][1] = 1.0 - 2.0*(x*x + z*z);
    _m[1][2] = 2.0*(y*z - w*x);

    _m[2][0] = 2.0*(x*z - w*y);
    _m[2][1] = 2.0*(y*z + w*x);
    _m[2][2] = 1.0 - 2.0*(x*x + y*y);
    return _m;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Conversions to Quaternions
  //////////////////////////////////////////////////////////////////////////////

  //assignment from Euler. First convert to matrix then to quaternion.
  inline Quaternion& convertFromEuler(Quaternion& _q, const EulerAngle& _e) {
    Matrix3x3 m;
    convertFromEuler(m, _e);
    return convertFromMatrix(_q, m);
  }
  //assignment from 3x3 rotation matrix
  inline Quaternion& convertFromMatrix(Quaternion& _q, const Matrix3x3& _m) {
    double t = trace(_m); //trace of the matrix
    double s, x, y, z, w;

    if(t > 0) {
      s = 0.5 / std::sqrt(1 + t);
      w = 0.25 / s;
      x = (_m[2][1] - _m[1][2]) * s;
      y = (_m[0][2] - _m[2][0]) * s;
      z = (_m[1][0] - _m[0][1]) * s;
    }
    else{ //if(t <= 0)
      //column 0
      if(_m[0][0] > _m[1][1] && _m[0][0] > _m[2][2]) {
        s = std::sqrt(1.0 + _m[0][0] - _m[1][1] - _m[2][2]) * 2;
        w = (_m[2][1] - _m[1][2]) / s;
        x = 0.25 * s;
        y = (_m[0][1] + _m[1][0]) / s;
        z = (_m[0][2] + _m[2][0]) / s;
      }
      //column 1
      else if (_m[1][1] > _m[2][2]) {
        s = std::sqrt(1.0 + _m[1][1] - _m[0][0] - _m[2][2]) * 2;
        w = (_m[0][2] - _m[2][0]) / s;
        x = (_m[0][1] + _m[1][0]) / s;
        y = 0.25 * s;
        z = (_m[1][2] + _m[2][1]) / s;
      }
      //column 2
      else {
        s = std::sqrt(1.0 + _m[2][2] - _m[0][0] - _m[1][1]) * 2;
        w = (_m[1][0] - _m[0][1]) / s;
        x = (_m[0][2] + _m[2][0]) / s;
        y = (_m[1][2] + _m[2][1]) / s;
        z = 0.25 * s;
      }
    }

    //The quaternion is then defined as: Q = | W X Y Z |
    _q.m_s = w;
    _q.m_v(x, y, z);
    return _q;
  }

}

#endif
