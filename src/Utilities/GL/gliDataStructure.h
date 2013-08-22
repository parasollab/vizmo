#ifndef _GLI_DATASTRUCTURE_H_
#define _GLI_DATASTRUCTURE_H_

#include <vector>
#include <string>
using namespace std;

#include <RotationConversions.h>
using namespace mathtool;

/**
 * Defines function and data for tansfromation.
 */
class gliTransform {
  public:

    gliTransform(){
      m_pos[0]=m_pos[1]=m_pos[2]=
        m_rot[0]=m_rot[1]=m_rot[2]=0;
      m_scale[0]=m_scale[1]=m_scale[2]=1;
    }
    virtual ~gliTransform() {}

    void glTransform();
    void Euler2Quaternion();
    void glScale();

    Matrix3x3 getMatrix() {
      Matrix3x3 m;
      convertFromQuaternion(m, m_q);
      return m;
    }
    Vector3d MatrixToEuler(Matrix3x3 m) {
      EulerAngle e;
      convertFromMatrix(e, m);
      return Vector3d(e.alpha(), e.beta(), e.gamma());
    }

    ///Translation
    double& tx() { return m_pos[0]; }
    double& ty() { return m_pos[1]; }
    double& tz() { return m_pos[2]; }
    const double& tx() const { return m_pos[0]; }
    const double& ty() const { return m_pos[1]; }
    const double& tz() const { return m_pos[2]; }

    ///Scale
    //the scale may be difficult for some models....
    virtual void Scale(double x, double y, double z) {
      m_scale[0]=x; m_scale[1]=y; m_scale[2]=z; };

    double& sx() { return m_scale[0]; }
    double& sy() { return m_scale[1]; }
    double& sz() { return m_scale[2]; }
    const double& sx() const { return m_scale[0]; }
    const double& sy() const { return m_scale[1]; }
    const double& sz() const { return m_scale[2]; }

    ///Rotation
    double& rx() { return m_rot[0]; }
    double& ry() { return m_rot[1]; }
    double& rz() { return m_rot[2]; }
    const double& rx() const { return m_rot[0]; }
    const double& ry() const { return m_rot[1]; }
    const double& rz() const { return m_rot[2]; }

    //Get&Set Quaternion
    const Quaternion& q() const {return m_q;}
    void q(const Quaternion& q){ m_q=q; }

  protected:
    double m_pos[3];         //Position
    double m_rot[3];         //Rotation
    double m_scale[3];       //Sacle
    Quaternion m_q;          //Rotation
};

typedef gliTransform* gliObj;

//definition for a 2D box
class gliBox{
  public:
    gliBox(){ l=r=t=b=0; }
    double l,r,t,b; //left, right,top,bottom
};

///////////////////////////////////////////////////////////////////////////////
class GLModel;
vector<GLModel*>& gliGetPickedSceneObjs();

#endif //_GLI_DATASTRUCTURE_H_

