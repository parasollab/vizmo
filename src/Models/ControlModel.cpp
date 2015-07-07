#include "ControlModel.h"

ControlModel::
ControlModel(MultiBodyModel* _owner) : m_owner(_owner), m_control(6, 0) {
}

void
ControlModel::
Read(istream& _is) {
  m_control.clear();
  string control;
  getline(_is, control);
  istringstream iss(control);
  double v;
  while(iss >> v)
    m_control.push_back(v);
}

ostream&
operator<<(ostream& _os, const ControlModel& _c) {
  ostringstream oss;
  for(const auto& v : _c.m_control)
    oss << v << " ";
  string s = oss.str();
  return _os << s.substr(0, s.length()-1);
}
