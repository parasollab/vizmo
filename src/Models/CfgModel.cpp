#include "CfgModel.h"

#include "MPProblem/Robot/Robot.h"

#include "ActiveMultiBodyModel.h"
#include "EnvModel.h"
#include "Vizmo.h"

CfgModel::Shape CfgModel::m_shape = CfgModel::PointShape;
float CfgModel::m_pointScale = 10;

CfgModel::
CfgModel(const size_t _index) : Model("Cfg"), 
    CfgType(GetVizmo().GetEnv()->GetRobot(_index)),
    m_robotIndex(_index), m_mutex(new mutex())  { }

CfgModel::
CfgModel(const Vector3d& _vec, const size_t _index) : Model("Cfg"),
    CfgType(_vec, GetVizmo().GetEnv()->GetRobot(_index)), 
    m_robotIndex(_index), m_mutex(new mutex()) { }

CfgModel::
CfgModel(Robot* _robot) : Model("Cfg"), 
    CfgType(_robot),
    m_mutex(new mutex())  { 
  const auto& robots = GetVizmo().GetEnv()->GetProblem()->GetRobots();
  for(size_t i = 0; i < robots.size(); i++) {
    if(robots[i].get() == _robot) {
      m_robotIndex = i;
      break;
    }
  }
}

//TODO::Add a way to get robot index out of Cfg. 
//      For now, it is always 0 in our current uses anyways
CfgModel::
CfgModel(const CfgType& _c) : Model("Cfg"), CfgType(_c), 
    m_robotIndex(0), m_mutex(new mutex()) { }
    //m_robotIndex(_c.m_robotIndex), m_mutex(new mutex()) { }

CfgModel::
CfgModel(const CfgModel& _c) : Model("Cfg"), CfgType(_c),
    m_robotIndex(0), m_mutex(new mutex()) { }
    //m_robotIndex(_c.m_robotIndex), m_mutex(new mutex()) { }

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
  m_dofs.assign(_newCfg.begin(), _newCfg.end());
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
    case RobotShape:
      {
        shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobotModel(m_robotIndex);
        robot->SetColor(c);
        robot->SetRenderMode(m_renderMode);
        robot->ConfigureRender(m_dofs);
        robot->DrawRender();
        break;
      }

    case PointShape:
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
    case RobotShape:
      {
        shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobotModel(m_robotIndex);
        robot->SetRenderMode(m_renderMode);
        robot->ConfigureRender(m_dofs);
        robot->DrawSelect();
      }
      break;

    case PointShape:
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
    case RobotShape:
      {
        shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobotModel(m_robotIndex);
        robot->SetRenderMode(WIRE_MODE);
        robot->ConfigureRender(m_dofs);
        robot->DrawSelectedImpl();
      }
      break;
    case PointShape:
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
  shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobotModel(m_robotIndex);
  robot->RestoreColor();
  robot->SetRenderMode(SOLID_MODE);
  robot->ConfigureRender(m_dofs);
  robot->DrawRender();
}

void
CfgModel::
Print(ostream& _os) const {
  if(m_index != (size_t)-1)
    _os << "VID: " << m_index << endl;

  _os << "Cfg: ";
  for(const auto& v : m_dofs)
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
