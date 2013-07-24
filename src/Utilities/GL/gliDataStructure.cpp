#include "gliDataStructure.h"
#include <qgl.h>

vector<GLModel*>  g_PickedSceneObjs;
vector<GLModel*>& gliGetPickedSceneObjs()
{
    return g_PickedSceneObjs;
}


void gliTransform::glTransform(){
    static double r2a=180/PI; //radian to angle
    glTranslated(m_pos[0], m_pos[1], m_pos[2]);
    const Vector3d& v = m_q.imaginary();
    double c_2 = m_q.real();
    double s_2 = v.norm();
    double t = atan2(s_2,c_2)*2*r2a;
    glRotated(t, v[0], v[1], v[2]);
    glScaled(m_scale[0], m_scale[1], m_scale[2]);
}

void gliTransform::Euiler2Quaternion()
{
	double cx_2=cos(rx()/2); double sx_2=sin(rx()/2);
	double cy_2=cos(ry()/2); double sy_2=sin(ry()/2);
	double cz_2=cos(rz()/2); double sz_2=sin(rz()/2);
	Quaternion qx(cx_2,sx_2*Vector3d(1,0,0));
	Quaternion qy(cy_2,sy_2*Vector3d(0,1,0));
	Quaternion qz(cz_2,sz_2*Vector3d(0,0,1));
	Quaternion nq=qz*qy*qx; //new q
	q(nq.normalize()); //set new q
}

void gliTransform::glScale(){
  glScaled(m_scale[0],m_scale[1],m_scale[2]);
}

string gliTransform::GetObjName(){
  return ObjName;
}
