/********************************************************************

    Quaternion.h    Header File

    Jyh-Ming Lien 03/30/2002
    Computer Science.
    Texas A&M University

*********************************************************************/

#if !defined( _QUATERNION_H_NPRM_ )
#define _QUATERNION_H_NPRM_

#include "Matrix.h"
#include "Vector.h"

namespace mathtool{

    class Quaternion {
    public:
        Quaternion(){ m_s=1; }
        Quaternion( double s, Vector3d v ){ m_v=v; m_s=s; }
        Quaternion( const Quaternion & q){ *this=q; }

        ////////////////////////////////////////////////////////////////////////
        // Operations for Quaternion
        Quaternion operator*(const Quaternion & q) const {
            //double k=q.m_s*m_s;
            q.m_v*m_v;
            double s=q.m_s*m_s-q.m_v*m_v;
            Vector3d v=m_s*q.m_v+q.m_s*m_v+m_v%q.m_v;
            return Quaternion(s,v);
        }
        Quaternion operator*(const Vector3d & v) const { return *this*Quaternion(0,v); }
        Quaternion operator/(double s) const { return Quaternion(m_s/s,m_v/s); }
        Quaternion & operator=(const Quaternion & q){ set(q.m_s,q.m_v); return *this; }
        Quaternion operator+(const Quaternion & q) const { return Quaternion(m_s+q.m_s,m_v+q.m_v); }
        Quaternion operator-(const Quaternion & q) const { return Quaternion(m_s-q.m_s,m_v-q.m_v); }
        Quaternion operator-() const { return Quaternion(m_s,-m_v); }
        friend Quaternion operator*(const Vector3d & v, const Quaternion & q);
        friend istream& operator>>(istream & in, Quaternion & q );
        friend ostream& operator<<(ostream & out, Quaternion & q );

        //////////////////////////////////////////////////////////////////////////
        //Normalization
        Quaternion normalize(){ 
            Quaternion q(*this);
            double l=q.norm();
            q=q/l;
            return q;
        }

        double norm(){ return sqrt(normsqr()); }
        double normsqr(){ return m_v.normsqr()+sqr(m_s); }

        //////////////////////////////////////////////////////////////////////////
        //Access
        Matrix3x3 getMatrix(){
            double x_2=2*sqr(m_v[0]); double y_2=2*sqr(m_v[1]); double z_2=2*sqr(m_v[2]);
            double xy=2*m_v[0]*m_v[1]; double yz=2*m_v[1]*m_v[2]; double zx=2*m_v[2]*m_v[0]; 
            double sx=2*m_s*m_v[0]; double sy=2*m_s*m_v[1]; double sz=2*m_s*m_v[2]; 
            return Matrix3x3(1-y_2-z_2, xy-sz, zx+sy,
                             xy+sz, 1-x_2-z_2, yz-sx,
                             zx-sy, yz+sx, 1-x_2-y_2);
        }

	Quaternion getQuaternionFromMatrix(Matrix3x3 mat){

	  double X, Y, Z, W, S;
	  double T = 1 + mat[0][0] + mat[1][1] + mat[2][2];

	  if ( T > 0.00000001 ){
	    S = 0.5 / sqrt(T);
	    X = ( mat[2][1] - mat[1][2] ) * S;
	    Y = ( mat[0][2] - mat[2][0] ) * S;
	    Z = ( mat[1][0] - mat[0][1] ) * S;
	    W = 0.25 / S;
	  }
	  else{ //if(T <= 0)
	    if ( (mat[0][0] > mat[1][1]) && (mat[0][0] > mat[2][2]) )  {	// Column 0: 
	      S  = sqrt( 1.0 + mat[0][0] - mat[1][1] - mat[2][2] ) * 2;
	      X = 0.25 * S;
	      Y = (mat[0][1] + mat[1][0] ) / S;
	      Z = (mat[2][0] + mat[0][2] ) / S;
	      W = (mat[1][2] - mat[2][1] ) / S;
	    } else if ( mat[1][1] > mat[2][2] ) {			// Column 1: 
	      S  = sqrt( 1.0 + mat[1][1] - mat[0][0] - mat[2][2] ) * 2;
	      X = (mat[0][1] + mat[1][0] ) / S;
	      Y = 0.25 * S;
	      Z = (mat[1][2] + mat[2][1] ) / S;
	      W = (mat[0][2] - mat[2][0] ) / S;
	    } else {						// Column 2:
	      S  = sqrt( 1.0 + mat[2][2] - mat[0][0] - mat[1][1] ) * 2;
	      X = (mat[2][0] + mat[0][2] ) / S;
	      Y = (mat[1][2] + mat[2][1] ) / S;
	      Z = 0.25 * S;
	      W = (mat[0][1] - mat[1][0] ) / S;
	    }
	  }

	  //The quaternion is then defined as: Q = | X Y Z W |
	  Vector3d v;
	  v.set(X, Y, Z);	
	  Quaternion qt(W,v);  
	  return qt;
	}

        void set(double s,const Vector3d & v){ m_v=v; m_s=s; }
        const Vector3d& getComplex() const { return m_v; }
        double getReal() const { return m_s; }

    private:
        Vector3d m_v;
        double m_s;
    };

}//emd of nprmlib

#endif //#if !defined( _QUATERNION_H_NPRM_ ) 

