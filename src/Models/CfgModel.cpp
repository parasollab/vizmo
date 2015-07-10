#include "CfgModel.h"

#include "ActiveMultiBodyModel.h"
#include "EnvModel.h"
#include "Vizmo.h"

CfgModel::Shape CfgModel::m_shape = CfgModel::Point;
float CfgModel::m_pointScale = 10;

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
        robot->BackUp();
        robot->SetRenderMode(m_renderMode);
        robot->Configure(m_v);
        robot->DrawRender();
        robot->Restore();
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
        shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(m_robotIndex);
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
      glVertex3dv(GetPoint());
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
