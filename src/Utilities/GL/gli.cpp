#include "gli.h"

#include <QMouseEvent>
#include <QKeyEvent>

#include "Camera.h"
#include "GLUtilities.h"
#include "PickBox.h"
#include "TransformTool.h"
#include "Models/Vizmo.h"

int GLI_SHOW_AXIS=1<<0;
int GLI_SHOW_PICKBOX=1<<1;
int GLI_SHOW_TRANSFORMTOOL=1<<2;
bool SHIFT_CLICK = false;

//set picking function
pick_func g_pick=NULL;
void gliSetPickingFunction(pick_func func) {
  g_pick=func;
}

//draw
void gliDraw(int option) {
  Camera* cam = GetCameraFactory().GetCurrentCamera();
  cam->Draw();
  if(option & GLI_SHOW_PICKBOX) GetPickBox().Draw();
  if(option & GLI_SHOW_TRANSFORMTOOL) GetTransformTool().Draw();
  if(option & GLI_SHOW_AXIS) DrawRotateAxis(cam);
}

bool gliMP(QMouseEvent * e) {

  if(e->buttons() && (e->modifiers() == Qt::ShiftModifier))
    SHIFT_CLICK = true;
  else
    SHIFT_CLICK = false;

  if(GetCameraFactory().GetCurrentCamera()->MousePressed(e)) {
    gliCM(); //camera moved
    return true;
  }
  if( GetTransformTool().MousePressed(e) ) return true;

  GetPickBox().MousePressed(e);

  return false; //need further process
}

bool gliMR(QMouseEvent * e, bool drawOnly) {
  if(GetCameraFactory().GetCurrentCamera()->MouseReleased(e)) {
    gliCM(); //camera moved
    return true;
  }
  if( GetTransformTool().MouseReleased(e) ) return true;

  //select
  PickBox& pick = GetPickBox();
  if(!drawOnly){
    if(pick.IsPicking()){
      pick.MouseReleased(e);
      if(g_pick != NULL){
        vector<Model*>& objs = GetPickedSceneObjs();
        //Shift key not pressed; discard current selection and start anew
        if(SHIFT_CLICK == false)
          objs.clear();
        //Get new set of picked objects if shift key not pressed
        //If shift is pressed, these are additional objects to add to
        //selection
        vector<Model*>& newobjs = g_pick(pick.GetBox());
        objs.insert(objs.end(), newobjs.begin(), newobjs.end());

        if(SHIFT_CLICK == true)
          newobjs = objs;
        GetTransformTool().CheckSelectObject();
      }
    }
  }

  return false; //need further process
}

/// mouse movement event, return true if handled
bool gliMM(QMouseEvent* e) {
  if(GetCameraFactory().GetCurrentCamera()->MouseMotion(e)) {
    gliCM(); //camera moved
    return true;
  }
  if(GetTransformTool().MouseMotion(e))
    return true;

  GetPickBox().MouseMotion(e);

  return false; //need further process
}

/// key event, return true if handled
bool
gliKEY(QKeyEvent* e) {
  return GetTransformTool().KeyPressed(e);
}


bool
gliCameraKEY(QKeyEvent* e) {
  return GetCameraFactory().GetCurrentCamera()->KeyPressed(e);
}

/// window resize event
void gliWS( int w, int h ) {
  GetTransformTool().SetWindowSize(w,h);
  GetPickBox().SetWinSize(w,h);
}

/// camera move event
void gliCM() {
  GetTransformTool().CameraMotion();
}

//simulate mouse up
void gliSimMouseUp() {
  PickBox& pick = GetPickBox();
  // Dummy
  QMouseEvent * e=NULL;
  pick.MouseReleased(e); /* simulate pick mouse */
}

void
gliPickBoxDim(int *xOffset,int *yOffset, int *w, int *h) {
  /*
     P4 --------------- P3
     |                   |
     |                   |
     |                   |
     |                   |
     |                   |
     |                   |
     P1 --------------- P2
     */

  const Box& box = GetPickBox().GetBox();

  // handle all the ways the box can be drawn
  *xOffset = min(box.m_left, box.m_right);
  *yOffset = max(box.m_bottom, box.m_top);
  *w = abs(box.m_right - box.m_left);
  *h = abs(box.m_bottom - box.m_top);
}

void gliReset(){
  GetTransformTool().ResetSelectedObj();
}

