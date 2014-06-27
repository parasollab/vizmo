#ifdef USE_PHANTOM

#include "Manager.h"

#include <iostream>
#include <cstdlib>

#include <glut.h>

#include <HDU/hduError.h>
#include <HLU/hlu.h>

#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

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
  hlDeleteShapes(m_shapeId, 1);
}

void
Manager::HapticRender() {

  hlBeginFrame();

  hdGetDoublev(HD_CURRENT_POSITION, m_pos);
  hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, m_rot);
  hdGetDoublev(HD_CURRENT_VELOCITY, m_vel);
  hdGetDoublev(HD_CURRENT_FORCE, m_force);

  cout << endl << endl << "Haptic State" << endl;
  cout << "\tPos: " << m_pos << endl;
  cout << "\tRot: " << m_rot << endl;
  cout << "\tVel: " << m_vel << endl;
  cout << "\tForce: " << m_force << endl;

  hlGetDoublev(HL_DEVICE_POSITION, m_worldPos);

  cout << endl << "World State" << endl;
  cout << "\tPos: " << m_worldPos << endl;

  //create a shape for the boundary
  //hlHinti(HL_SHAPE_FEEDBACK_BUFFER_VERTICES, 8);

  hlMaterialf(HL_FRONT_AND_BACK, HL_STIFFNESS, 0.7f);
  hlMaterialf(HL_FRONT_AND_BACK, HL_DAMPING, 0.1f);

  hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, m_shapeId);

  //GetVizmo().GetEnv()->GetBoundary()->DrawRender();
  glutSolidSphere(15, 32, 32);

  hlEndShape();

  hlEndFrame();
}

void
Manager::DrawRender() {
  //draw object to touch
  glutSolidSphere(15, 32, 32);

  //draw sphere at device position
  glPushMatrix();
  glTranslatef(m_worldPos[0], m_worldPos[1], m_worldPos[2]);
  glColor3f(0.1, 0.4, 0.9);
  glutSolidSphere(1, 25, 25);
  glPopMatrix();
}

void
Manager::UpdateWorkspace() {
  //GLdouble modelview[16];
  GLdouble projection[16];
  //GLint viewport[4];

  //glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  //glGetIntegerv(GL_VIEWPORT, viewport);

  hlMatrixMode(HL_TOUCHWORKSPACE);
  hlLoadIdentity();

  // Fit haptic workspace to view volume.
  hluFitWorkspace(projection);
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

    hlEnable(HL_HAPTIC_CAMERA_VIEW);

    //set limits of haptic workspace in millimeters.
    hlWorkspace(-150, -45, -90, 150, 250, 50);

    // Force mapping to TestRigid bounds
    hlDisable(HL_USE_GL_MODELVIEW);
    //hlMatrixMode(HL_TOUCHWORKSPACE);
    //hlLoadIdentity();

    //hlOrtho(-25, 25, -25, 25, -25, 25);

    UpdateWorkspace();

    m_shapeId = hlGenShapes(1);

    hlTouchableFace(HL_FRONT);

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
