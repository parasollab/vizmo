#include "gli.h"
#include "gliCamera.h"
#include "gliUtility.h"
#include "gliPickBox.h"
#include "gliTransTool.h"

int GLI_SHOW_AXIS=1<<0;
int GLI_SHOW_PICKBOX=1<<1;
int GLI_SHOW_TRANSFORMTOOL=1<<2;
int GLI_SHOW_GRID=1<<3;


//set picking function
pick_func g_pick=NULL;
void gliSetPickingFunction(pick_func func)
{
    g_pick=func;
}

//draw
void gliDraw(int option){
    gliCamera * pcam=gliGetCameraFactory().getCurrentCamera();
    pcam->Draw();
    if( option & GLI_SHOW_GRID ) gliDrawGrid();
    if( option & GLI_SHOW_PICKBOX ) gliGetPickBox().Draw();
    if( option & GLI_SHOW_TRANSFORMTOOL ) gliGetTransformTool().Draw();
    if( option & GLI_SHOW_AXIS ) gliDrawRotateAxis(*pcam);
}

bool gliMP(QMouseEvent * e)
{
    if( gliGetCameraFactory().getCurrentCamera()->MP(e) ){
        gliCM(); //camera moved 
        return true;
    }
    if( gliGetTransformTool().MP(e) ) return true;
    gliGetPickBox().MP(e);
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
    gliPickBox& pick=gliGetPickBox();
	if( !drawonly ){
		if( pick.isPicking() ){
			pick.MR(e);
			if( g_pick!=NULL ){
				vector<gliObj>& objs=gliGetPickedSceneObjs();
				if(!(e->state()&Qt::ShiftButton)) objs.clear(); //add
				vector<gliObj>& newobjs=g_pick(pick.getPickBox());
				objs.insert(objs.end(),newobjs.begin(),newobjs.end());
				if(e->state()&Qt::ShiftButton)  newobjs=objs; //add
				gliGetTransformTool().CheckSelectObject();
			}
		}
	}

    return false; //need further process
}

//mouse movement evenet, return true if handled
bool gliMM( QMouseEvent * e )
{
    if( gliGetCameraFactory().getCurrentCamera()->MM(e) ){
        gliCM(); //camera moved
        return true;
    }
    if( gliGetTransformTool().MM(e) ) return true;
    gliGetPickBox().MM(e);

    return false; //need further process
}

//key evenet, return true if handled
bool gliKEY( QKeyEvent * e )
{
    if( gliGetTransformTool().KEY(e) ) return true;
    return false;
}

//window resize event
void gliWS( int w, int h )
{
    gliGetTransformTool().setWinSize(w,h);
    gliGetPickBox().setWinSize(w,h);
}

//camera move event
void gliCM()
{
    gliGetTransformTool().CM();
}

//simulate mouse up
void gliSimMouseUp()
{
    gliPickBox& pick=gliGetPickBox();
    // Dummy
    QMouseEvent * e=NULL;
    pick.MR(e); /* simulate pick mouse */  
}

bool gliPickBoxDim(int *xOffset,int *yOffset, int *w, int *h)
{
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
    
    gliPickBox& pick=gliGetPickBox();
    const gliBox box=pick.getPickBox();
    
    // handle all the ways the box can be drawn
    if(box.l<box.r)
        *xOffset=(int)box.l;
    else
        *xOffset=(int)box.r;
    
    if(box.b<box.t)
        *yOffset=(int)box.b;
    else
        *yOffset=(int)box.t;
    
    *w=(int)(box.r-box.l);
    *h=(int)(box.b-box.t);
    
    if(*w<0)
        *w=-*w;
    
    if(*h<0)
        *h=-*h;
    return true;
}

