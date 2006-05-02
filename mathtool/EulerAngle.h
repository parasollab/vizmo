// EularAngle.h: interface for the CL_CulerAngle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_EULERANGLE_H_)
#define _EULERANGLE_H_

#include "Basic.h"
#include <iostream>
#include <math.h>

using namespace std;

namespace mathtool {

    class EulerAngle  
    {
    public:
        ///////////////////////////////////////////////////////////////////////////////
        // Constructors & Destructor
        EulerAngle(){ m_alpha=0.0; m_beta=0.0; m_gamma=0.0; }
        
        EulerAngle(double alpha, double beta, double gamma){
            SetCoordinate( alpha, beta, gamma);
        }
        
        ///////////////////////////////////////////////////////////////////////////////
        // Access
        void SetCoordinate(double alpha, double beta, double gamma){
            m_alpha=fmod(alpha ,2*PI);
            m_beta =fmod(beta  ,2*PI);
            m_gamma=fmod(gamma ,2*PI);
        }
        
        void SetCoordinate(const EulerAngle & other){
            SetCoordinate( other.m_alpha, other.m_beta, other.m_gamma);
        }
        
        EulerAngle & operator=( const EulerAngle & other ){
            SetCoordinate(other.m_alpha, other.m_beta, other.m_gamma );
            return *this;
        }
        
        void operator+=(const EulerAngle & other){
            double alpha =fmod(other.m_alpha +m_alpha,2*PI);
            double beta  =fmod(other.m_beta  +m_beta, 2*PI);
            double gamma =fmod(other.m_gamma +m_gamma,2*PI);
            
            SetCoordinate(alpha, beta, gamma);
        }
        
        double Alpha() const {  return m_alpha; }
        double Beta() const  {  return m_beta;  }
        double Gamma() const {  return m_gamma; }
        
    ///////////////////////////////////////////////////////////////////////////////
    // Access
    private:
        double m_alpha, m_beta, m_gamma;
    };

    ostream & operator<<(ostream & out, const EulerAngle & angle); 
}//end of nprmlib namespace .

#endif // !defined(_EULERANGLE_H_)

