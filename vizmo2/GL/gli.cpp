#include "gli.h"
#include "gliCamera.h"
#include "gliUtility.h"
#include "gliPickBox.h"
#include "gliTransTool.h"

//set picking function
pick_func g_pick=NULL;
void gliSetPickingFunction(pick_func func)
{
    g_pick=func;
}

//draw
void gliDraw(){
    gliCamera * pcam=gliGetCameraFactory().getCurrentCamera();
    pcam->Draw();
    gliDrawGrid();
    gliGetPickBox().Draw();
    gliGetTransformTool().Draw();
    gliDrawRotateAxis(*pcam);
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

bool gliMR(QMouseEvent * e)
{
    if( gliGetCameraFactory().getCurrentCamera()->MR(e) ){
        gliCM(); //camera moved
        return true;
    }
    if( gliGetTransformTool().MR(e) ) return true;

    //select
    gliPickBox& pick=gliGetPickBox();
    if( pick.isPicking() ){
        pick.MR(e);
        if( g_pick!=NULL ){
            vector<gliObj>& objs=gliGetPickedSceneObjs();
            objs.clear();
            vector<gliObj> newobjs=g_pick(pick.getPickBox());
            objs.insert(objs.end(),newobjs.begin(),newobjs.end());
            gliGetTransformTool().CheckSelectObject();
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

