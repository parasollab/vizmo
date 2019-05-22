#ifdef USE_SPACEMOUSE

#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include "GUI/GLWidget.h"
#include "GUI/MainWindow.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"
#include "Utilities/Camera.h"
#include "Utilities/Cursor3d.h"

#include "SpaceMouseManager.h"
#include <spnav.h>

using namespace std;


/*-------------------------- Space Mouse Manager -----------------------------*/

SpaceMouseManager::
SpaceMouseManager() : QObject(GetMainWindow()) {
  // Set up thread and device reader. Move reader to thread
  m_thread = new QThread(this);
  m_reader = new SpaceMouseReader(this);
  m_reader->moveToThread(m_thread);
  connect(this, SIGNAL(run()), m_reader, SLOT(InputLoop()));
  if(GetVizmo().GetEnv())
    SetSpeed();
}


SpaceMouseManager::
~SpaceMouseManager() {
  // Disable camera control.
  if(m_enableCamera)
    this->DisableCamera();
  // Disable cursor control.
  if(m_enableCursor)
    this->DisableCursor();
  // Disable input collection.
  if(m_enable)
    this->Disable();
  // Quit the input loop, forcibly if necessary.
  m_thread->quit();
  if(!m_thread->wait(100)) {
    m_thread->terminate();
    m_thread->wait(100);
  }
  // Delete dynamically allocated objects.
  delete m_reader;
  delete m_thread;
}


void
SpaceMouseManager::
Enable() {
  if(m_debug)
    cout << "Starting spacemouse input thread...";
  m_lock.lockForWrite();
  if(!m_thread->isRunning()) {
    m_enable = true;
    m_thread->start();
    emit run();
  }
  m_lock.unlock();
  if(m_debug)
    cout << "done." << endl;
}


void
SpaceMouseManager::
Disable() {
  m_lock.lockForWrite();
  if(m_debug)
    cout << "Stopping spacemouse input thread...";
  if(m_thread->isRunning()) {
    m_enable = false;
    m_thread->quit();
  }
  if(m_debug)
    cout << "done." << endl;
  m_lock.unlock();
}


void
SpaceMouseManager::
EnableCamera() {
  if(!m_enable || m_enableCamera)
    return;
  if(m_enableCursor)
    DisableCursor();
  if(m_debug)
    cout << "Enabling camera control." << endl;
  m_enableCamera = true;
  Camera* currentCamera = GetMainWindow()->GetGLWidget()->GetCurrentCamera();
  connect(this, SIGNAL(TranslateEvent(Vector3d)),
          currentCamera, SLOT(Translate(Vector3d)));
  connect(this, SIGNAL(RotateEvent(Vector3d, double)),
          currentCamera, SLOT(Rotate(Vector3d, double)));
}


void
SpaceMouseManager::
DisableCamera() {
  if(!m_enable || !m_enableCamera)
    return;
  if(m_debug)
    cout << "Disabling camera control." << endl;
  m_enableCamera = false;
  Camera* currentCamera = GetMainWindow()->GetGLWidget()->GetCurrentCamera();
  disconnect(this, SIGNAL(TranslateEvent(Vector3d)),
             currentCamera, SLOT(Translate(Vector3d)));
  disconnect(this, SIGNAL(RotateEvent(Vector3d, double)),
             currentCamera, SLOT(Rotate(Vector3d, double)));
}


void
SpaceMouseManager::
EnableCursor() {
  if(!m_enable || m_enableCursor)
    return;
  if(m_enableCamera)
    DisableCamera();
  if(m_debug)
    cout << "Enabling cursor control." << endl;
  m_enableCursor = true;
  Cursor3d* cursor = GetMainWindow()->GetGLWidget()->GetCursor();
  connect(this, SIGNAL(TranslateEvent(Vector3d)),
          cursor, SLOT(Translate(Vector3d)));
  connect(this, SIGNAL(ButtonEvent(int, bool)),
          cursor, SLOT(ButtonHandler(int, bool)));
}


void
SpaceMouseManager::
DisableCursor() {
  if(!m_enable || !m_enableCursor)
    return;
  if(m_debug)
    cout << "Disabling cursor control." << endl;
  m_enableCursor = false;
  Cursor3d* cursor = GetMainWindow()->GetGLWidget()->GetCursor();
  disconnect(this, SIGNAL(TranslateEvent(Vector3d)),
             cursor, SLOT(Translate(Vector3d)));
  disconnect(this, SIGNAL(ButtonEvent(int, bool)),
             cursor, SLOT(ButtonHandler(int, bool)));
}


void
SpaceMouseManager::
SetSpeed() {
  m_speed = .02 * GetVizmo().GetEnv()->GetRadius() / m_pollRate;
}


bool
SpaceMouseManager::
IsEnabled() {
  m_lock.lockForRead();
  bool status = m_enable;
  m_lock.unlock();
  return move(status);
}


void
SpaceMouseManager::
TranslateHandler(const Vector3d& _v) {
  emit TranslateEvent(_v * m_speed);
}


void
SpaceMouseManager::
RotateHandler(const Vector3d& _v) {
  static const double scalingFactor = 8 * PI * m_pollRate;
  double rotMag = _v.norm() / scalingFactor;
  if(rotMag)
    emit RotateEvent(_v.normalize(), rotMag);
}

/*-------------------------- Input Gathering Worker --------------------------*/

SpaceMouseReader::
SpaceMouseReader(SpaceMouseManager* _m) : QObject(), m_manager(_m) {
  // Open a connection to the device driver daemon.
  if(spnav_open() == -1) {
    cerr << "Error: SpaceMouseManager could not connect to the spacenavd daemon."
         << endl;
    exit(0);
  }
  connect(this, SIGNAL(TranslateInput(Vector3d)),
          _m, SLOT(TranslateHandler(Vector3d)));
  connect(this, SIGNAL(RotateInput(Vector3d)),
          _m, SLOT(RotateHandler(Vector3d)));
  connect(this, SIGNAL(ButtonInput(int, bool)),
          _m, SIGNAL(ButtonEvent(int, bool)));
}


SpaceMouseReader::
~SpaceMouseReader() {
  // Remove pending input events. Failing to do so will crash the device driver.
  spnav_remove_events(SPNAV_EVENT_ANY);
  // Close connection to the device driver.
  spnav_close();
}


void
SpaceMouseReader::
InputLoop() {
  if(m_manager->m_debug)
    cout << "Entering spacemouse input loop." << endl;

  spnav_event inputEvent;
  Vector3d pos, rot;

  while(m_manager->IsEnabled()) {
    // Check for an event (non-blocking).
    if(spnav_poll_event(&inputEvent)) {

      // Handle motion events.
      if(inputEvent.type == SPNAV_EVENT_MOTION) {
        pos[0] =  inputEvent.motion.x;
        pos[1] =  inputEvent.motion.y;
        pos[2] = -inputEvent.motion.z;
        emit TranslateInput(pos);
        rot[0] =  inputEvent.motion.rx;
        rot[1] =  inputEvent.motion.ry;
        rot[2] = -inputEvent.motion.rz;
        emit RotateInput(rot);
        if(m_manager->m_debug)
          cout << "Motion detected!" << endl
               << "\tTranslation: " << pos << endl
               << "\tRotation: " << rot << endl;
      }
      // Handle button events.
      else if(inputEvent.type == SPNAV_EVENT_BUTTON) {
        emit ButtonInput(inputEvent.button.bnum, (bool)inputEvent.button.press);
        if(m_manager->m_debug)
          cout << "Button " << inputEvent.button.bnum << " was "
               << (inputEvent.button.press ? "pressed" : "released")
               << "!" << endl;
      }
    }

    // Sleep to accept thread quit commands.
    usleep(m_manager->m_pollRate);
    // Remove any events that have accumulated while sleeping.
    spnav_remove_events(SPNAV_EVENT_ANY);
  }
}

#endif
