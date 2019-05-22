/* EulerAngle is expressed in EulerZYX or FixedXYZ rotation format
 */

#ifndef EULERANGLE_H_
#define EULERANGLE_H_

#include "Basic.h"

namespace mathtool {

  template<size_t N, size_t M> class Matrix;
  typedef Matrix<3, 3> Matrix3x3;
  class Quaternion;

  class EulerAngle {
    public:

      EulerAngle(double _alpha = 0.0, double _beta = 0.0, double _gamma = 0.0) {
        operator()(_alpha, _beta, _gamma);
      }

      //assignment
      EulerAngle& operator=(const EulerAngle& _e){
        return operator()(_e.m_alpha, _e.m_beta, _e.m_gamma);
      }
      //set the values of the EulerAngle
      EulerAngle& operator()(double _alpha = 0.0, double _beta = 0.0, double _gamma = 0.0) {
        m_alpha = _alpha; m_beta = _beta; m_gamma = _gamma;
        return *this;
      }

      //access
      double alpha() const {return m_alpha;}
      double beta() const {return m_beta;}
      double gamma() const {return m_gamma;}

      //equality
      bool operator==(const EulerAngle& _e) const {
        return m_alpha == _e.m_alpha && m_beta == _e.m_beta && m_gamma == _e.m_gamma;
      }
      //inequality
      bool operator!=(const EulerAngle& _e) const {
        return !(*this == _e);
      }

      //self addition
      EulerAngle& operator+=(const EulerAngle& _e) {
        m_alpha = fmod(_e.m_alpha + m_alpha, TWOPI);
        m_beta = fmod(_e.m_beta + m_beta, TWOPI);
        m_gamma = fmod(_e.m_gamma + m_gamma, TWOPI);
        return *this;
      }
      //self subtraction
      EulerAngle& operator-=(const EulerAngle& _e) {
        m_alpha = fmod(_e.m_alpha - m_alpha, TWOPI);
        m_beta = fmod(_e.m_beta - m_beta, TWOPI);
        m_gamma = fmod(_e.m_gamma - m_gamma, TWOPI);
        return *this;
      }

      //inversion
      EulerAngle operator-() const {
        return EulerAngle(fmod(-m_alpha + PI, TWOPI), m_beta, fmod(-m_gamma + PI, TWOPI));
      }
      //addition
      EulerAngle operator+(const EulerAngle& _e) const {
        EulerAngle e(*this);
        e += _e;
        return e;
      }
      //subtraction
      EulerAngle operator-(const EulerAngle& _e) const {
        EulerAngle e(*this);
        e -= _e;
        return e;
      }

      friend std::istream& operator>>(std::istream& _is, EulerAngle& _e);
      friend std::ostream& operator<<(std::ostream& _os, const EulerAngle& _e);

      friend EulerAngle& convertFromMatrix(EulerAngle& _e, const Matrix3x3& _m);
      friend Matrix3x3& convertFromEuler(Matrix3x3& _m, const EulerAngle& _e);

    private:
      double m_alpha, m_beta, m_gamma;
  };

  //input EulerAngle from degrees
  inline std::istream& operator>>(std::istream& _is, EulerAngle& _e) {
    double a, b, g;
    _is >> g >> b >> a;
    _e.m_alpha = fmod(degToRad(a), TWOPI);
    _e.m_beta = fmod(degToRad(b), TWOPI);
    _e.m_gamma = fmod(degToRad(g), TWOPI);
    return _is;
  }

  //output EulerAngle to degrees
  inline std::ostream& operator<<(std::ostream& _os, const EulerAngle& _e) {
    std::ios::fmtflags f(_os.flags());
    _os << std::fixed
      << std::setprecision(4) << radToDeg(fmod(_e.m_alpha, TWOPI)) << " "
      << std::setprecision(4) << radToDeg(fmod(_e.m_beta, TWOPI)) << " "
      << std::setprecision(4) << radToDeg(fmod(_e.m_gamma, TWOPI));
    _os.flags(f);
    return _os;
  }
}

#endif
