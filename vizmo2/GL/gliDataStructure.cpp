#include "gliDataStructure.h"
#include <qgl.h>

vector<gliObj>  g_PickedSceneObjs;
vector<gliObj>& gliGetPickedSceneObjs()
{
    return g_PickedSceneObjs;
}

    
void gliTransform::glTransform(){
    static double r2a=180/PI; //radian to angle
    glTranslated(m_Pos[0],m_Pos[1],m_Pos[2]);
    const Vector3d& v=m_q.getComplex();
    double c_2=m_q.getReal();
    double s_2=v.norm();
    double t=atan2(s_2,c_2)*2*r2a;
    glRotated(t,v[0],v[1],v[2]);
}

