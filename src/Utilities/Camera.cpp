#include "Camera.h"

#include <GL/glu.h>

#include <QMouseEvent>

#include "Models/EnvModel.h"
#include "Models/Vizmo.h"
#include "PHANToM/Manager.h"


Camera::
Camera(const Point3d& _eye, const Vector3d& _at, const Vector3d& _up) :
    QObject(NULL),
    m_eye(_eye),
    m_dir((_at - _eye).normalize()),
    m_up(_up),
    m_speed(0.4),
    m_mousePressed(false),
    m_freeFloating(false) {
}


void
Camera::
Set(const Vector3d& _eye, const Vector3d& _at, const Vector3d& _up) {
  /// Also adjusts the camera speed relative to the current EnvModel.
  m_eye = _eye;
  m_dir = (_at - _eye).normalize();
  m_up  = _up.normalize();

  EnvModel* e = GetVizmo().GetEnv();
  m_speed = 1./e->GetRadius();
}


void
Camera::
Draw() {
  /// If haptics are in use, also informs the haptics manager that the viewing
  /// perspective has changed.
  Vector3d c = m_eye + m_dir;
  gluLookAt(
      m_eye[0], m_eye[1], m_eye[2],
      c[0], c[1], c[2],
      m_up[0], m_up[1], m_up[2]
      );

  if(Haptics::UsingPhantom())
    GetVizmo().GetPhantomManager()->UpdateWorkspace();
}


bool
Camera::
MousePressed(QMouseEvent* _e) {
  if(_e->buttons()) {
    if(_e->modifiers() & Qt::ControlModifier) {
      m_mousePressed = true;
      m_pressedPt = _e->pos();
      return true; //handled
    }
  }
  return false;
}


bool
Camera::
MouseReleased(QMouseEvent* _e) {
  if(m_mousePressed) {
    m_mousePressed = false;
    return true;
  }
  return false;
}


bool
Camera::
MouseMotion(QMouseEvent* _e) {
  /// The current controls require mouse click-and-drag with a keyboard modifier
  /// held down. The available controls are:
  /// \arg <i>zoom</i>: <tt>ctrl + right-click</tt>
  /// \arg <i>pan</i>: <tt>ctrl + middle-click</tt>
  /// \arg <i>look</i>: <tt>ctrl + shift + left-click</tt>
  /// \arg <i>rotate about origin</i>: <tt>ctrl + left-click</tt>
  if(m_mousePressed) {
    double trSpeed = (m_eye - Point3d()).norm() / 200.0;
    double rotSpeed = 0.5;
    QPoint drag = _e->pos() - m_pressedPt;
    double dx = drag.x();
    double dy = -drag.y();
    const Vector3d xHat = Vector3d(1, 0, 0);
    const Vector3d yHat = Vector3d(0, 1, 0);
    const Vector3d zHat = Vector3d(0, 0, 1);

    //ctrl+shift+left
    //  Y - rotates at vector up/down
    //  X - rotates at vector right/left
    if(_e->modifiers() & Qt::ShiftModifier && _e->buttons() == Qt::LeftButton) {
      //rotate about x-axis by theta
      double xRot = degToRad( dy * rotSpeed);
      Rotate(xHat, xRot);
      //rotate about y-axis by phi
      double yRot = degToRad(-dx * rotSpeed);
      Rotate(yHat, yRot);
    }

    //ctrl+shift+right/middle
    //  not handled by camera
    else if(_e->modifiers() & Qt::ShiftModifier)
      return false;

    //ctrl+left
    //  Y - changes elevation of camera
    //  X - changes azimuth of camera
    else if(_e->buttons() == Qt::LeftButton) {
      // If camera is not free-floating, use old azimuthal controls.
      if(!m_freeFloating) {
        //rotate about y-axis by phi and
        double xRot = degToRad( dy * rotSpeed);
        if(abs(xRot) > .001) {
          m_eye.rotate(GetWindowX(), xRot);
          m_dir.rotate(GetWindowX(), xRot);
        }
        double yRot = degToRad(-dx * rotSpeed);
        if(abs(yRot) > .001) {
          m_eye.rotate(m_up, yRot);
          m_dir.rotateY(yRot).selfNormalize();
        }
      }
      // Otherwise, control camera roll with this control.
      else {
        double zRot = degToRad(dx * rotSpeed);
        m_up.rotate(m_dir, zRot).selfNormalize();
      }
    }

    //ctrl+right
    //  Y - moves camera in/out of at direction
    else if(_e->buttons() == Qt::RightButton)
      Translate(-zHat * dy * trSpeed);

    //ctrl+middle
    //  Y - moves camera up/down of at direction
    //  X - moves camera right/left of at direction
    else if(_e->buttons() == Qt::MidButton)
      Translate(xHat * dx * trSpeed + yHat * dy * trSpeed);

    //not handled by camera
    else
      return false;

    //handled by camera successfully
    m_pressedPt = _e->pos();
    return true;
  }
  //mouse not pressed, not handled by camera
  return false;
}


bool
Camera::
KeyPressed(QKeyEvent* _e) {
  /// The available controls are:
  /// \arg <i>zoom in/out</i>: <tt>W/S</tt>
  /// \arg <i>pan left/right</i>: <tt>A/D</tt>
  /// \arg <i>pan up/down</i>: <tt>Q/E</tt>
  /// \arg <i>look</i>: <tt>arrow keys</tt>

  const Vector3d xHat(1, 0, 0);
  const Vector3d yHat(0, 1, 0);
  const Vector3d zHat(0, 0, 1);

  switch(_e->key()) {
    case '-':
      // Half camera speed.
      m_speed *= 0.5;
      return true;
    case '+':
      // Double camera speed.
      m_speed *= 2;
      return true;
    case 'A':
      // Move left.
      Translate(-xHat * 10 * m_speed);
      return true;
    case 'D':
      // Move right.
      Translate(xHat * 10 * m_speed);
      return true;
    case 'W':
      // Move in.
      Translate(-zHat * 10 * m_speed);
      return true;
    case 'S':
      // Move out.
      Translate(zHat * 10 * m_speed);
      return true;
    case 'Q':
      // Move up.
      Translate(yHat * 10 * m_speed);
      return true;
    case 'E':
      // Move down.
      Translate(-yHat * 10 * m_speed);
      return true;
    case Qt::Key_Left:
      // Look left.
      Rotate(yHat, degToRad(10 * m_speed));
      return true;
    case Qt::Key_Right:
      // Look right.
      Rotate(yHat, degToRad(10 * -m_speed));
      return true;
    case Qt::Key_Up:
      // Look up.
      Rotate(xHat, degToRad(10 * m_speed));
      return true;
    case Qt::Key_Down:
      // Look down.
      Rotate(xHat, degToRad(10 * -m_speed));
      return true;
    default:
      return false;
  }
}


Vector3d
Camera::
GetWindowX() const {
  return (m_dir % m_up).normalize();
}


Vector3d
Camera::
GetWindowY() const {
  return (m_dir % m_up % m_dir).normalize();
}


Vector3d
Camera::
GetWindowZ() const {
  return -m_dir;
}


void
Camera::
ToggleFreeFloat() {
  if(m_freeFloating)
    m_freeFloating = false;
  else {
    m_freeFloating = true;
    m_up = GetWindowY();
  }
}


void
Camera::
ResetUp() {
  if(m_freeFloating) {
    Vector3d xHat = GetWindowX();
    m_up = Vector3d(0, 1, 0);
    m_dir = (m_up % xHat).normalize();
  }
  else
    m_up = Vector3d(0, 1, 0);
}


/*---------------------------- Cross-thread Controls -------------------------*/

void
Camera::
Translate(const Vector3d& _delta) {
  // Transform _delta to world coordinates.
  Vector3d worldDelta = _delta[0] * GetWindowX() +
                        _delta[1] * GetWindowY() +
                        _delta[2] * GetWindowZ();

  // Apply translation.
  m_eye += worldDelta;
}


void
Camera::
Rotate(const Vector3d& _axis, double _theta) {
  // Ignore rotations that are too small to reduce camera shake.
  if(abs(_theta) < .001)
    return;

  // Transform _axis to world coordinates.
  Vector3d worldAxis = _axis[0] * GetWindowX() +
                       _axis[1] * GetWindowY() +
                       _axis[2] * GetWindowZ();

  // Apply rotation.
  m_dir.rotate(worldAxis, _theta).selfNormalize();

  // If free-floating, rotate the up direction as well.
  if(m_freeFloating)
    m_up.rotate(worldAxis, _theta).selfNormalize();
}
