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
class GLTransform {
  public:

    GLTransform() : m_scale(1, 1, 1) {}
    virtual ~GLTransform() {}

    void glTransform();

    ///Translation
    double& tx() {return m_pos[0];}
    double& ty() {return m_pos[1];}
    double& tz() {return m_pos[2];}
    const double& tx() const {return m_pos[0];}
    const double& ty() const {return m_pos[1];}
    const double& tz() const {return m_pos[2];}

    //Scale
    double& sx() {return m_scale[0];}
    double& sy() {return m_scale[1];}
    double& sz() {return m_scale[2];}
    const double& sx() const {return m_scale[0];}
    const double& sy() const {return m_scale[1];}
    const double& sz() const {return m_scale[2];}

    ///Rotation
    double& rx() {return m_rot[0];}
    double& ry() {return m_rot[1];}
    double& rz() {return m_rot[2];}
    const double& rx() const {return m_rot[0];}
    const double& ry() const {return m_rot[1];}
    const double& rz() const {return m_rot[2];}

    //Get&Set Quaternion
    Quaternion& q() {return m_q;}
    const Quaternion& q() const {return m_q;}

  protected:
    Vector3d m_pos, m_scale;
    Vector3d m_rot;
    Quaternion m_q;          //Rotation
};

typedef GLTransform* gliObj;

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

