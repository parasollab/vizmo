#include "RobotAvatar.h"

#include "GUI/GLWidget.h"
#include "GUI/MainWindow.h"

#include "Models/TempObjsModel.h"
#include "Models/Vizmo.h"

#include "PHANToM/Manager.h"

#include "Utilities/Camera.h"
#include "Utilities/GLUtils.h"
#include "Utilities/VizmoExceptions.h"

RobotAvatar::
RobotAvatar(InputType _t) : m_input(_t) {
  //create temp objs model for avatar display
  m_tempObjs = new TempObjsModel();

  //create avatar and add it to tempObjs
  m_avatar = new CfgModel();
  m_prev = new CfgModel(*m_avatar);
  m_avatar->SetShape(CfgModel::Robot);
  m_tempObjs->AddCfg(m_avatar);

  //get the camera pointer
  m_camera = GetMainWindow()->GetGLWidget()->GetCurrentCamera();
}

RobotAvatar::
RobotAvatar(CfgModel* _initialCfg, InputType _t) : m_input(_t) {
  //create temp objs model for avatar display
  m_tempObjs = new TempObjsModel();

  //create avatar, initialize, and add it to tempObjs
  m_avatar = new CfgModel(*_initialCfg);
  m_prev = new CfgModel(*m_avatar);
  m_avatar->SetShape(CfgModel::Robot);
  m_tempObjs->AddCfg(m_avatar);

  //get the camera pointer
  m_camera = GetMainWindow()->GetGLWidget()->GetCurrentCamera();
}

RobotAvatar::
~RobotAvatar() {
  delete m_tempObjs; //also deletes m_avatar
  delete m_prev;
}

//Attach the main timer to the appropriate update slot
void
RobotAvatar::
Connect() {
  switch(m_input) {
    case Mouse:
      connect(GetMainWindow()->GetMainClock(), SIGNAL(timeout()),
          this, SLOT(UpdateMouse()));
      break;
    case CameraPath:
      connect(GetMainWindow()->GetMainClock(), SIGNAL(timeout()),
          this, SLOT(UpdateCameraPath()));
      break;
    case Haptic:
      connect(GetMainWindow()->GetMainClock(), SIGNAL(timeout()),
          this, SLOT(UpdateHaptic()));
      break;
    default:
      ostringstream msg;
      msg << "\nError: failed to identify input type for RobotAvatar!\n"
          << "\tAborting execution from Disonnect() in RobotAvatar.cpp.\n";
      throw PMPLException("Unknown input type", WHERE, msg.str());
  }
}

//break update connection
void
RobotAvatar::
Disconnect() {
  switch(m_input) {
    case Mouse:
      disconnect(GetMainWindow()->GetMainClock(), SIGNAL(timeout()),
          this, SLOT(UpdateMouse()));
      break;
    case CameraPath:
      disconnect(GetMainWindow()->GetMainClock(), SIGNAL(timeout()),
          this, SLOT(UpdateCameraPath()));
      break;
    case Haptic:
      disconnect(GetMainWindow()->GetMainClock(), SIGNAL(timeout()),
          this, SLOT(UpdateHaptic()));
      break;
    default:
      ostringstream msg;
      msg << "\nError: failed to identify input type for RobotAvatar!\n"
          << "\tAborting execution from Disonnect() in RobotAvatar.cpp.\n";
      throw PMPLException("Unknown input type", WHERE, msg.str());
  }
}

//update avatar data based on mouse input
void
RobotAvatar::
UpdateMouse() {
  //get mouse pos
  Point2d m = GetMainWindow()->GetGLWidget()->GetMouse();
  Point3d pos = ProjectToWorld(m[0], m[1], Point3d(), Vector3d(0, 0, 1));

  //update avatar data
  vector<double> data = m_avatar->GetDataCfg();
  copy(pos.begin(), pos.end() - 1, data.begin());
  m_avatar->SetCfg(data);
}

//update avatar data based on camera path input
void
RobotAvatar::
UpdateCameraPath() {
  //get camera
  Point3d pos = m_camera->GetEye();
  Point3d rot = -m_camera->GetWindowZ();

  //update avatar data
  vector<double> data = m_avatar->GetDataCfg();
  copy(pos.begin(), pos.end(), data.begin());
  copy(rot.begin(), rot.end(), data.begin() + 3);
  m_avatar->SetCfg(data);
}

//update avatar data based on haptic input
void
RobotAvatar::
UpdateHaptic() {
  //get phantom position and rotation
  Point3d pos = GetVizmo().GetManager()->GetWorldPos();
  Point3d rot = GetVizmo().GetManager()->GetWorldRot();

  //update avatar data
  vector<double> data = m_avatar->GetDataCfg();
  copy(pos.begin(), pos.end(), data.begin());
  copy(rot.begin(), rot.end(), data.begin() + 3);
  m_avatar->SetCfg(data);
}

//bring the mouse cursor to the avatar
void
RobotAvatar::
SummonMouse() {
  vector<double> pos = m_avatar->GetPosition();
  Point3d worldPos(0., 0., 0.);
  copy(pos.begin(), pos.end(), worldPos.begin());
  Point3d screenPos = ProjectToWindow(worldPos);
  QCursor::setPos(int(screenPos[0]), int(screenPos[1]));
}
