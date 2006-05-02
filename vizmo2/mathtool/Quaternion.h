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

        void set(double s,const Vector3d & v){ m_v=v; m_s=s; }
        const Vector3d& getComplex() const { return m_v; }
        double getReal() const { return m_s; }

    private:
        Vector3d m_v;
        double m_s;
    };

}//emd of nprmlib

#endif //#if !defined( _QUATERNION_H_NPRM_ ) 

