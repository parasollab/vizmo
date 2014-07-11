#ifdef USE_PHANTOM

#include "Manager.h"

#include <iostream>
#include <cstdlib>

#include <glut.h>

#include <HDU/hduError.h>
#include <HLU/hlu.h>

#include "Models/BodyModel.h"
#include "Models/EnvModel.h"
#include "Models/PolyhedronModel.h"
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
  hlDeleteShapes(m_boundaryId, 1);
}

void
Manager::HapticRender() {
  glDisable(GL_CULL_FACE);
  hlBeginFrame();

  //Get all state variables
  GetState();

  hlBeginFrame();

  //create a shape for the boundary
  BoundaryRender();

  //create shapes for obstacles
  ObstacleRender();

  hlEndFrame();
}

void
Manager::DrawRender() {
  //draw spherical cursor and crosshairs at device position
  glColor3f(0.1, 0.4, 0.9);

  //cursor
  glPushMatrix();
  glTranslatef(m_worldPos[0], m_worldPos[1], m_worldPos[2]);
  //glutSolidSphere(1, 25, 25);
  glPointSize(5);
  glBegin(GL_POINTS);
  //glVertex3f(m_worldPos[0], m_worldPos[1], m_worldPos[2]);
  glVertex3f(0, 0, 0);
  glEnd();

  //crosshair
  if(IsInsideBBX()) {
    glColor4f(.9, .9, .9, .2);
    glLineWidth(4);
    glBegin(GL_LINES);
      glVertex3f(m_worldRange[0].first  - m_worldPos[0], 0., 0.);
      glVertex3f(m_worldRange[0].second - m_worldPos[0], 0., 0.);
      glVertex3f(0., m_worldRange[1].first  - m_worldPos[1], 0.);
      glVertex3f(0., m_worldRange[1].second - m_worldPos[1], 0.);
      glVertex3f(0., 0., m_worldRange[2].first  - m_worldPos[2]);
      glVertex3f(0., 0., m_worldRange[2].second - m_worldPos[2]);
    glEnd();
  }
  glPopMatrix();
}

void
Manager::UpdateWorkspace() {
  GLdouble modelview[16];
  GLdouble projection[16];
  GLint viewport[4];

  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport);

  hlMatrixMode(HL_TOUCHWORKSPACE);
  hlLoadIdentity();

  //get min and max points from boundary
  double minP[3] = {m_worldRange[0].first, m_worldRange[1].first,
      m_worldRange[2].first};
  double maxP[3] = {m_worldRange[0].second, m_worldRange[1].second,
      m_worldRange[2].second};
  hluFitWorkspaceBox(modelview, minP, maxP);
}

void
Manager::
Initialize() {

  m_worldRange = GetVizmo().GetEnv()->GetBoundary()->GetRanges();
  HDErrorInfo error;

  try {
    cout << "Initializing Phantom" << endl;

    // Initialize the device.  This needs to be called before any other
    // actions on the device are performed.
    m_hhd = hdInitDevice(HD_DEFAULT_DEVICE);

    cout << "Found device " << hdGetString(HD_DEVICE_MODEL_TYPE) << endl;

    // Get the workspace dimensions.
    /*HDdouble maxWorkspace[6];
    hdGetDoublev(HD_MAX_WORKSPACE_DIMENSIONS, maxWorkspace);
    // Low/left/back point of device workspace.
    hduVector3Dd llb(maxWorkspace[0], maxWorkspace[1], maxWorkspace[2]);
    // Top/right/front point of device workspace.
    hduVector3Dd trf(maxWorkspace[3], maxWorkspace[4], maxWorkspace[5]);

    cout << "BBX1: " << llb << "\t" << trf << endl;
    */

    //create hl context
    m_hhlrc = hlCreateContext(m_hhd);
    hlMakeCurrent(m_hhlrc);

    hlEnable(HL_HAPTIC_CAMERA_VIEW);

    //set limits of haptic workspace in millimeters.
    hlWorkspace(-150, -45, 0, 150, 250, 90);

    UpdateWorkspace();

    m_boundaryId = hlGenShapes(1);
    m_obstaclesId = hlGenShapes(GetVizmo().GetEnv()->GetMultiBodies().size()-1);

    //initialize with force disabled
    hdDisable(HD_FORCE_OUTPUT);
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

void
Manager::
GetState() {
  hdGetDoublev(HD_CURRENT_POSITION, m_pos);
  hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, m_rot);
  hdGetDoublev(HD_CURRENT_VELOCITY, m_vel);
  hdGetDoublev(HD_CURRENT_FORCE, m_force);

  /*cout << endl << endl << "Haptic State" << endl;
  cout << "\tPos: " << m_pos << endl;
  cout << "\tRot: " << m_rot << endl;
  cout << "\tVel: " << m_vel << endl;
  cout << "\tForce: " << m_force << endl;*/

  hlGetDoublev(HL_DEVICE_POSITION, m_worldPos);

  /*cout << endl << "World State" << endl;
  cout << "\tPos: " << m_worldPos << endl;*/
}

void
Manager::
BoundaryRender() {

  hlTouchableFace(HL_BACK);

  //create a shape for the boundary
  hlHinti(HL_SHAPE_FEEDBACK_BUFFER_VERTICES, 8);

  hlMaterialf(HL_FRONT_AND_BACK, HL_STIFFNESS, 0.7f);
  hlMaterialf(HL_FRONT_AND_BACK, HL_DAMPING, 0.1f);

  hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, m_boundaryId);
  GetVizmo().GetEnv()->GetBoundary()->DrawHaptics();
  hlEndShape();
}

void
Manager::
ObstacleRender() {

  hlTouchableFace(HL_FRONT);

  size_t count = 0;
  const vector<MultiBodyModel*>& mbs = GetVizmo().GetEnv()->GetMultiBodies();
  typedef vector<MultiBodyModel*>::const_iterator CMIT;
  for(CMIT cmit = mbs.begin(); cmit != mbs.end(); ++cmit) {
    if((*cmit)->IsActive())
      continue;

    hlMaterialf(HL_FRONT_AND_BACK, HL_STIFFNESS, 1.0f);
    hlMaterialf(HL_FRONT_AND_BACK, HL_DAMPING, 0.0f);

    BodyModel* bm = *(*cmit)->Begin();
    PolyhedronModel* pm = bm->GetPolyhedronModel();

    hlHinti(HL_SHAPE_FEEDBACK_BUFFER_VERTICES, pm->GetNumVertices());
    hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, m_obstaclesId + count++);
    (*cmit)->DrawHaptics();
    hlEndShape();
  }
}

bool
Manager::
IsInsideBBX() {
  for(int i = 0; i < 3; ++i) {
    if(m_worldPos[i] < m_worldRange[i].first ||
        m_worldPos[i] > m_worldRange[i].second)
      return false;
  }

  return true;
}

void
Manager::
ToggleForceOutput() {
  if(hdIsEnabled(HD_FORCE_OUTPUT))
    hdDisable(HD_FORCE_OUTPUT);
  else
    hdEnable(HD_FORCE_OUTPUT);
}

}

#endif
