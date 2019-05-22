#include "UserPathModel.h"

#include "GUI/MainWindow.h"

#include "Models/AvatarModel.h"
#include "Models/CfgModel.h"
#include "Models/EnvModel.h"
#include "Models/MultiBodyModel.h"
#include "Models/Vizmo.h"

#include "PHANToM/Manager.h"

#include "Utilities/GLUtils.h"

class Camera;


UserPathModel::
UserPathModel(InputType _t) : Model("User Path"),
    m_type(_t), m_finished(false), m_valid(true),
    m_oldPos(), m_newPos(), m_userPath() { }


void
UserPathModel::
Select(GLuint* _index, vector<Model*>& _sel) {
  if(_index)
    _sel.push_back(this);
}


void
UserPathModel::
DrawRender() {
  if(m_valid)
    glColor3f(0, 1, 0);
  else
    glColor3f(1, 0, 0);

  glLineWidth(4);

  glBegin(GL_LINE_STRIP);
  for(auto& point: m_userPath)
    glVertex3dv(point);
  glEnd();
}


void
UserPathModel::
DrawSelect() {
  glLineWidth(4);

  glBegin(GL_LINE_STRIP);
  for(vector<Point3d>::iterator it = m_userPath.begin();
      it != m_userPath.end(); ++it)
    glVertex3dv(*it);
  glEnd();
}


void
UserPathModel::
DrawSelected() {
  glLineWidth(8);

  glBegin(GL_LINE_STRIP);
  for(vector<Point3d>::iterator it = m_userPath.begin();
      it != m_userPath.end(); ++it)
    glVertex3dv(*it);
  glEnd();

  DrawRender();
}


void
UserPathModel::
Print(ostream& _os) const {
  _os << Name() << endl;
  PrintPath(_os);
}


void
UserPathModel::
PrintPath(ostream& _os) const {
  _os << m_userPath.size();
  for(vector<Point3d>::const_iterator it = m_userPath.begin();
      it != m_userPath.end(); ++it)
    _os << endl << *it;
  _os << flush;
}


void
UserPathModel::
SendToPath(const Point3d& _p) {
  /// Consecutive repeats are ignored so that leaving the input device still
  /// creates only one position.
  if(m_userPath.empty() || _p != m_userPath.back()) {
    UpdatePositions(_p);
    UpdateValidity();
    m_userPath.push_back(_p);
  }
}


shared_ptr< vector<CfgModel> >
UserPathModel::
GetCfgs(bool _randomize) const {
  shared_ptr< vector<CfgModel> > cfgs(new vector<CfgModel>(m_userPath.size()));
  if(m_finished) {
    //iterate through each path point and generate a cfg there
    vector<CfgModel>::iterator cit = cfgs->begin();
    vector<Point3d>::const_iterator pit = m_userPath.begin();
    for(;cit != cfgs->end() && pit != m_userPath.end(); ++cit, ++pit) {
      if(_randomize)
        cit->GetRandomCfg(GetVizmo().GetEnv()->GetEnvironment());
      SetCfgPosition(*cit, *pit);
    }
  }
  return cfgs;
}


bool
UserPathModel::
MousePressed(QMouseEvent* _e, Camera* _c) {
  if(m_type == Mouse && _e->buttons() == Qt::LeftButton && !m_finished) {
    //start drawing path
    Point3d p = GLUtils::ProjectToWorld(_e->pos().x(),
        GLUtils::windowHeight - _e->pos().y());
    UpdatePositions(p);
    SendToPath(p);
    AvatarModel* avatar = GetVizmo().GetEnv()->GetAvatar();
    avatar->UpdatePosition(p);
    avatar->SetInputType(AvatarModel::Mouse);
    avatar->Enable();
    return true;
  }
  return false;
}


bool
UserPathModel::
MouseReleased(QMouseEvent* _e, Camera* _c) {
  if(m_type == Mouse && !m_finished && (m_userPath.begin() != m_userPath.end())) {
    m_finished = true;
    GetVizmo().GetEnv()->GetAvatar()->Disable();
    return true;
  }
  return false;
}


bool
UserPathModel::
MouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_type == Mouse && !m_finished) {
    //get current mouse position
    Point3d p = GLUtils::ProjectToWorld(_e->pos().x(),
        GLUtils::windowHeight - _e->pos().y());
    SendToPath(p);
    return true;
  }
  return false;
}


bool
UserPathModel::
KeyPressed(QKeyEvent* _e) {
  if(m_type == Mouse)
    return false;

  if(_e->key() == Qt::Key_Space) {
    m_finished = true;
    return true;
  }

  return false;
}

ostream&
operator<<(ostream& _os, const UserPathModel& _upm) {
  _os << "Type ";  
  switch (_upm.m_type) {
    case UserPathModel::Mouse:
      _os << "Mouse" << endl;
      break;
    case UserPathModel::CameraPath:
      _os << "CameraPath" << endl;
      break;
    case UserPathModel::Haptic:
      _os << "Haptic" << endl;
      break;
  }
  _os << "Validity " << _upm.m_valid << endl;
  _os << "NumPoints " << _upm.m_userPath.size() << endl;

  for(const auto& point: _upm.m_userPath)
    _os << point << endl;
  return _os;
}

istream& 
operator>>(istream& _is, UserPathModel& _upm) {  
  ///\todo Incorporate error checking into path format
 
  // Reading input type
  string temp;
  _is >> temp >> temp;

  if(temp == "Mouse") 
    _upm.m_type = UserPathModel::Mouse;
  else if(temp == "CameraPath")
    _upm.m_type = UserPathModel::CameraPath;
  else if(temp == "Haptic")
    _upm.m_type = UserPathModel::Haptic;

  //read validity
  _is >> temp >> _upm.m_valid;
  
  //read points
  size_t numpoints;
  _is >> temp >> numpoints;
  _upm.m_userPath.clear();
  _upm.m_userPath.resize(numpoints);
  for(auto& point : _upm.m_userPath)
    _is >> point;
  _upm.m_finished = true; 
  return _is;
}

void
UserPathModel::
UpdatePositions(const Point3d& _p) {
  m_oldPos = m_newPos;
  SetCfgPosition(m_newPos, _p);
}


void
UserPathModel::
UpdateValidity() {
  if(!GetVizmo().CollisionCheck(m_newPos))
    m_valid = false;
}


void
UserPathModel::
SetCfgPosition(CfgModel& _cfg, const Point3d& _p) const {
  //copy data from the point to the cfg data until the end of either is reached
  for(size_t i = 0; i < _cfg.PosDOF() && i < 3; ++i)
    _cfg[i] = _p[i];
}
