#ifndef _VIZMO_H
#define _VIZMO_H
//***************************************************************
// Vizmo 3D - A Visualization Tool for Motion Modelling
// Author: Renu Isaac
// Filename : Vizmo.h
// Headers for includes and prototypes used by Vizmo.C - Main
//      routine for initialization and invoking the event loop
//***************************************************************


#include "vizIncl.h"
#include "vizEnvironment.h"
#include "Path.h"
#include "Query.h"
#include "OBPRM.h"
#include "RoadmapGraph.h"

//RoadmapGraph <Cfg,WEIGHT> rdmp;


#define  EXPORT(a,b) a b


//****************************************************************
// my exported functions 
//****************************************************************
#ifdef SGI
EXTERN EXPORT(int, Vizmolibsgi_Init) _ANSI_ARGS_((Tcl_Interp *interp));
#endif

#ifdef LINUX
EXTERN EXPORT(int, Vizmoliblinux_Init) _ANSI_ARGS_((Tcl_Interp *interp));
#endif


//***************************************************************
// prototypes for callbacks from ActorProps dialog
//****************************************************************

extern "C" int ActorPropsOnModifyClicked(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] );
extern "C" int ActorPropsOnCloseClicked(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] );
extern "C" int ActorPropsGetIsRobot(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] );
extern "C" int ActorPropsGetIsSolid(ClientData clientData, Tcl_Interp *interp,
                int argc, char *argv[] );

extern "C" int ActorPropsGetCurrentColor(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] );
extern "C" int ActorPropsGetOrientationValue(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] );
extern "C" int ActorPropsSetOrientationValue(ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int ActorPropsGetActorName(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] );
extern "C" int ActorPropsGetActorFileName( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);

//***************************************************************
// prototypes for callbacks into vizEnvironment objects
//****************************************************************
extern "C" int vizEnvironmentGetNumberActors(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] );
extern "C" int vizEnvironmentSetPickedActorIndex(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] );
extern "C" int vizEnvironmentGetPickedActorIndex(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] );
extern "C" int vizEnvironmentAddActor(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] );
extern "C" int vizEnvironmentDeleteActor(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] );
extern "C" int vizEnvironmentPickActor (ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int vizEnvironmentOpen( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int vizEnvironmentModOpen ( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int vizEnvironmentSave( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int vizEnvironmentClose( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int vizEnvironmentRandomizeColor( ClientData clientData,
                        Tcl_Interp *interp, int objc,
                        Tcl_Obj *CONST objv[]);

extern "C" int vizEnvironmentGetFileName ( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int vizEnvironmentGetNumberRobots(ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int vizEnvironmentDisplayRoadmap(ClientData clientData,
                        Tcl_Interp *interp, int objc,
                        Tcl_Obj *CONST objv[]);
extern "C" int vizEnvironmentHideRoadmap(ClientData clientData,
                        Tcl_Interp *interp, int objc,
                        Tcl_Obj *CONST objv[]);



//***************************************************************
// prototypes for callbacks into Query objects
//****************************************************************

extern "C" int QueryInitialize( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryGetPickedRobotIndex( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QuerySetPickedRobotIndex( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryAddGoal( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryDeleteGoal( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryGetPickedConfigIndex( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QuerySetPickedConfigIndex( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryGetConfigValue( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QuerySetConfigValue( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryGetRobotFileName( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryGetNumberRobotGoals( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryGetGoalName( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryGetFileName( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryGetNumberRobots( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryGetRobotName( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryGetNumberRobots( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryGetCurrentConfiguration( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QuerySave( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryClose( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int QueryOpen( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);

//***************************************************************
// prototypes for callbacks into Path objects
//****************************************************************
extern "C" int PathOpen( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int PathInitialize( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);

extern "C" int HideSwapVolume( ClientData clientData,
                        Tcl_Interp *interp, int argc,
                        char *argv[]);


extern "C" int ShowSwapVolume( ClientData clientData,
                        Tcl_Interp *interp, int argc,
                        char *argv[]);

extern "C" int PathClose( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int PathGetCurrentFrame( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);
extern "C" int PathSetDirection( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);

extern "C" int PathGoToStep( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);

extern "C" int PathGetNumberFrames( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);


//****************************************************************
// general purpose callbacks 
//****************************************************************

extern "C" int GetFileNames( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[]);



#endif

