#ifndef CONNECTIONMODEL_H_
#define CONNECTIONMODEL_H_

#include "Plum/GLModel.h"

#include <string>
using namespace std;

#include <Transformation.h>
using namespace mathtool;

class ConnectionModel : public GLModel {
  public:
    enum JointType {NONACTUATED, REVOLUTE, SMAJOINT, SPHERICAL}; //0dof, 1dof, or 2dof rotational joints

    ConnectionModel() : m_globalIndex(m_globalCounter++) {}

    virtual const string GetName() const{return "Connection";}

    size_t GetGlobalIndex() const {return m_globalIndex;}
    size_t GetPreviousIndex() const {return m_previousIndex;}
    size_t GetNextIndex() const {return m_nextIndex;}
    JointType GetJointType() const {return m_jointType;}
    static JointType GetJointTypeFromTag(const string& _tag);
    static string GetTagFromJointType(const JointType& _joint);

    void SetAlpha(double _a) {m_alpha = _a;}
    void SetTheta(double _t) {m_theta = _t;}

    const Transformation& GetTransform() const {return m_currentTransform;}
    void SetTransform(const Transformation& _t) {m_currentTransform = _t;}
    void ComputeTransformFromPrev(const Transformation& _prevTransform);
    bool IsTransformDone() const {return m_transformDone;}
    void SetTransformDone(bool _t) {m_transformDone = _t;}
    void ResetTransform() {m_currentTransform = Transformation(); m_transformDone = false;}
    
    void Draw(GLenum _mode);
    void DrawSelect();

    Transformation DHTransform() const;
    const Transformation& TransformToDHFrame() const {return m_toDHFrame;}
    const Transformation& TransformToBody2() const {return m_toBody2;}

    friend istream& operator>>(istream& _is, ConnectionModel& _c);
    friend ostream& operator<<(ostream& _os, const ConnectionModel& _c);

  private:
    static size_t m_globalCounter;

    size_t m_globalIndex; //ordering information
    size_t m_previousIndex, m_nextIndex; //indices of bodies
    JointType m_jointType; //joint type
    pair<double, double> m_jointLimits[2]; //ranges of joints
    Transformation m_toDHFrame, m_toBody2; //transformations from/to body frames
    double m_alpha, m_theta, m_a, m_d; //dh parameters
    double m_l, m_w; //length and width (SMA joint only)

    Transformation m_currentTransform;
    bool m_transformDone; //has current transform been computed?
};

#endif
