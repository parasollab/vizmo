#ifdef USE_SPACEMOUSE

#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include "GUI/GLWidget.h"
#include "GUI/MainWindow.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"
#include "Utilities/Camera.h"

#include "SpaceMouseManager.h"
#include <spnav.h>

using namespace std;

bool spaceMouseDebug = false;

/*-------------------------- Space Mouse Manager -----------------------------*/

SpaceMouseManager::
SpaceMouseManager() : QObject(NULL), m_enable(false), m_enableCamera(false),
    m_pos(), m_rot(), m_thread(NULL), m_reader(NULL), m_lock() {
  // Set up thread and device reader. Move reader to thread
  m_thread = new QThread(this);
  m_reader = new SpaceMouseReader(this);
  m_reader->moveToThread(m_thread);
  connect(this, SIGNAL(run()), m_reader, SLOT(InputLoop()));
}


SpaceMouseManager::
~SpaceMouseManager() {
  // Disable camera control.
  if(m_enableCamera)
    this->DisableCamera();
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
  m_lock.lockForWrite();
  if(spaceMouseDebug)
    cout << "Starting spacemouse input thread...";
  if(!m_thread->isRunning()) {
    m_enable = true;
    m_thread->start();
    emit run();
  }
  if(spaceMouseDebug)
    cout << "done." << endl;
  m_lock.unlock();
}


void
SpaceMouseManager::
Disable() {
  m_lock.lockForWrite();
  if(spaceMouseDebug)
    cout << "Stopping spacemouse input thread...";
  if(m_thread->isRunning()) {
    m_enable = false;
    m_thread->quit();
  }
  if(spaceMouseDebug)
    cout << "done." << endl;
  m_lock.unlock();
}


void
SpaceMouseManager::
EnableCamera() {
  if(spaceMouseDebug)
    cout << "Enabling camera control." << endl;
  m_lock.lockForWrite();
  m_enableCamera = true;
  m_lock.unlock();
}


void
SpaceMouseManager::
DisableCamera() {
  if(spaceMouseDebug)
    cout << "Disabling camera control." << endl;
  m_lock.lockForWrite();
  m_enableCamera = false;
  m_lock.unlock();
}


const bool
SpaceMouseManager::
IsEnabled() {
  m_lock.lockForRead();
  bool status = m_enable;
  m_lock.unlock();
  return status;
}


const bool
SpaceMouseManager::
IsEnabledCamera() {
  m_lock.lockForRead();
  bool status = m_enableCamera;
  m_lock.unlock();
  return status;
}


const Vector3d
SpaceMouseManager::
GetPos() {
  m_lock.lockForRead();
  Vector3d pos = m_pos;
  m_lock.unlock();
  return pos;
}


const Vector3d
SpaceMouseManager::
GetRot() {
  m_lock.lockForRead();
  Vector3d rot = m_rot;
  m_lock.unlock();
  return rot;
}


void
SpaceMouseManager::
SetPos(const Vector3d& _pos) {
  m_lock.lockForWrite();
  m_pos = _pos;
  m_lock.unlock();
}


void
SpaceMouseManager::
SetRot(const Vector3d& _rot) {
  m_lock.lockForWrite();
  m_rot = _rot;
  m_lock.unlock();
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
  m_speed = 200. / GetVizmo().GetEnv()->GetRadius();
  Camera* currentCamera = GetMainWindow()->GetGLWidget()->GetCurrentCamera();
  connect(this, SIGNAL(TranslateCamera(Vector3d)),
          currentCamera, SLOT(Translate(Vector3d)));
  connect(this, SIGNAL(RotateCamera(Vector3d, double)),
          currentCamera, SLOT(Rotate(Vector3d, double)));
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
  if(spaceMouseDebug)
    cout << "Entering spacemouse input loop." << endl;

  spnav_event inputEvent;
  Vector3d pos, rot;
  double rotMag;
  double scalingFactor = m_speed / 350.;

  // Continue while input is enabled.
  while(m_manager->IsEnabled()) {
    // Check for an event (non-blocking).
    if(spnav_poll_event(&inputEvent)) {
      // An event has occured.
      if(inputEvent.type == SPNAV_EVENT_MOTION) {
        // Handle motion events.
        if(spaceMouseDebug)
          cout << "Motion detected!" << endl;
        pos[0] = inputEvent.motion.x * scalingFactor;
        pos[1] = inputEvent.motion.y * scalingFactor;
        pos[2] = -inputEvent.motion.z * scalingFactor;
        rot[0] = inputEvent.motion.rx * scalingFactor;
        rot[1] = inputEvent.motion.ry * scalingFactor;
        rot[2] = -inputEvent.motion.rz * scalingFactor;
        if(m_manager->IsEnabledCamera()) {
          emit TranslateCamera(pos);
          rotMag = rot.norm() / 75.;
          if(rotMag) {
            rot.selfNormalize();
            emit RotateCamera(rot, rotMag);
          }
        }
        m_manager->SetPos(pos);
        m_manager->SetRot(rot);
      }
      else {
        // Handle button events.
        if(spaceMouseDebug)
          cout << "Button detected!" << endl;
      }
    }

    // Sleep to accept thread quit commands.
    usleep(1000);
    // Remove any events that have accumulated while sleeping.
    spnav_remove_events(SPNAV_EVENT_ANY);
  }
}

#endif
