#ifndef GLTRANSFORM_H_
#define GLTRANSFORM_H_

#include <vector>
#include <string>
using namespace std;

#include <RotationConversions.h>
using namespace mathtool;

//Defines function and data for tansfromation.
class GLTransform {
  public:

    GLTransform() : m_scale(1, 1, 1) {}
    virtual ~GLTransform() {}

    void Transform();

    //Translation
    Vector3d& Translation() {return m_pos;}
    const Vector3d& Translation() const {return m_pos;}

    //Scale
    Vector3d& Scale() {return m_scale;}
    const Vector3d& Scale() const {return m_scale;}

    //Rotation
    Vector3d& Rotation() {return m_rot;}
    const Vector3d& Rotation() const {return m_rot;}
    Quaternion& RotationQ() {return m_rotQ;}
    const Quaternion& RotationQ() const {return m_rotQ;}

  protected:
    Vector3d m_pos, m_scale;
    Vector3d m_rot;
    Quaternion m_rotQ; //Rotation
};

#endif
