#include "RobotInfo.h"
#include <iostream>
#include <cstdlib>

#include "MultiBodyInfo.h"

class IsConnectionGloballyFirst {
  public:
    bool operator()(const plum::CConnectionInfo* _a, const plum::CConnectionInfo* _b) const {
      return _a->m_globalIndex < _b->m_globalIndex;
    }
} connectionComparitor;

Robot::Robot(Base _base, BaseMovement _baseMovement, 
    JointMap _joints, int _bodyIndex) : 
  m_base(_base), m_baseMovement(_baseMovement), 
  m_bodyIndex(_bodyIndex), m_joints(_joints) {
    std::sort(m_joints.begin(), m_joints.end(), connectionComparitor);
  }

Robot::Base Robot::GetBaseFromTag(const string _tag){
  if(_tag == "PLANAR")
    return Robot::PLANAR;
  else if(_tag == "VOLUMETRIC")
    return Robot::VOLUMETRIC;
  else if(_tag == "FIXED")
    return Robot::FIXED;
  else if(_tag == "JOINT")
    return Robot::JOINT;
  else{
    cerr << "Error::Incorrect Base Type Specified::" << _tag << endl;
    cerr << "Choices are:: Planar, Volumetric, Fixed, or Joint" << endl;
    exit(1);
  }
}

Robot::BaseMovement Robot::GetMovementFromTag(const string _tag){
  if(_tag == "ROTATIONAL")
    return Robot::ROTATIONAL;
  else if (_tag == "TRANSLATIONAL")
    return Robot::TRANSLATIONAL;
  else {
    cerr << "Error::Incorrect Movement Type Specified::" << _tag << endl;
    cerr << "Choices are:: Rotational or Translational" << endl;
    exit(1);
  }
}

