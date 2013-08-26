#include "Camera.h"

#include <cmath>
#include <iostream>

#include <QMouseEvent>

#include "Models/Vizmo.h"

Camera::Camera(const string& _name, const Point3d& _pos, const Vector3d& _up)
  : m_camName(_name), m_cameraPos(_pos), m_up(_up),
  m_currentAzim(0), m_deltaAzim(0),
  m_currentElev(0), m_deltaElev(0),
  m_speed(0.5), m_angle(0.5),
  m_mousePressed(false) {
  }

void
Camera::Set(const Point3d& _pos, double _azim, double _elev) {
  m_cameraPos = _pos;
  m_currentAzim = _azim;
  m_currentElev = _elev;
}

void
Camera::Draw() {
  glTranslatef(m_cameraPos[0]+m_deltaDis[0],
      m_cameraPos[1]+m_deltaDis[1],
      -m_cameraPos[2]-m_deltaDis[2]);

  glRotated(m_currentElev+m_deltaElev, 1.0, 0.0, 0.0);
  glRotated(m_currentAzim+m_deltaAzim, 0.0, 1.0, 0.0);

  /*
     gluLookAt( (m_cameraPos[0]+m_deltaDis[0]), //eye pos
     -(m_cameraPos[1]+m_deltaDis[1]),
     -(m_cameraPos[2]+m_deltaDis[2]),
     0,0,0, //center
     m_up[0],m_up[1],m_up[2]);
     */
}

bool
Camera::MousePressed(QMouseEvent* _e) {
  if(_e->buttons() && _e->modifiers() == Qt::ControlModifier) {
    m_mousePressed = true;
    m_pressedPt = _e->pos();
    return true; //handled
  }
  return false;
}

bool
Camera::MouseReleased(QMouseEvent* _e) {
  if(m_mousePressed) {
    m_mousePressed = false;

    m_cameraPos += m_deltaDis;
    m_currentElev += m_deltaElev;
    m_currentAzim += m_deltaAzim;

    m_deltaDis(0, 0, 0);
    m_deltaElev = 0.0;
    m_deltaAzim = 0.0;

    return true;
  }
  return false;
}

bool
Camera::MouseMotion(QMouseEvent* _e){
  if(m_mousePressed) {
    QPoint drag = _e->pos() - m_pressedPt;

    //displacement
    if(_e->buttons() & Qt::MidButton) {
      m_deltaDis[0] = drag.x()/10.0;
      m_deltaDis[1] = -drag.y()/10.0;
    }
    else if(_e->buttons() & Qt::RightButton) {
      m_deltaDis[2] = drag.y()/10.0;
    }
    else if(_e->buttons() & Qt::LeftButton) {
      m_deltaAzim = drag.x()/5.0;
      m_deltaElev = drag.y()/5.0;
    }
    else
      return false; //not handled

    return true;
  }
  return false; //mouse is not pressed
}

bool
Camera::KeyPressed(QKeyEvent* _e) {

  switch(_e->key()){
    //case 'h': PrintHelp();return true;
    case '-':
      m_speed /= 1.5;
      return true;
    case '+':
      m_speed *= 1.5;
      return true;
    case '/':
      m_angle /= 1.5;
      return true;
    case '*':
      m_angle *= 1.5;
      return true;
    case '6': //left
      m_cameraPos[0] += m_speed;
      return true;
    case '4': //right
      m_cameraPos[0] -= m_speed;
      return true;
    case '7': //up
      m_cameraPos[1] += m_speed;
      return true;
    case '1': //down
      m_cameraPos[1] -= m_speed;
      return true;
    case '8': //forward
      m_cameraPos[2] -= m_speed;
      return true;
    case '2': //back
      m_cameraPos[2] += m_speed;
      return true;
    case Qt::Key_Left: //look left
      m_currentAzim -= m_angle;
      return true;
    case Qt::Key_Right: //look right
      m_currentAzim += m_angle;
      return true;
    case Qt::Key_Up: //look up
      m_currentElev -= m_angle;
      return true;
    case Qt::Key_Down: //look down
      m_currentElev += m_angle;
      return true;
    default:
      return false;
  }
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

//Global variables and access
CameraFactory g_cameraFactory;

CameraFactory&
GetCameraFactory() {
  return g_cameraFactory;
}

