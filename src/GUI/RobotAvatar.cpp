#include "RobotAvatar.h"

#include "GUI/GLWidget.h"
#include "GUI/MainWindow.h"

#include "Models/CfgModel.h"
#include "Models/TempObjsModel.h"
#include "Models/Vizmo.h"

#include "PHANToM/Manager.h"

#include "Utilities/Camera.h"
#include "Utilities/GLUtils.h"

RobotAvatar::
RobotAvatar(InputType _t) : m_input(_t) {
  //create temp objs model for avatar display
  m_tempObjs = new TempObjsModel();

  //create avatar and add it to tempObjs
  m_avatar = new CfgModel();
  m_avatar->SetShape(CfgModel::Robot);
  m_tempObjs->AddCfg(m_avatar);

  //configure the signals/slots
  Connect();

  //get the camera pointer
  m_camera = GetMainWindow()->GetGLWidget()->GetCurrentCamera();
}

RobotAvatar::
RobotAvatar(CfgModel* _initialCfg, InputType _t) : m_input(_t) {
  //create temp objs model for avatar display
  m_tempObjs = new TempObjsModel();

  //create avatar, initialize, and add it to tempObjs
  m_avatar = new CfgModel();
  m_avatar->SetCfg(_initialCfg->GetDataCfg());
  m_avatar->SetShape(CfgModel::Robot);
  m_tempObjs->AddCfg(m_avatar);

  //configure the signals/slots
  Connect();

  //get the camera pointer
  m_camera = GetMainWindow()->GetGLWidget()->GetCurrentCamera();
}

RobotAvatar::
~RobotAvatar() {
  Disconnect();
  delete m_tempObjs;
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
      //throw exception here
      cerr << "\nError: failed to identify input type for RobotAvatar!\n"
           << "\tAborting execution from SetupConnections() in "
           << "RobotAvatar.cpp.\n" << flush;
      exit(-1);
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
      //throw exception here
      cerr << "\nError: failed to identify input type for RobotAvatar!\n"
           << "\tAborting execution from SetupConnections() in "
           << "RobotAvatar.cpp.\n" << flush;
      exit(-1);
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

