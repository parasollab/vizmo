#ifdef USE_PHANTOM

#include "Manager.h"

#include <iostream>
#include <cstdlib>

#include <HDU/hduError.h>

namespace PHANToM {

Manager& GetManager(){
  static Manager manager;
  return manager;
}

Manager::
Manager() : m_hhd(HD_INVALID_HANDLE), m_hhlrc(NULL) {
  Initialize();
}

Manager::
~Manager() {
  Clean();
}

void
Manager::Frame() {

  if(m_hhd == HD_INVALID_HANDLE)
    return;

  hlBeginFrame();

  hdGetDoublev(HD_CURRENT_POSITION, m_pos);
  hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, m_rot);
  hdGetDoublev(HD_CURRENT_VELOCITY, m_vel);
  hdGetDoublev(HD_CURRENT_FORCE, m_force);

  cout << endl << "Haptic State" << endl;
  cout << "\tPos: " << m_pos << endl;
  cout << "\tRot: " << m_rot << endl;
  cout << "\tVel: " << m_vel << endl;
  cout << "\tForce: " << m_force << endl;

  hduVector3Dd pos = m_pos;

  hduVector3Dd force(0, 0, 0);

  double r = 100;
  double dist = pos.magnitude();
  if(dist > r) {
    double penDist = r-dist;

    force = pos/dist;

    double k = 0.25;

    force = k * penDist * force;
  }

  //hdSetDoublev(HD_CURRENT_FORCE, force);

  hlEndFrame();

  hlGetDoublev(HL_DEVICE_POSITION, m_worldPos);

  cout << endl << "World State" << endl;
  cout << "\tPos: " << m_worldPos << endl;
}

void
Manager::
Initialize() {

  HDErrorInfo error;

  try {
    cout << "Initializing Phantom" << endl;

    // Initialize the device.  This needs to be called before any other
    // actions on the device are performed.
    m_hhd = hdInitDevice(HD_DEFAULT_DEVICE);

    cout << "Found device " << hdGetString(HD_DEVICE_MODEL_TYPE) << endl;

    // Get the workspace dimensions.
    HDdouble maxWorkspace[6];
    hdGetDoublev(HD_MAX_WORKSPACE_DIMENSIONS, maxWorkspace);
    // Low/left/back point of device workspace.
    hduVector3Dd llb(maxWorkspace[0], maxWorkspace[1], maxWorkspace[2]);
    // Top/right/front point of device workspace.
    hduVector3Dd trf(maxWorkspace[3], maxWorkspace[4], maxWorkspace[5]);

    cout << "BBX1: " << llb << "\t" << trf << endl;

    //create hl context
    m_hhlrc = hlCreateContext(m_hhd);
    hlMakeCurrent(m_hhlrc);

    //set limits of haptic workspace in millimeters.
    //hlWorkspace(-150, -45, -90, 150, 450, 50);

    // Force mapping to TestRigid bounds
    /*hlDisable(HL_USE_GL_MODELVIEW);
      hlMatrixMode(HL_TOUCHWORKSPACE);
      hlLoadIdentity();

      hlOrtho(-25, 25, -15, 15, -25, 25);
      */
  }
  catch(...) {
    if(HD_DEVICE_ERROR(error = hdGetError()))
      hduPrintError(stderr, &error, "Failed to initialize PHANToM");
    else
      cerr << "Unknown error." << endl;
    Clean();
    exit(1);
  }
}

void
Manager::
Clean() {
  // Free the haptics device
  if(m_hhd != HD_INVALID_HANDLE) {
    hlMakeCurrent(NULL);
    hlDeleteContext(m_hhlrc);
    hdDisableDevice(m_hhd);
    m_hhlrc = NULL;
    m_hhd = HD_INVALID_HANDLE;
  }
}

}

#endif
