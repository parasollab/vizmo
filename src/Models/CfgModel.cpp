#include "CfgModel.h"

#include "ActiveMultiBodyModel.h"
#include "EnvModel.h"
#include "Vizmo.h"

CfgModel::Shape CfgModel::m_shape = CfgModel::Point;
float CfgModel::m_pointScale = 10;

CfgModel::
#ifdef PMPCfgMultiRobot
CfgModel() : Model(""), CfgMultiRobot() {
#else
CfgModel() : Model(""), Cfg() {
#endif
  m_index = -1;
  m_isValid = true;
  m_cc = NULL;
  m_isQuery = false;
}

CfgModel::
#ifdef PMPCfgMultiRobot
CfgModel(const CfgMultiRobot& _c) : Model(""), CfgMultiRobot(_c) {
#else
CfgModel(const Cfg& _c) : Model(""), Cfg(_c) {
#endif
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
  SetData(_newCfg);
  /* m_v.assign(_newCfg.begin(), _newCfg.end()); */
}

Point3d
CfgModel::
GetPoint() const {
#ifdef PMPCfgMultiRobot
  //FIXME should return a composite C-space point (not first one only)
  //auto& cfg = GetRobotsCollect()[0];
  shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(0);
  return Point3d(m_v[0], m_v[1], robot->IsPlanar() ? 0 : m_v[2]);
#else
  shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(m_robotIndex);
  return Point3d(m_v[0], m_v[1], robot->IsPlanar() ? 0 : m_v[2]);
#endif
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
#ifdef PMPCfgMultiRobot
        for(auto& cfg : m_robotsCollect) {
          shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(cfg.GetRobotIndex());
          robot->SetColor(c);
          robot->SetRenderMode(m_renderMode);
          robot->ConfigureRender(cfg.GetData());
          robot->DrawRender();
        }
#else
        shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(m_robotIndex);
        robot->SetColor(c);
        robot->SetRenderMode(m_renderMode);
        robot->ConfigureRender(m_v);
        robot->DrawRender();
#endif
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
#ifdef PMPCfgMultiRobot
        for(auto& cfg : m_robotsCollect) {
          shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(cfg.GetRobotIndex());
          robot->SetRenderMode(m_renderMode);
          robot->ConfigureRender(cfg.GetData());
          robot->DrawSelect();
        }
#else
        shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(m_robotIndex);
        robot->SetRenderMode(m_renderMode);
        robot->ConfigureRender(m_v);
        robot->DrawSelect();
#endif
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
#ifdef PMPCfgMultiRobot
        for(auto& cfg : m_robotsCollect) {
          shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(cfg.GetRobotIndex());
          robot->SetRenderMode(WIRE_MODE);
          robot->ConfigureRender(cfg.GetData());
          robot->DrawSelectedImpl();
        }
#else
        shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(m_robotIndex);
        robot->SetRenderMode(WIRE_MODE);
        robot->ConfigureRender(m_v);
        robot->DrawSelectedImpl();
#endif
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
#ifdef PMPCfgMultiRobot
  for(auto& cfg : m_robotsCollect) {
    shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(cfg.GetRobotIndex());
    robot->RestoreColor();
    robot->SetRenderMode(SOLID_MODE);
    robot->ConfigureRender(cfg.GetData());
    robot->DrawRender();
  }
#else
  shared_ptr<ActiveMultiBodyModel> robot = GetVizmo().GetEnv()->GetRobot(m_robotIndex);
  robot->RestoreColor();
  robot->SetRenderMode(SOLID_MODE);
  robot->ConfigureRender(m_v);
  robot->DrawRender();
#endif
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
