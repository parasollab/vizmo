#include "CfgModel.h"

#include "RobotModel.h"
#include "Vizmo.h"

double CfgModel::m_defaultDOF = 0;
CfgModel::Shape CfgModel::m_shape = CfgModel::Point;
float CfgModel::m_pointScale = 10;
bool CfgModel::m_isPlanarRobot = false;
bool CfgModel::m_isVolumetricRobot = false;
bool CfgModel::m_isRotationalRobot = false;


CfgModel::
CfgModel() : Model(""), Cfg() {
  m_index = -1;
  m_isValid = true;
  m_cc = NULL;
  m_isQuery = false;
}


CfgModel::
CfgModel(const Cfg& _c) : Model(""), Cfg(_c) {
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

  RobotModel* robot = GetVizmo().GetRobot();
  if(m_isValid) {
    glColor4fv(m_color);
    robot->SetColor(m_color);
  }
  else {
    Color4 ic(1.0-m_color[0], 1.0-m_color[1], 1.0-m_color[2], 0.0);
    glColor4fv(ic);
    robot->SetColor(ic); //Invert colors. Black case?
  }

  switch(m_shape) {
    case Robot:
      robot->BackUp();
      robot->SetRenderMode(m_renderMode);
      robot->Configure(m_v);
      robot->DrawRender();
      robot->Restore();
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
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  switch(m_shape) {
    case Robot:
      {
        RobotModel* robot = GetVizmo().GetRobot();
        robot->BackUp();
        robot->SetRenderMode(m_renderMode);
        robot->Configure(m_v);
        robot->DrawSelect();
        robot->Restore();
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
        RobotModel* robot = GetVizmo().GetRobot();
        robot->BackUp();
        robot->SetRenderMode(WIRE_MODE);
        robot->Configure(m_v);
        robot->DrawSelected();
        robot->Restore();
      }
      break;
    case Point:
      glPointSize(m_pointScale + 3);
      glDisable(GL_LIGHTING);
      glBegin(GL_POINTS);
      if(m_isPlanarRobot)
        glVertex3d(m_v[0], m_v[1], 0);
      else if(m_isVolumetricRobot)
        glVertex3d(m_v[0], m_v[1], m_v[2]);
      glEnd();
      break;
  }
}


void
CfgModel::
Print(ostream& _os) const {
  /// \todo Function is not used: this information is controlled by
  ///       VizmoRoadmapGUI::printNodeCfg() in roadmap.cpp, which calls
  ///       CfgModel::GetNodeInfo().
  _os << "Node ID = " << m_index << endl
      << "Cfg ( " << *this << " )" << endl;

  if(!m_isValid)
    _os << "**** IS IN COLLISION!! ****" << endl;
}
