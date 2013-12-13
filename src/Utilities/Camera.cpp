#include "Camera.h"

#include <cmath>
#include <iostream>

#include <QMouseEvent>
#include <GL/glu.h>

#include "Models/Vizmo.h"
#include "Models/EnvModel.h"

Camera::Camera(const string& _name, const Point3d& _pos, const Vector3d& _up)
  : m_camName(_name),  m_up(_up),
  m_eye(Vector3d(0,0,0)), m_at(Vector3d(0,0,0)),
  m_vector(Vector3d(0,0,0)),
  m_drag(QPoint(0,0)),
  m_currentAzim(0), m_deltaAzim(0),
  m_currentElev(0), m_deltaElev(0),
  m_speed(0.4), m_defaultSpeed(0),
  m_mousePressed(false) {
  }

void
Camera::Set(const Vector3d& _eye, const Vector3d& _at, const Vector3d& _up) {
  m_eye = _eye;
  m_at = _at ;
  m_up= _up;
  m_vector=m_eye-m_at;
  if(m_up[1]>=0)
    m_currentElev=radToDeg(acos(-m_vector[1]/ m_vector.norm())-M_PI/2);
  else
    m_currentElev=radToDeg(2*M_PI-acos(-m_vector[1]/ m_vector.norm())-M_PI/2);
  if(m_vector[2]>=0)
    m_currentAzim=radToDeg(acos(-m_vector[0]/sqrt(sqr(m_vector[0])+ sqr(m_vector[2])))-PI/2)+180*(m_up[1]<0);
  else
    m_currentAzim=radToDeg(-acos(-m_vector[0]/sqrt(sqr(m_vector[0])+ sqr(m_vector[2])))-PI/2)+180*(m_up[1]<0);
  EnvModel* e = GetVizmo().GetEnv();
  m_defaultSpeed=2000/e->GetRadius();
}

void
Camera::Draw() {
  gluLookAt( m_eye[0], m_eye[1], m_eye[2],
  m_at[0],m_at[1],m_at[2],
  m_up[0],m_up[1],m_up[2]);
}

bool
Camera::MousePressed(QMouseEvent* _e) {
  if(_e->buttons()) {
    if(_e->modifiers() & Qt::ControlModifier){
      m_mousePressed = true;
      m_pressedPt = _e->pos();
      m_drag=QPoint(0,0);
      return true; //handled
    }
  }
  return false;
}

bool
Camera::MouseReleased(QMouseEvent* _e) {
  if(m_mousePressed) {
    m_mousePressed = false;

    m_currentElev += m_deltaElev;
    m_currentAzim += m_deltaAzim;
    m_deltaDis(0, 0, 0);
    m_deltaElev = 0.0;
    m_deltaAzim = 0.0;
    m_drag=QPoint(0,0);

    return true;
  }
  return false;
}

bool
Camera::MouseMotion(QMouseEvent* _e){
  if(m_mousePressed) {
    if((_e->modifiers() & Qt::ShiftModifier)&&(_e->buttons() & Qt::LeftButton)) {
      /*******Y-AXIS:ROTATE UP/DOWN**X/Z-AXIS:ROTATE RIGHT-LEFT**********/
      m_drag = _e->pos() - m_pressedPt;
      m_deltaAzim = -m_drag.x()/5*m_speed;
      m_deltaElev = m_drag.y()/5*m_speed;
      double radAzim=degToRad(m_currentAzim+m_deltaAzim-90);
      double radElev=degToRad(m_currentElev+m_deltaElev+90);
      if(sin(radElev)>=0)
        m_up[1]=1;
      else
        m_up[1]=-1;
      m_eye[0]= sin(radElev)*m_vector.norm()*cos(radAzim);
      m_eye[1]= -cos(radElev)*m_vector.norm();
      m_eye[2]= -sin(radElev)*m_vector.norm()*sin(radAzim);
      m_eye+=m_at;
    }
    else if(_e->modifiers() & Qt::ShiftModifier){}//DO NOTHING
    else if(_e->buttons() & Qt::RightButton) {
      /*******Y-AXIS:GO UP/DOWN**X/Z-AXIS:GO RIGHT-LEFT**********/
      if(((m_drag.x()<0)&&(m_drag.x()<(_e->pos().x()-m_pressedPt.x())))||
          ((m_drag.x()>0)&&(m_drag.x()>(_e->pos().x()-m_pressedPt.x()))))
        m_pressedPt.setX( _e->pos().x());
      else if(((m_drag.y()<0)&&(m_drag.y()<(_e->pos().y()-m_pressedPt.y())))||
          ((m_drag.y()>0)&&(m_drag.y()>(_e->pos().y()-m_pressedPt.y()))))
        m_pressedPt.setY( _e->pos().y());
      m_drag = _e->pos() - m_pressedPt;
      m_deltaDis[0] = m_drag.x()/m_defaultSpeed*m_speed;
      m_deltaDis[1] = m_drag.y()/m_defaultSpeed*m_speed;
      double radAzim=degToRad(m_currentAzim);

      m_eye[0] = m_eye[0]       + cos(radAzim)*m_deltaDis[0];
      m_at[0] = m_at[0] + cos(radAzim)*m_deltaDis[0];
      m_eye[1]=m_eye[1]-m_up[1]*m_deltaDis[1];
      m_at[1]=m_at[1]-m_up[1]*m_deltaDis[1];
      m_eye[2] = m_eye[2]       - sin(radAzim)*m_deltaDis[0];
      m_at[2] = m_at[2] - sin(radAzim)*m_deltaDis[0];

    }
    else if(_e->buttons() & Qt::LeftButton) {
      /******Y/Z-AXIS:LOOK UP/DOWN**X/Z-AXIS:LOOK RIGHT-LEFT*****/
      m_drag = _e->pos() - m_pressedPt;
      m_deltaAzim = -m_drag.x()/5*m_speed;
      m_deltaElev = m_drag.y()/5*m_speed;
      double radAzim=degToRad(m_currentAzim+m_deltaAzim-90);
      double radElev=degToRad(m_currentElev+m_deltaElev+90);
      if(sin(radElev)>=0)
        m_up[1]=1;
      else
        m_up[1]=-1;
      m_at[0]= -sin(radElev)*m_vector.norm()*cos(radAzim);
      m_at[1]= cos(radElev)*m_vector.norm();
      m_at[2]= sin(radElev)*m_vector.norm()*sin(radAzim);
      m_at+=m_eye;
    }
    else if(_e->buttons()&Qt::MidButton){
      /*****************X/Z-AXIS:GO FORWARD/BACKWARD*************/
      if(((m_drag.x()<0)&&(m_drag.x()<(_e->pos().x()-m_pressedPt.x())))||
          ((m_drag.x()>0)&&(m_drag.x()>(_e->pos().x()-m_pressedPt.x()))))
        m_pressedPt.setX( _e->pos().x());
      else if(((m_drag.y()<0)&&(m_drag.y()<(_e->pos().y()-m_pressedPt.y())))||
          ((m_drag.y()>0)&&(m_drag.y()>(_e->pos().y()-m_pressedPt.y()))))
        m_pressedPt.setY( _e->pos().y());
      m_drag = _e->pos() - m_pressedPt;
      m_deltaDis[0] = m_drag.x()/m_defaultSpeed*m_speed;
      m_deltaDis[2] = m_drag.y()/m_defaultSpeed*m_speed;
      double radAzim=degToRad(m_currentAzim);
      m_eye[0] = m_eye[0]       +m_up[1]*sin(radAzim)*m_deltaDis[2]+ cos(radAzim)*m_deltaDis[0];
      m_at[0] = m_at[0] +m_up[1]*sin(radAzim)*m_deltaDis[2]+ cos(radAzim)*m_deltaDis[0];
      m_eye[2] = m_eye[2]       +m_up[1]*cos(radAzim)*m_deltaDis[2]- sin(radAzim)*m_deltaDis[0];
      m_at[2] = m_at[2] +m_up[1]*cos(radAzim)*m_deltaDis[2]- sin(radAzim)*m_deltaDis[0];
    }
    else
      return false; //not handled

    m_vector=m_eye-m_at;
    return true;
  }
  return false; //mouse is not pressed
}

bool
Camera::KeyPressed(QKeyEvent* _e) {

  switch(_e->key()){
    //case 'h': PrintHelp();return true;
    case '-':
        m_speed -= 0.1;
      if(m_speed<=0.0)
        m_speed=0.1;
      return true;
    case '+':
      m_speed += 0.1;
      return true;
    case Qt::Key_Left: //look left
      m_deltaAzim += 10;
      KeyRotatePressed();
      return true;
    case Qt::Key_Right: //look right
      m_deltaAzim -= 10;
      KeyRotatePressed();
      return true;
    case Qt::Key_Up: //look up
      if(_e->modifiers() & Qt::ControlModifier){
        if(_e->modifiers() & Qt::ShiftModifier){
          m_eye=m_eye*4/5;
          m_vector=m_eye-m_at;
        }
      }
      else{
        m_deltaElev += 10;
        KeyRotatePressed();
      }
      return true;
    case Qt::Key_Down: //look down
      if(_e->modifiers() & Qt::ControlModifier){
        if(_e->modifiers() & Qt::ShiftModifier){
          m_eye=m_eye*5/4;
          m_vector=m_eye-m_at;
        }
      }
      else{
        m_deltaElev -= 10;
        KeyRotatePressed();
      }
      return true;
    default:
      return false;
  }
}
void
Camera::KeyRotatePressed(){

  double radAzim=degToRad(m_currentAzim+m_deltaAzim-90);
  double radElev=degToRad(m_currentElev+m_deltaElev+90);
  if(sin(radElev)>=0)
    m_up[1]=1;
  else
    m_up[1]=-1;
  m_eye[0]= sin(radElev)*m_vector.norm()*cos(radAzim);
  m_eye[1]= -cos(radElev)*m_vector.norm();
  m_eye[2]= -sin(radElev)*m_vector.norm()*sin(radAzim);
  m_eye+=m_at;

  m_currentElev += m_deltaElev;
  m_currentAzim += m_deltaAzim;

  m_deltaDis(0, 0, 0);
  m_deltaElev = 0.0;
  m_deltaAzim = 0.0;

  m_vector=m_eye-m_at;
}

Vector3d
Camera::GetWindowX() const {
  Vector3d v(1, 0, 0);
  v.rotateX(m_currentElev).rotateY(m_currentAzim);
  v[2] = -v[2];
  return v;
}

Vector3d
Camera::GetWindowY() const {
  Vector3d v(0, 1, 0);
  v.rotateX(m_currentElev).rotateY(m_currentAzim);
  v[2] = -v[2];
  return v;
}

Vector3d
Camera::GetWindowZ() const {
  return GetWindowX() % GetWindowY();
}

CameraFactory::CameraFactory() {
  //create a default camera perspective
  AddCamera(Camera("default", Point3d(0, 0, 500), Vector3d(0, 1, 0)));
  m_currentCam = &m_cameras.begin()->second;
}

void
CameraFactory::AddCamera(const Camera& _camera) {
  if(m_cameras.find(_camera.GetCameraName()) == m_cameras.end())
    m_cameras.insert(make_pair(_camera.GetCameraName(), _camera));
  else
    cerr << "Warning::Camera '" << _camera.GetCameraName()
      << "' already exists in CameraFactory." << endl;
}

void
CameraFactory::SetCurrentCamera(const string& _name) {
  map<string, Camera>::iterator c = m_cameras.find(_name);
  if(c != m_cameras.end())
    m_currentCam = &c->second;

  cerr << "Warning::Camera '" << _name
    << "' does not exist in CameraFactory." << endl;
}

vector<Vector3d>
Camera::GetCameraPos() const{
  vector<Vector3d> pos;
  pos.push_back(m_eye);
  pos.push_back(m_at);
  pos.push_back(m_up);
  return pos;
}
