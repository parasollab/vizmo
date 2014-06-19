/*
 *  This method provides a tool for capturing approximate paths in two
 *  dimensions. The path is entered with the mouse by click-and-drag. Collision
 *  detection on the mouse position is provided by creating a square robot with
 *  side length = environment resolution.
 */

#include "UserPathModel.h"

#include <QtGui>

#include "Models/CfgModel.h"
#include "Models/EnvModel.h"
#include "Models/MultiBodyModel.h"
#include "Models/Vizmo.h"
#include "Utilities/GLUtils.h"

class Camera;

UserPathModel::
UserPathModel() : Model("User Path"), m_finished(false), m_drawing(false),
    m_userPath(), m_oldPos(), m_newPos(), m_valid(true) { }

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
  glBegin(GL_LINE_STRIP);
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
  if(_e->buttons() == Qt::LeftButton && !m_finished) {
    //start drawing path
    m_drawing = true;

    //get first point
    Point3d p = ProjectToWorld(_e->pos().x(), g_height - _e->pos().y(),
        Point3d(), Vector3d(0, 0, 1));

    //set up new/old positions (two calls for initialize)
    UpdatePositions(p);
    UpdatePositions(p);

    //collision check current position & set color
    UpdateValidity();

    //add first point to path
    m_userPath.push_back(p);
    return true;
  }

  return false;
}

bool
UserPathModel::
MouseReleased(QMouseEvent* _e, Camera* _c) {
  if(!m_finished && (m_userPath.begin() != m_userPath.end())) {
    m_finished = true;
    m_drawing = false;
    return true;
  }

  return false;
}

bool
UserPathModel::
MouseMotion(QMouseEvent* _e, Camera* _c) {
  if(!m_finished && m_drawing) {
    //get current mouse position
    Point3d p = ProjectToWorld(_e->pos().x(), g_height - _e->pos().y(),
        Point3d(), Vector3d(0, 0, 1));

    //check that this position is not already in the path
    if(p != m_userPath.back()) {
      //update new/old positions
      UpdatePositions(p);

      //collision check new position & set color
      UpdateValidity();

      //store new point
      m_userPath.push_back(p);
      return true;
    }
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
  GetVizmo().CollisionCheck(m_newPos);
  if(!m_newPos.IsValid())
    m_valid = false;
}

vector<double>
UserPathModel::
Point3dToVector(const Point3d& _p) {
  vector<double> data(m_newPos.DOF(), 0.);
  copy(_p.begin(), _p.end(), data.begin());
  return data;
}
