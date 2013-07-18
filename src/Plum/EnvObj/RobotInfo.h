#ifndef ROBOT_H_
#define ROBOT_H_

#include <vector>
#include <string>

using namespace std;

namespace plum {
  class CConnectionInfo;
}

struct Robot{
  enum Base {PLANAR, VOLUMETRIC, FIXED, JOINT}; //2D plane vs 3D
  enum BaseMovement {ROTATIONAL, TRANSLATIONAL}; //rotation+translation, just translation, no movement
  typedef plum::CConnectionInfo* Joint;
  typedef vector<Joint> JointMap;
  //index of next body,
  //joint type
  typedef JointMap::iterator JointIT;

  Base m_base; //Base Type
  BaseMovement m_baseMovement; //can the base rotate? is the base fixed?
  int m_bodyIndex; //free body index for base

  Robot(Base _base, BaseMovement _baseMovement,
      JointMap _joints, int _bodyIndex);

  static Base GetBaseFromTag(const string _tag);
  static BaseMovement GetMovementFromTag(const string _tag);
  const JointMap& GetJointMap() const {return m_joints;}

  private:
  JointMap m_joints; //Joints associated with robot
};

#endif
