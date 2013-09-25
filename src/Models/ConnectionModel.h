#ifndef CONNECTIONMODEL_H_
#define CONNECTIONMODEL_H_

#include <string>
using namespace std;

#include <Transformation.h>
using namespace mathtool;

class ConnectionModel {
  public:
    enum JointType {NONACTUATED, REVOLUTE, SPHERICAL}; //0dof, 1dof, or 2dof rotational joints

    ConnectionModel() : m_globalIndex(m_globalCounter++), m_previousIndex(-1),
    m_nextIndex(-1), m_jointType(NONACTUATED), m_alpha(0), m_theta(0), m_a(0), m_d(0) {}

    size_t GetGlobalIndex() const {return m_globalIndex;}
    size_t GetPreviousIndex() const {return m_previousIndex;}
    size_t GetNextIndex() const {return m_nextIndex;}
    JointType GetJointType() const {return m_jointType;}
    static JointType GetJointTypeFromTag(const string& _tag);

    void SetAlpha(double _a) {m_alpha = _a;}
    void SetTheta(double _t) {m_theta = _t;}

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
};

#endif