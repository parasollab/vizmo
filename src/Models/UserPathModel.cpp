
/*
 *  This method provides a tool for capturing approximate paths in two
 *  dimensions. The path is entered with the mouse by click-and-drag. Collision
 *  detection on the mouse position is provided by creating a square robot with
 *  side length = environment resolution.
 */

#include "UserPathModel.h"

#include "GUI/MainWindow.h"
#include "GUI/RobotAvatar.h"

#include "Models/CfgModel.h"
#include "Models/EnvModel.h"
#include "Models/MultiBodyModel.h"
#include "Models/Vizmo.h"

#include "PHANToM/Manager.h"

#include "Utilities/GLUtils.h"

class Camera;

UserPathModel::
UserPathModel(MainWindow* _mainWindow, InputType _t) :
    Model("User Path"), m_checkCollision(false), m_mainWindow(_mainWindow),
    m_type(_t), m_finished(false), m_valid(true), m_oldPos(), m_newPos(),
    m_userPath(), m_avatar(NULL) {
  if(m_type == Haptic)
    UpdatePositions(GetVizmo().GetManager()->GetWorldPos());
}

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
  for(vector<Point3d>::iterator it = m_userPath.begin(); it != m_userPath.end(); ++it)
    glVertex3dv(*it);
  glEnd();
}

void
UserPathModel::
DrawSelect() {
  glLineWidth(4);
  for(vector<Point3d>::iterator it = m_userPath.begin(); it != m_userPath.end(); ++it)
    glVertex3dv(*it);
  glEnd();
}

void
UserPathModel::
DrawSelected() {
  glLineWidth(8);
  glBegin(GL_LINE_STRIP);
  for(vector<Point3d>::iterator it = m_userPath.begin(); it != m_userPath.end(); ++it)
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

shared_ptr< vector<CfgModel> >
UserPathModel::
GetCfgs() {
  shared_ptr< vector<CfgModel> > cfgs(new vector<CfgModel>(m_userPath.size()));
  if(m_finished) {
    size_t count = 0;
    for(vector<Point3d>::const_iterator it = m_userPath.begin();
        it != m_userPath.end(); ++it) {
      (*cfgs)[count] = CfgModel();
      (*cfgs)[count].SetCfg(Point3dToVector(*it));
      count++;
    }
  }
  return cfgs;
}


bool
UserPathModel::
MousePressed(QMouseEvent* _e, Camera* _c) {
  if(m_type == Mouse && _e->buttons() == Qt::LeftButton && !m_finished) {
    //start drawing path
    Point3d p = ProjectToWorld(_e->pos().x(), g_height - _e->pos().y(),
        Point3d(), Vector3d(0, 0, 1));
    UpdatePositions(p);
    SendToPath(p);
    m_avatar = new RobotAvatar(&m_newPos, RobotAvatar::Mouse);
    return true;
  }

  return false;
}

bool
UserPathModel::
MouseReleased(QMouseEvent* _e, Camera* _c) {
  if(m_type == Mouse && !m_finished && (m_userPath.begin() != m_userPath.end())) {
    m_finished = true;
    delete m_avatar;
    m_avatar = NULL;
    return true;
  }

  return false;
}

bool
UserPathModel::
MouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_type == Mouse && !m_finished) {
    //get current mouse position
    Point3d p = ProjectToWorld(_e->pos().x(), g_height - _e->pos().y(),
        Point3d(), Vector3d(0, 0, 1));
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

void
UserPathModel::
UpdatePositions(const Point3d& _p) {
  m_oldPos = m_newPos;
  m_newPos.SetCfg(Point3dToVector(_p));
}

void
UserPathModel::
UpdateValidity() {
  //pair<bool, double> result = GetVizmo().VisibilityCheck(m_newPos, m_oldPos);}
  if(!GetVizmo().CollisionCheck(m_newPos))
    m_valid = false;
}

void
UserPathModel::
SendToPath(const Point3d& _p) {
  //check that this position is new
  if(m_userPath.empty() || _p != m_userPath.back()) {
    UpdatePositions(_p);
    UpdateValidity();
    m_userPath.push_back(_p);
  }
}

vector<double>
UserPathModel::
Point3dToVector(const Point3d& _p) {
  vector<double> data(m_newPos.DOF(), 0.);
  copy(_p.begin(), _p.end(), data.begin());
  return data;
}

