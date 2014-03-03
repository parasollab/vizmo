#include "ConnectionModel.h"

#include "Utilities/VizmoExceptions.h"
#include "Utilities/IO.h"

size_t ConnectionModel::m_globalCounter = 0;

ConnectionModel::JointType
ConnectionModel::GetJointTypeFromTag(const string& _tag){
  if(_tag == "NONACTUATED")
    return ConnectionModel::NONACTUATED;
  else if(_tag == "REVOLUTE")
    return ConnectionModel::REVOLUTE;
  else if (_tag == "SPHERICAL")
    return ConnectionModel::SPHERICAL;
  else if (_tag == "SMAJOINT")
    return ConnectionModel::SMAJOINT;
  else
    throw ParseException(WHERE, "Failed parsing joint type. Choices are Revolute, Spherical, or SMAJoint.");
}

Transformation
ConnectionModel::DHTransform() const {
  if(m_jointType == SMAJOINT) {
    //transform just like a regular joint, except only half of theta
    double ca(cos(m_alpha)), sa(sin(m_alpha)), ct(cos(m_theta/2)), st(sin(m_theta/2));
    Vector3d pos1(m_a, -sa*m_d, ca*m_d);
    Matrix3x3 rot1;
    getMatrix3x3(rot1,
        ct, -st, 0.0,
        st*ca, ct*ca, -sa,
        st*sa, ct*sa, ca);
    //then, translate the correct distance along y and rotate the rest of theta
    double l = (m_theta == 0) ? m_l : 2*m_l/m_theta*st;
    Vector3d pos2(0, l, 0);
    Matrix3x3 rot2;
    getMatrix3x3(rot2,
        ct, -st, 0,
        st, ct, 0,
        0, 0, 1);
    return Transformation(pos1, Orientation(rot1))*Transformation(pos2, Orientation(rot2));
  } else {
    double ca(cos(m_alpha)), sa(sin(m_alpha)), ct(cos(m_theta)), st(sin(m_theta));
    Vector3d pos(m_a, -sa*m_d, ca*m_d);
    Matrix3x3 rot;
    getMatrix3x3(rot,
        ct, -st, 0.0,
        st*ca, ct*ca, -sa,
        st*sa, ct*sa, ca);
    return Transformation(pos, Orientation(rot));
  }
}

void
ConnectionModel::ChangeIndex(int _num){
  m_globalIndex+=_num;
  m_previousIndex+=_num;
  m_nextIndex+=_num;
}

istream&
operator>>(istream& _is, ConnectionModel& _c) {
  //body indices
  _c.m_previousIndex = ReadField<size_t>(_is, WHERE, "Failed reading previous body index");
  _c.m_nextIndex = ReadField<size_t>(_is, WHERE, "Failed reading next body index");

  //Grab the joint type
  string connectionTypeTag = ReadFieldString(_is, WHERE, "Failed reading connection type.");
  _c.m_jointType = ConnectionModel::GetJointTypeFromTag(connectionTypeTag);

  //Grab the joint limits for revolute and spherical joints
  if(_c.m_jointType != ConnectionModel::NONACTUATED){
    _c.m_jointLimits[0].first = _c.m_jointLimits[1].first = -1;
    _c.m_jointLimits[0].second = _c.m_jointLimits[1].second = 1;
    size_t numRange = (_c.m_jointType == ConnectionModel::SPHERICAL) ? 2 : 1;
    for(size_t i = 0; i<numRange; i++) {
      string tok;
      if(_is >> tok) {
        size_t del = tok.find(":");
        if(del == string::npos)
          throw ParseException(WHERE, "Failed reading joint range. Should be delimited by ':'.");

        istringstream minv(tok.substr(0,del)), maxv(tok.substr(del+1, tok.length()));
        if(!(minv >> _c.m_jointLimits[i].first && maxv >> _c.m_jointLimits[i].second))
          throw ParseException(WHERE, "Failed reading joint range.");
      }
      else if(numRange > i) //error. too few tokens provided.
        throw ParseException(WHERE, "Failed reading joint ranges. Not enough provided.");
    }
  }

  //Transformations and DH parameters
  _c.m_toDHFrame = ReadField<Transformation>(_is, WHERE, "Failed reading transformation to DHFrame");

  Vector4d dhparameters = ReadField<Vector4d>(_is, WHERE, "Failed reading DH Parameters.");
  _c.m_alpha = dhparameters[0];
  _c.m_a = dhparameters[1];
  _c.m_d = dhparameters[2];
  _c.m_theta = dhparameters[3];
  if(_c.m_jointType == ConnectionModel::SMAJOINT) {
    _c.m_l = ReadField<double>(_is, WHERE, "Failed to read SMA length parameter.");
    _c.m_w = ReadField<double>(_is, WHERE, "Failed to read SMA width parameter.");
  }
  else
    _c.m_l = _c.m_w = 0;

  _c.m_toBody2 = ReadField<Transformation>(_is, WHERE, "Failed reading transformation to body 2");

  return _is;
}

ostream&
operator<<(ostream& _os, const ConnectionModel& _c){
  _os << _c.m_previousIndex << " " << _c.m_nextIndex << " ";
  if(_c.m_jointType == ConnectionModel::NONACTUATED)
    _os << "NONACTUATED" << endl;
  else if(_c.m_jointType == ConnectionModel::REVOLUTE)
    _os << "REVOLUTE "
      << _c.m_jointLimits[0].first << ":" << _c.m_jointLimits[0].second << endl;
  else if(_c.m_jointType == ConnectionModel::SPHERICAL)
    _os << "SPHERICAL "
      << _c.m_jointLimits[0].first << ":" << _c.m_jointLimits[0].second << " "
      << _c.m_jointLimits[1].first << ":" << _c.m_jointLimits[1].second << endl;
  else if(_c.m_jointType == ConnectionModel::SMAJOINT)
    _os << "SMAJoint "
      << _c.m_jointLimits[0].first << ":" << _c.m_jointLimits[0].second << endl;
  
  _os << _c.m_toDHFrame << "\t"
    << _c.m_alpha << " " << _c.m_a << " " << _c.m_d << " " << _c.m_theta;
  if(_c.m_jointType == ConnectionModel::SMAJOINT)
    _os << " " << _c.m_l << " " << _c.m_w; //SMA joints have extra parameters
  _os << "\t" << _c.m_toBody2;
  return _os;
}
