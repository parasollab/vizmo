/********************************************************************

    Quaternion.cpp    Source File

    Jyh-Ming Lien 03/30/2002
    Computer Science.
    Texas A&M University

*********************************************************************/
#include "Quaternion.h"

namespace mathtool{

    istream& operator>>(istream & in, Quaternion & q )
    {
        return in;
    }

    ostream& operator<<(ostream & out, Quaternion & q )
    {
        
      out<<"Angle: "<<q.m_s<<endl;
      out<<"Vector: "<<q.m_v[0]<<", "<< q.m_v[1]<<", "<< q.m_v[2]<<endl;
      return out;
    }

    Quaternion operator*(const Vector3d & v, const Quaternion & q2)
    {
        Quaternion q1(0,v);
        return q1*q2;
    }
}
