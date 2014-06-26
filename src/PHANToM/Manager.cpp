#ifdef USE_PHANTOM

#include "Manager.h"

#include <iostream>
#include <cstdlib>

#include "Callbacks.h"
#include "DeviceState.h"

namespace PHANToM {

//static HHLRC ghHLRC = NULL;

Manager& GetManager(){
  static Manager manager;
  return manager;
}

Manager::
Manager() : m_hhd(HD_INVALID_HANDLE), m_schedulerCallback(HD_INVALID_HANDLE) {
  CDOn = false;
  proceed = false;
  phantomforce = .5;
  Collision = -1;
  UseFeedback = true;
  Initialize();
}

Manager::
~Manager() {
  Clean();
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

    // Create a haptic context for the device.  The haptic context maintains
    // the state that persists between frame intervals and is used for
    // haptic rendering.
    //ghHLRC = hlCreateContext(m_hhd);
    //hlMakeCurrent(ghHLRC);
    hdEnable(HD_FORCE_OUTPUT);
    hdEnable(HD_MAX_FORCE_CLAMPING);

    hdStartScheduler();

    // Get the workspace dimensions.
    HDdouble maxWorkspace[6];
    hdGetDoublev(HD_MAX_WORKSPACE_DIMENSIONS, maxWorkspace);
    // Low/left/back point of device workspace.
    hduVector3Dd llb(maxWorkspace[0], maxWorkspace[1], maxWorkspace[2]);
    // Top/right/front point of device workspace.
    hduVector3Dd trf(maxWorkspace[3], maxWorkspace[4], maxWorkspace[5]);

    m_schedulerCallback = hdScheduleAsynchronous(ForceFeedback, NULL, HD_DEFAULT_SCHEDULER_PRIORITY);
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
  if(m_schedulerCallback != HD_INVALID_HANDLE) {
    hdStopScheduler();
    hdUnschedule(m_schedulerCallback);
    m_schedulerCallback = HD_INVALID_HANDLE;
  }

  // Free the haptics device
  if(m_hhd != HD_INVALID_HANDLE) {
    hdDisableDevice(m_hhd);
    m_hhd = HD_INVALID_HANDLE;
  }
}

hduVector3Dd
Manager::
GetPosition() {
  DeviceState state;
  hdScheduleSynchronous(GetDeviceState, &state, HD_MIN_SCHEDULER_PRIORITY);
  return state.m_position;
}

hduVector3Dd
Manager::
GetRotation() {
  DeviceState state;
  hdScheduleSynchronous(GetDeviceState, &state, HD_MIN_SCHEDULER_PRIORITY);
  return state.m_rotation;
}


hduVector3Dd
Manager::
GetVelocity() {
  DeviceState state;
  hdScheduleSynchronous(GetDeviceState, &state, HD_MIN_SCHEDULER_PRIORITY);
  return state.m_velocity;
}

}
///////////////////////////////////////////////////////////////////////////
#endif
