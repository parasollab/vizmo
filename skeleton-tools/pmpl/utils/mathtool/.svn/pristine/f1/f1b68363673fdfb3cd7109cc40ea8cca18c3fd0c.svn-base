//Operations for Quaternion - addition, scalar multiplication, and
//quaternion multiplication

#ifndef QUATERNION_H
#define QUATERNION_H

#include "Basic.h"
#include "Vector.h"

namespace mathtool{

  class EulerAngle;
  template<size_t N, size_t M> class Matrix;
  typedef Matrix<3, 3> Matrix3x3;

  class Quaternion {

    public:
      Quaternion(double _s = 1, const Vector3d& _v = Vector3d()) : m_s(_s), m_v(_v) {}

      //access
      const double& real() const {return m_s;}
      const Vector3d& imaginary() const {return m_v;}

      //equality
      bool operator==(const Quaternion& _q) const {
        return m_s == _q.m_s && m_v == _q.m_v;
      }
      //inequality
      bool operator!=(const Quaternion& _q) const {
        return !(*this == _q);
      }

      //self addition
      Quaternion& operator+=(const Quaternion& _q) {
        m_s += _q.m_s; m_v += _q.m_v;
        return *this;
      }
      //self subtraction
      Quaternion& operator-=(const Quaternion& _q) {
        m_s -= _q.m_s; m_v -= _q.m_v;
        return *this;
      }
      //self scalar multiplication
      Quaternion& operator*=(double _d) {
        m_s *= _d; m_v *= _d;
        return *this;
      }
      //self scalar division
      Quaternion& operator/=(double _d) {
        m_s /= _d; m_v /= _d;
        return *this;
      }
      //self multiplication
      Quaternion& operator*=(const Quaternion& _q) {
        double s = m_s;
        m_s = s*_q.m_s - m_v*_q.m_v;
        m_v = s*_q.m_v + m_v*_q.m_s + m_v%_q.m_v;
        return *this;
      }
      Quaternion& operator*=(const Vector3d& _v) {
        return *this *= Quaternion(0, _v);
      }

      //inverse of quaternion
      Quaternion operator-() const {
        return Quaternion(m_s, -m_v);
      }
      //addition
      Quaternion operator+(const Quaternion& _q) const {
        return Quaternion(m_s + _q.m_s, m_v + _q.m_v);
      }
      //subtraction
      Quaternion operator-(const Quaternion& _q) const {
        return Quaternion(m_s - _q.m_s, m_v - _q.m_v);
      }
      //scalar multiplication
      Quaternion operator*(double _d) const {
        return Quaternion(m_s*_d, m_v*_d);
      }
      //scalar division
      Quaternion operator/(double _d) const {
        return Quaternion(m_s/_d, m_v/_d);
      }
      //multiplication
      Quaternion operator*(const Quaternion& _q) const {
        Quaternion q(*this);
        q *= _q;
        return q;
      }
      Quaternion operator*(const Vector3d & _v) const {
        return *this * Quaternion(0, _v);
      }

      //magnitude
      double norm() const {return sqrt(normsqr());}
      //magnitude squared
      double normsqr() const {return m_s*m_s + m_v.normsqr();}
      //Normalization
      Quaternion& normalize() {
        return *this /= norm();
      }
      Quaternion normalized() const {
        return *this / norm();
      }

      friend std::istream& operator>>(std::istream& _is, Quaternion& _q);
      friend std::ostream& operator<<(std::ostream& _os, const Quaternion& _q);
      friend Quaternion& convertFromMatrix(Quaternion& _q, const Matrix3x3& _m);
      friend Matrix3x3& convertFromQuaternion(Matrix3x3& _m, const Quaternion& _q);
      friend EulerAngle& convertFromQuaternion(EulerAngle& _m, const Quaternion& _q);

    private:

      double m_s; //real component of Quaternion
      Vector3d m_v; //vector component of Quaternion
  };

  //////////////////////////////////////////////////////////////////////////////
  // Utility for multiplication, input, and output
  //////////////////////////////////////////////////////////////////////////////
  inline Quaternion operator*(const Vector3d& _v, const Quaternion& _q) {
    return Quaternion(0, _v)*_q;
  }

  inline std::istream& operator>>(std::istream& _is, Quaternion& _q) {
    return _is >> _q.m_s >> _q.m_v;
  }

  inline std::ostream& operator<<(std::ostream& _os, const Quaternion& _q) {
    return _os << _q.m_s << " " << _q.m_v;
  }

}

#endif
