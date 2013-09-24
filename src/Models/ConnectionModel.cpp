#include "ConnectionModel.h"

#include "Utilities/Exceptions.h"
#include "Utilities/IOUtils.h"

size_t ConnectionModel::m_globalCounter = 0;

ConnectionModel::JointType
ConnectionModel::GetJointTypeFromTag(const string& _tag){
  if(_tag == "NONACTUATED")
    return ConnectionModel::NONACTUATED;
  else if(_tag == "REVOLUTE")
    return ConnectionModel::REVOLUTE;
  else if (_tag == "SPHERICAL")
    return ConnectionModel::SPHERICAL;
  else
    throw ParseException(WHERE, "Failed parsing joint type. Choices are Revolute or Spherical.");
}

Transformation
ConnectionModel::DHTransform() const {
  double ca(cos(m_alpha)), sa(sin(m_alpha)), ct(cos(m_theta)), st(sin(m_theta));
  Vector3d pos(m_a, -sa*m_d, ca*m_d);
  Matrix3x3 rot;
  getMatrix3x3(rot,
      ct, -st, 0.0,
      st*ca, ct*ca, -sa,
      st*sa, ct*sa, ca);
  return Transformation(pos, Orientation(rot));
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
  if(_c.m_jointType == ConnectionModel::REVOLUTE || _c.m_jointType == ConnectionModel::SPHERICAL){
    _c.m_jointLimits[0].first = _c.m_jointLimits[1].first = -1;
    _c.m_jointLimits[0].second = _c.m_jointLimits[1].second = 1;
    size_t numRange = (_c.m_jointType == ConnectionModel::REVOLUTE) ? 1 : 2;
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
      else if(numRange == 2 && i==1) //error. only 1 token provided.
        throw ParseException(WHERE, "Failed reading joint ranges. Only one provided.");
    }
  }

  //Transformations and DH parameters
  _c.m_toDHFrame = ReadField<Transformation>(_is, WHERE, "Failed reading transformation to DHFrame");

  Vector4d dhparameters = ReadField<Vector4d>(_is, WHERE, "Failed reading DH Parameters.");
  _c.m_alpha = dhparameters[0];
  _c.m_a = dhparameters[1];
  _c.m_d = dhparameters[2];
  _c.m_theta = dhparameters[3];

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
  _os << _c.m_toDHFrame << "\t"
    << _c.m_alpha << " " << _c.m_a << " " << _c.m_d << " " << _c.m_theta << "\t"
    << _c.m_toBody2;
  return _os;
}
