#include "Camera.h"

#include <GL/glu.h>

#include <QMouseEvent>

#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

Camera::Camera(const string& _name, const Point3d& _eye, const Vector3d& _at) :
  m_name(_name),
  m_up(0, 1, 0),
  m_eye(_eye), m_currEye(_eye),
  m_dir((_at-_eye).normalize()), m_currDir(m_dir),
  m_speed(0.4),
  m_mousePressed(false) {
  }

void
Camera::Set(const Vector3d& _eye, const Vector3d& _at) {
  m_eye = m_currEye = _eye;
  m_dir = m_currDir = (_at - _eye).normalize();

  EnvModel* e = GetVizmo().GetEnv();
  m_speed = 1./e->GetRadius();
}

void
Camera::Draw() {
  Vector3d c = m_currEye + m_currDir;
  gluLookAt(
      m_currEye[0], m_currEye[1], m_currEye[2],
      c[0], c[1], c[2],
      m_up[0], m_up[1], m_up[2]
      );
}

bool
Camera::MousePressed(QMouseEvent* _e) {
  if(_e->buttons()) {
    if(_e->modifiers() & Qt::ControlModifier){
      m_mousePressed = true;
      m_pressedPt = _e->pos();
      return true; //handled
    }
  }
  return false;
}

bool
Camera::MouseReleased(QMouseEvent* _e) {
  if(m_mousePressed) {
    m_mousePressed = false;
    m_eye = m_currEye;
    m_dir = m_currDir;
    return true;
  }
  return false;
}

bool
Camera::MouseMotion(QMouseEvent* _e){
  if(m_mousePressed) {
    double trSpeed = (m_eye - Point3d()).norm() / 200.0;
    double rotSpeed = 0.5;
    QPoint drag = _e->pos() - m_pressedPt;
    double dx = drag.x();
    double dy = -drag.y();

    //ctrl+shift+left
    //  Y - rotates at vector up/down
    //  X - rotates at vector right/left
    if(_e->modifiers() & Qt::ShiftModifier && _e->buttons() == Qt::LeftButton) {
      double phi = degToRad(-dx * rotSpeed);
      double theta = degToRad(dy * rotSpeed);
      double beta = acos(m_up * m_dir);

      Vector3d left = (m_dir % m_up).normalize();
      m_currDir = m_dir;

      //rotate at vector around left by theta
      if((theta > 0 && beta - theta > 0.001) || (theta < 0 && beta - theta < PI - 0.001)) {
        Rotate(m_currDir, left, theta);
        m_currDir.selfNormalize();
      }

      //rotate at vector around up by phi
      Rotate(m_currDir, m_up, phi);
      m_currDir.selfNormalize();
    }
    //ctrl+shift+right/middle
    //  not handled by camera
    else if(_e->modifiers() & Qt::ShiftModifier) {
      return false;
    }
    //ctrl+left
    //  Y - changes elevation of camera
    //  X - changes azimuth of camera
    else if(_e->buttons() == Qt::LeftButton) {
      double phi = degToRad(-dx * rotSpeed);
      double theta = degToRad(dy * rotSpeed);

      m_currEye = m_eye;
      m_currDir = m_dir;

      //rotate around Y-axis
      m_currEye.rotateY(phi);
      m_currDir.rotateY(phi);

      //rotate around left-axis
      Vector3d left = (m_currDir % m_up).normalize();
      Rotate(m_currEye, left, theta);
      Rotate(m_currDir, left, theta);
      m_currDir.selfNormalize();
    }
    //ctrl+right
    //  Y - moves camera in/out of at direction
    else if(_e->buttons() == Qt::RightButton) {
      m_currEye = m_eye + m_dir*dy * trSpeed;
    }
    //ctrl+middle
    //  Y - moves camera up/down of at direction
    //  X - moves camera right/left of at direction
    else if(_e->buttons() == Qt::MidButton) {
      Vector3d left = (m_dir % m_up).normalize();
      Vector3d trueUp = (left % m_dir).normalize();
      m_currEye = m_eye + left*dx*trSpeed + trueUp*dy*trSpeed;
    }
    //not handled by camera
    else
      return false;

    //handled by camera successfully
    return true;
  }
  //mouse not pressed, not handled by camera
  return false;
}

bool
Camera::KeyPressed(QKeyEvent* _e) {

  Vector3d left = (m_dir % m_up).normalize();

  switch(_e->key()){
    //case 'h': PrintHelp();return true;
    case '-':
      m_speed *= 0.5;
      return true;
    case '+':
      m_speed *= 2;
      return true;
    case 'A': //move right
      m_currEye = m_eye -= left * 10*m_speed;
      return true;
    case 'D': // move left
      m_currEye = m_eye += left * 10*m_speed;
      return true;
    case 'W': //move in
      m_currEye = m_eye += m_dir * 10*m_speed;
      return true;
    case 'S': // move out
      m_currEye = m_eye -= m_dir * 10*m_speed;
      return true;
    case 'Q': //move up
      m_currEye = m_eye += (left % m_dir).normalize() * 10*m_speed;
      return true;
    case 'E': // move down
      m_currEye = m_eye -= (left % m_dir).normalize() * 10*m_speed;
      return true;
    case Qt::Key_Left: //look left
      Rotate(m_dir, m_up, degToRad(10*m_speed));
      m_currDir = m_dir.selfNormalize();
      return true;
    case Qt::Key_Right: //look right
      Rotate(m_dir, m_up, degToRad(10*-m_speed));
      m_currDir = m_dir.selfNormalize();
      return true;
    case Qt::Key_Up: //look up
      if(acos(m_up * m_dir) - 10*m_speed > 0.001) {
        Rotate(m_dir, left, degToRad(10*m_speed));
        m_currDir = m_dir.selfNormalize();
      }
      return true;
    case Qt::Key_Down: //look down
      if(acos(m_up * m_dir) + 10*m_speed < PI - 0.001) {
        Rotate(m_dir, left, degToRad(10*-m_speed));
        m_currDir = m_dir.selfNormalize();
      }
      return true;
    default:
      return false;
  }
}

Vector3d
Camera::GetWindowX() const {
  return (m_up % m_currDir).normalize();
}

Vector3d
Camera::GetWindowY() const {
  return (m_currDir % m_up % m_currDir).normalize();
}

Vector3d
Camera::GetWindowZ() const {
  return m_currDir;
}

void
Camera::Rotate(Vector3d& _vec, const Vector3d& _axis, double _theta) {
  double u = _axis[0], v = _axis[1], w = _axis[2],
         x = _vec[0], y = _vec[1], z = _vec[2],
         c = cos(_theta), s = sin(_theta),
         dot1c = _axis*_vec*(1-c);

  _vec(
      u*dot1c + x*c + (-w*y + v*z)*s,
      v*dot1c + y*c + (w*x - u*z)*s,
      w*dot1c + z*c + (-v*x + u*y)*s
      );
}

CameraFactory::CameraFactory() {
  //create a default camera perspective
  AddCamera(Camera("default", Point3d(0, 0, 500), Vector3d(0, 0, 0)));
  m_currentCam = &m_cameras.begin()->second;
}

void
CameraFactory::AddCamera(const Camera& _camera) {
  if(m_cameras.find(_camera.GetName()) == m_cameras.end())
    m_cameras.insert(make_pair(_camera.GetName(), _camera));
  else
    cerr << "Warning::Camera '" << _camera.GetName()
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

