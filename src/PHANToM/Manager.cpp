#ifdef USE_PHANTOM

#include "Manager.h"

#include <iostream>
#include <cstdlib>

#include "Callbacks.h"
#include "DeviceState.h"

namespace PHANToM {

//static HHLRC ghHLRC = NULL;
static HHD ghHD = HD_INVALID_HANDLE;
static HDSchedulerHandle gSchedulerCallback = HD_INVALID_HANDLE;


Manager& GetManager(){
  static Manager manager;
  return manager;
}

void
  ExitHandler() {
    hdStopScheduler();
    hdUnschedule(gSchedulerCallback);

    // Free the haptics device
    if (ghHD != HD_INVALID_HANDLE) {
      hdDisableDevice(ghHD);
      ghHD = HD_INVALID_HANDLE;
    }
  }

Manager::
Manager() {
  CDOn = false;
  proceed = false;
  phantomforce = .5;
  Collision = -1;
  UseFeedback = true;
  InitPhantom();
}

Manager::
~Manager() {
  hdStopScheduler();
  hdUnschedule(gSchedulerCallback);

  // Free the haptics device
  if (ghHD != HD_INVALID_HANDLE) {
    hdDisableDevice(ghHD);
    ghHD = HD_INVALID_HANDLE;
  }
}

void
Manager::
InitPhantom() {

  HDErrorInfo error;

  printf("initializing Phantom\n");

  atexit(ExitHandler);

  // Initialize the device.  This needs to be called before any other
  // actions on the device are performed.
  ghHD = hdInitDevice(HD_DEFAULT_DEVICE);
  if(HD_DEVICE_ERROR(error = hdGetError())) {
    hduPrintError(stderr, &error, "Failed to initialize haptic device");
    fprintf(stderr, "\nPress any key to quit.\n");
    getchar();
    exit(-1);
  }

  printf("Found device %s\n", hdGetString(HD_DEVICE_MODEL_TYPE));

  // Create a haptic context for the device.  The haptic context maintains
  // the state that persists between frame intervals and is used for
  // haptic rendering.
  //ghHLRC = hlCreateContext(ghHD);
  //hlMakeCurrent(ghHLRC);
  hdEnable(HD_FORCE_OUTPUT);
  hdEnable(HD_MAX_FORCE_CLAMPING);

  hdStartScheduler();
  if(HD_DEVICE_ERROR(error = hdGetError())) {
    hduPrintError(stderr, &error, "Failed to start scheduler");
    fprintf(stderr, "\nPress any key to quit.\n");
    getchar();
    exit(-1);
  }

  // Get the workspace dimensions.
  HDdouble maxWorkspace[6];
  hdGetDoublev(HD_MAX_WORKSPACE_DIMENSIONS, maxWorkspace);
  // Low/left/back point of device workspace.
  hduVector3Dd llb(maxWorkspace[0], maxWorkspace[1], maxWorkspace[2]);
  // Top/right/front point of device workspace.
  hduVector3Dd trf(maxWorkspace[3], maxWorkspace[4], maxWorkspace[5]);

  ScheduleForceCallback();
}

void
Manager::
ScheduleForceCallback() {
  cout << "haptics callback" << endl;

  gSchedulerCallback = hdScheduleAsynchronous(ForceFeedback, NULL, HD_DEFAULT_SCHEDULER_PRIORITY);

  HDErrorInfo error;
  if(HD_DEVICE_ERROR(error = hdGetError())) {
    hduPrintError(stderr, &error, "Failed to initialize haptic device");
    fprintf(stderr, "\nPress any key to quit.\n");
    getchar();
    exit(-1);
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
