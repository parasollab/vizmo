#ifndef _GLI_H_
#define _GLI_H_

#include <vector>
using namespace std;

#include <qgl.h>

extern int GLI_SHOW_AXIS;
extern int GLI_SHOW_PICKBOX;
extern int GLI_SHOW_TRANSFORMTOOL;
extern int GLI_SHOW_GRID;

//draw
void gliDraw(int option=0);

/// mouse button presse event, return true if handled
bool gliMP( QMouseEvent * e );

//mouse button release event, return true if handled
bool gliMR( QMouseEvent * e, bool drawonly=false );

/// mouse movement evenet, return true if handled
bool gliMM( QMouseEvent * e );

/// key event, return true if handled
bool gliKEY( QKeyEvent * e );

bool gliCameraKEY( QKeyEvent * e );

bool gliRobotKEY( QKeyEvent * e);

/// window resize event
void gliWS( int w, int h );

/// camera move event
void gliCM();

//set select method
class Box;
class GLModel;
typedef vector<GLModel*>& (*pick_func)(const Box&);
void gliSetPickingFunction(pick_func func);

/// simulate mouse up
void gliSimMouseUp();

// Get the picking box dimensions
void gliPickBoxDim(int *xOffset,int *yOffset,int *w,int *h);

//Reset gliObj
void gliReset();

#endif //_GLI_H_
