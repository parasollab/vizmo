#ifndef _GLI_H_
#define _GLI_H_

#include <vector>
using namespace std;

#include <qgl.h>

#include "gliDataStructure.h"

//draw
void gliDraw();

//mouse button presse event, return true if handled
bool gliMP( QMouseEvent * e );

//mouse button release event, return true if handled
bool gliMR( QMouseEvent * e,bool );

//mouse movement evenet, return true if handled
bool gliMM( QMouseEvent * e );

//key event, return true if handled
bool gliKEY( QKeyEvent * e );

//window resize event
void gliWS( int w, int h );

//camera move event
void gliCM();

// Get the picking box dimensions
bool gliPickBoxDim(int *xOffset,int *yOffset,int *w,int *h);

// simulate mouse up
void gliSimMouseUp();


//set select method
class gliBox;
typedef vector<gliObj> (*pick_func)(const gliBox&);
void gliSetPickingFunction(pick_func func);

#endif //_GLI_H_
