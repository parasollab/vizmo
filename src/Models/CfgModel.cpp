#include "CfgModel.h"

#include "ActiveMultiBodyModel.h"
#include "EnvModel.h"
#include "Vizmo.h"

CfgModel::Shape CfgModel::m_shape = CfgModel::Point;
float CfgModel::m_pointScale = 10;

CfgModel::
CfgModel() : Model(""), CfgType() {
  m_index = -1;
  m_isValid = true;
  m_cc = NULL;
  m_isQuery = false;
}

CfgModel::
CfgModel(const Cfg& _c) : Model(""), CfgType(_c) {
  m_index = -1;
  m_isValid = true;
  m_cc = NULL;
  m_isQuery = false;
}

void
CfgModel::
SetName() {
  ostringstream temp;
  temp << "Node " << m_index;
  m_name = temp.str();
}

void
CfgModel::
SetCfg(const vector<double>& _newCfg) {
  m_v.assign(_newCfg.begin(), _newCfg.end());
}

Point3d
CfgModel::
GetPoint() const {
  shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(m_robotIndex);
  return Point3d(m_v[0], m_v[1], robot->IsPlanar() ? 0 : m_v[2]);
}

void
CfgModel::
Set(size_t _index, CCModel<CfgModel, EdgeModel>* _cc) {
  SetIndex(_index);
  SetCCModel(_cc);
}

void
CfgModel::
Scale(float _scale) {
  m_pointScale = _scale*10;
}

void
CfgModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  Color4 c = m_isValid ? m_color :
    Color4(1 - m_color[0], 1 - m_color[1], 1 - m_color[2], 1);

  switch(m_shape) {
    case Robot:
      {
        shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(m_robotIndex);
        robot->SetColor(c);
        robot->SetRenderMode(m_renderMode);
        robot->ConfigureRender(m_v);
        robot->DrawRender();
        break;
      }

    case Point:
      glColor4fv(c);
      glBegin(GL_POINTS);
      glVertex3dv(GetPoint());
      glEnd();
      break;
  }
}


void
CfgModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  switch(m_shape) {
    case Robot:
      {
        shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(m_robotIndex);
        robot->SetRenderMode(m_renderMode);
        robot->ConfigureRender(m_v);
        robot->DrawSelect();
      }
      break;

    case Point:
      glBegin(GL_POINTS);
      glVertex3dv(GetPoint());
      glEnd();
      break;
  }
}


void
CfgModel::
DrawSelected() {
  glDisable(GL_LIGHTING);
  switch(m_shape) {
    case Robot:
      {
        shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(m_robotIndex);
        robot->SetRenderMode(WIRE_MODE);
        robot->ConfigureRender(m_v);
        robot->DrawSelectedImpl();
      }
      break;
    case Point:
      glPointSize(m_pointScale + 3);
      glDisable(GL_LIGHTING);
      glBegin(GL_POINTS);
      glVertex3dv(GetPoint());
      glEnd();
      break;
  }
}

void
CfgModel::
DrawPathRobot() {
  shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(m_robotIndex);
  robot->RestoreColor();
  robot->SetRenderMode(SOLID_MODE);
  robot->ConfigureRender(m_v);
  robot->DrawRender();
}

void
CfgModel::
Print(ostream& _os) const {
  if(m_index != (size_t)-1)
    _os << "VID: " << m_index << endl;

  _os << "Cfg: ";
  for(const auto& v : m_v)
    _os << v << " ";
  _os << endl;

#ifdef PMPState
  _os << "Vel: ";
  for(const auto& v : m_vel)
    _os << v << " ";
  _os << endl;
#endif

  if(!m_isValid)
    _os << endl << "**** Invalid! ****" << endl;
}
