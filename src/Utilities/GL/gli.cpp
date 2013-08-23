#include "gli.h"
#include "gliCamera.h"
#include "gliUtility.h"
#include "PickBox.h"
#include "gliTransTool.h"
#include "Models/Vizmo.h"
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>

int GLI_SHOW_AXIS=1<<0;
int GLI_SHOW_PICKBOX=1<<1;
int GLI_SHOW_TRANSFORMTOOL=1<<2;
int GLI_SHOW_GRID=1<<3;


//set picking function
pick_func g_pick=NULL;
void gliSetPickingFunction(pick_func func) {
  g_pick=func;
}

//draw
void gliDraw(int option) {
  gliCamera * pcam=gliGetCameraFactory().getCurrentCamera();
  pcam->Draw();
  if(option & GLI_SHOW_GRID) gliDrawGrid();
  if(option & GLI_SHOW_PICKBOX) GetPickBox().Draw();
  if(option & GLI_SHOW_TRANSFORMTOOL) gliGetTransformTool().Draw();
  if(option & GLI_SHOW_AXIS) gliDrawRotateAxis(*pcam);
}

bool gliMP(QMouseEvent * e) {
  if( gliGetCameraFactory().getCurrentCamera()->MP(e) ){
    gliCM(); //camera moved
    return true;
  }
  if( gliGetTransformTool().MP(e) ) return true;

  GetPickBox().MousePressed(e);

  return false; //need further process
}

bool gliMR(QMouseEvent * e, bool drawonly)
{
  if( gliGetCameraFactory().getCurrentCamera()->MR(e) ){
    gliCM(); //camera moved
    return true;
  }
  if( gliGetTransformTool().MR(e) ) return true;

  //select
  PickBox& pick = GetPickBox();
  if(!drawonly) {
    if(pick.IsPicking()){
      pick.MouseReleased(e);
      if(g_pick != NULL){
        vector<GLModel*>& objs = GetPickedSceneObjs();
        //if(!(e->state() & Qt::ShiftButton)) objs.clear(); //add
        if(!(e->buttons() & Qt::ShiftModifier)) objs.clear(); //add
        vector<GLModel*>& newobjs = g_pick(pick.GetBox());
        objs.insert(objs.end(), newobjs.begin(), newobjs.end());
        //if(e->state()&Qt::ShiftButton)  newobjs=objs; //add
        if(e->buttons() & Qt::ShiftModifier) newobjs = objs; //add
        gliGetTransformTool().CheckSelectObject();
      }
    }
  }

  return false; //need further process
}

/// mouse movement event, return true if handled
bool gliMM( QMouseEvent * e ) {
  if( gliGetCameraFactory().getCurrentCamera()->MM(e) ){
    gliCM(); //camera moved
    return true;
  }
  if( gliGetTransformTool().MM(e) ) return true;

  GetPickBox().MouseMotion(e);

  return false; //need further process
}

/// key event, return true if handled
bool gliKEY( QKeyEvent * e ) {
  if( gliGetTransformTool().KEY(e) ) return true;
  return false;
}


bool gliCameraKEY( QKeyEvent * e ) {
  if( gliGetCameraFactory().getCurrentCamera()->KP(e) )return true;
  return false;
}

bool gliRobotKEY(QKeyEvent* _e) {
  return GetVizmo().GetRobot()->KP(_e);
}


/// window resize event
void gliWS( int w, int h ) {
  gliGetTransformTool().setWinSize(w,h);
  GetPickBox().SetWinSize(w,h);
}

/// camera move event
void gliCM() {
  gliGetTransformTool().CM();
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

  gliGetTransformTool().resetObj();

}

