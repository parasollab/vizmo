//***************************************************************
//***************************************************************
// Vizmo 3D - A Visualization Tool for Motion Modelling
// Author: Renu Isaac
// Filename : vizEnvironment.C
// Handles all interaction for the vizEnvironment object, as well
// as all required wrapper code
//**************************************************************

#include "vizEnvironment.h"
#include "OBPRM.h"
#include "RoadmapGraph.h"
#include "Stat_Class.h"
#include "CollisionDetection.h"

Stat_Class Stats;
CollisionDetection cd;


int lengthPtr;
#define Tcl_GetString(_a_) Tcl_GetStringFromObj(_a_,&lengthPtr)

#include "vizmo.h"

//*****************************************************************
// my exported function
//*****************************************************************

FILE * logfd;
int verbosity=1;
int audio=0;
static int VizmoLibObjCmd _ANSI_ARGS_(( ClientData clientData, 
			Tcl_Interp *interp, int objc, 
			Tcl_Obj *CONST objv[]));

Tcl_Interp* myInterp;
vizEnvironment *thevizEnvironment;
Path *thePath;
Query *theQuery;


//*****************************************************************
// need to create objects with registered tcl call backs
//*****************************************************************
void initializeObjects()
{
        setbuf(stdout,NULL);
   	logfd=stdout;
	thevizEnvironment = new vizEnvironment;
	thevizEnvironment->Initialize();
   	thePath = new Path;
   	/* will only initialize path when required */
	theQuery = new Query;
	/* will only initialize query when required */

}
//*****************************************************************
// need to delete objects created in initialization
//*****************************************************************
void cleanupObjects()
{
	if (thevizEnvironment != NULL)
   	{
   		delete thevizEnvironment;
      		thevizEnvironment = NULL;
  	}
  	if (thePath != NULL)
  	{
   		delete thePath;
    		thePath = NULL;
  	}
	if (theQuery != NULL)
  	{
   		delete theQuery;
    		thePath = NULL;
  	}
}

//*****************************************************************
// Main routine of Vizmo 3D program which
// initializes the Tcl/Tk interface and application commands and
// invokes startup script.
//*****************************************************************

EXPORT(int,Vizmolib_Init)(Tcl_Interp *interp)
{


	/* perform my application's initialization and processing of
	input parameters */
	myInterp = interp;
	initializeObjects() ;

	/* register the dll entry point */
	/*
	Tcl_CreateCommand(interp, "VizmoLib",
   		VizmoLibCmd,
		NULL,
		NULL);

	*/
	Tcl_CreateObjCommand(interp, "VizmoLib",
   		VizmoLibObjCmd,
		NULL,
		NULL);

	return Tcl_PkgProvide(interp, "VizmoLib", "1.0");
}

static int VizmoLibObjCmd( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
	int result = 0;

	// register all your application specific commands
   	/*
	Tcl_CreateCommand(interp, "ActorPropsDisplay",
   		ActorPropsDisplay,
		(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	*/
	Tcl_CreateCommand(interp, "ActorPropsOnModifyClicked",
   		ActorPropsOnModifyClicked,
		(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "ActorPropsOnCloseClicked",
   		ActorPropsOnCloseClicked,
		(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "ActorPropsGetIsRobot",
   		ActorPropsGetIsRobot,
		(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
        Tcl_CreateCommand(interp, "ActorPropsGetIsSolid",
                ActorPropsGetIsSolid,
                (ClientData)NULL,
                (Tcl_CmdDeleteProc*)NULL);

	Tcl_CreateCommand(interp, "ActorPropsGetCurrentColor",
   		ActorPropsGetCurrentColor,
		(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "ActorPropsGetOrientationValue",
   		ActorPropsGetOrientationValue,
		(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "ActorPropsSetOrientationValue",
   		ActorPropsSetOrientationValue,
		(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "EnvironmentGetNumberActors",
   		vizEnvironmentGetNumberActors,
		(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "EnvironmentDisplayRoadmap",
                vizEnvironmentDisplayRoadmap,
                (ClientData)NULL,
                (Tcl_CmdDeleteProc*)NULL);
        Tcl_CreateObjCommand(interp, "EnvironmentHideRoadmap",
                vizEnvironmentHideRoadmap,
                (ClientData)NULL,
                (Tcl_CmdDeleteProc*)NULL);



	Tcl_CreateObjCommand(interp, "EnvironmentGetNumberRobots",
   		vizEnvironmentGetNumberRobots,
         	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "ActorPropsGetActorName",
   		ActorPropsGetActorName,
		(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "EnvironmentSetPickedActorIndex",
   		vizEnvironmentSetPickedActorIndex,
		(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "EnvironmentPickActor",
   		vizEnvironmentPickActor,
         (ClientData)NULL,
		   (Tcl_CmdDeleteProc*)NULL);
   	Tcl_CreateCommand(interp, "EnvironmentGetPickedActorIndex",
   		vizEnvironmentGetPickedActorIndex,
		(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "EnvironmentAddActor",
   		vizEnvironmentAddActor,
		(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "EnvironmentDeleteActor",
   		vizEnvironmentDeleteActor,
		(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);

   	Tcl_CreateObjCommand(interp, "EnvironmentOpen",
   		vizEnvironmentOpen,
         	(ClientData)NULL,
		   (Tcl_CmdDeleteProc*)NULL);
   	Tcl_CreateObjCommand(interp, "EnvironmentModOpen",
   		vizEnvironmentModOpen,
         	(ClientData)NULL,
		   (Tcl_CmdDeleteProc*)NULL);
   	Tcl_CreateObjCommand(interp, "EnvironmentGetFileName",
   		vizEnvironmentGetFileName,
         	(ClientData)NULL,
		   (Tcl_CmdDeleteProc*)NULL);
   	Tcl_CreateObjCommand(interp, "EnvironmentSave",
   		vizEnvironmentSave,
         	(ClientData)NULL,
		   (Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "EnvironmentClose",
   		vizEnvironmentClose,
         	(ClientData)NULL,
		   (Tcl_CmdDeleteProc*)NULL);
        Tcl_CreateObjCommand(interp, "RandomizeColor",
                vizEnvironmentRandomizeColor,
                (ClientData)NULL,
                   (Tcl_CmdDeleteProc*)NULL);

	Tcl_CreateObjCommand(interp, "ActorPropsGetActorFileName",
   		ActorPropsGetActorFileName,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);

	Tcl_CreateObjCommand(interp, "QueryInitialize",
   		QueryInitialize,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryGetPickedRobotIndex",
   		QueryGetPickedRobotIndex,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QuerySetPickedRobotIndex",
   		QuerySetPickedRobotIndex,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryGetPickedConfigIndex",
   		QueryGetPickedConfigIndex,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QuerySetPickedConfigIndex",
   		QuerySetPickedConfigIndex,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryGetConfigValue",
   		QueryGetConfigValue,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QuerySetConfigValue",
   		QuerySetConfigValue,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryGetRobotFileName",
   		QueryGetRobotFileName,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryGetNumberRobotGoals",
   		QueryGetNumberRobotGoals,
        	(ClientData)thevizEnvironment,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryGetGoalName",
   		QueryGetGoalName,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryGetFileName",
   		QueryGetFileName,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryGetNumberRobots",
   		QueryGetNumberRobots,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryGetRobotName",
   		QueryGetRobotName,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryGetNumberRobots",
   		QueryGetNumberRobots,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryGetCurrentConfiguration",
   		QueryGetCurrentConfiguration,
        	(ClientData)thevizEnvironment,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QuerySave",
   		QuerySave,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryClose",
   		QueryClose,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryOpen",
   		QueryOpen,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryAddGoal",
   		QueryAddGoal,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "QueryDeleteGoal",
   		QueryDeleteGoal,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "PathOpen",
   		PathOpen,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "PathInitialize",
   		PathInitialize,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
        Tcl_CreateCommand(interp, "ShowSwapVolume",
                ShowSwapVolume,
                (ClientData)NULL,
                (Tcl_CmdDeleteProc*)NULL);
        Tcl_CreateCommand(interp, "HideSwapVolume",
                HideSwapVolume,
                (ClientData)NULL,
                (Tcl_CmdDeleteProc*)NULL);


	Tcl_CreateObjCommand(interp, "PathClose",
   		PathClose,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "PathGetCurrentFrame",
   		PathGetCurrentFrame,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "PathSetDirection",
   		PathSetDirection,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "PathGoToStep",
   		PathGoToStep,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "PathGetNumberFrames",
   		PathGetNumberFrames,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateObjCommand(interp, "GetFileNames",
   		GetFileNames,
        	(ClientData)NULL,
		(Tcl_CmdDeleteProc*)NULL);
	

   	/* register a startup file name to be read in case program started
   	interactively */

	// result = Tcl_EvalFile(interp, "mywin2.tcl");
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
/*************************************************************************/
int ActorPropsOnModifyClicked(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] )
{
	thevizEnvironment->ActorPropertiesDialogOnModifyClicked(atoi(argv[1]),
		atof(argv[2]),atof(argv[3]),atof(argv[4]),atof(argv[5]),
		atof(argv[6]),atof(argv[7]),atof(argv[8]),atof(argv[9]),
		atof(argv[10]),atof(argv[11]),atof(argv[12]),atof(argv[13]),
                 atof(argv[14]));
	return TCL_OK;
}

int ActorPropsOnCloseClicked(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] )
{
	thevizEnvironment->ActorPropertiesDialogOnCloseClicked();
	return TCL_OK;
}
int ActorPropsGetIsRobot(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] )
{
	sprintf(interp->result, "%d", thevizEnvironment->getIsRobot());
	return TCL_OK;
}
int ActorPropsGetIsSolid(ClientData clientData, Tcl_Interp *interp,
                int argc, char *argv[] )
{
        sprintf(interp->result, "%d", thevizEnvironment->getIsSolid());
        return TCL_OK;
}

int ActorPropsGetCurrentColor(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] )
{
	sprintf(interp->result, "%d",
		thevizEnvironment->getCurrentColor(atoi(argv[1])));
	return TCL_OK;
}

int ActorPropsSetOrientationValue(ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
	int result;
	int conftype;
	double value[3];
	Tcl_GetIntFromObj( interp,objv[1],&conftype);
	Tcl_GetDoubleFromObj( interp,objv[2],&value[0]);
	Tcl_GetDoubleFromObj( interp,objv[3],&value[1]);
	Tcl_GetDoubleFromObj( interp,objv[4],&value[2]);
	result=thevizEnvironment->setOrientationValue( conftype,
			(float)value[0], 
			(float)value[1], 
			(float)value[2]);
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}

int ActorPropsGetOrientationValue(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] )
{
	sprintf(interp->result, "%f",
		thevizEnvironment->getOrientationValue(atoi(argv[1]),atoi(argv[2])));
	return TCL_OK;
}

int ActorPropsGetActorName(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] )
{
	sprintf(interp->result, "%s", thevizEnvironment->getActorName(atoi(argv[1])));
	return TCL_OK;
}
int ActorPropsGetActorFileName( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int temp, result, length;
	char tempStr[MAX_FILENAME];
	result = Tcl_GetIntFromObj( interp, objv[1], &temp);
	if (result != TCL_OK)
		return result;
   	strcpy(tempStr, thevizEnvironment->getActorFileName(temp));
	length = strlen(tempStr);
	//printf("\n ActorPropsGetActorFileName %s",tempStr );
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetStringObj(resultPtr, tempStr, length);
	return TCL_OK;
} 

int vizEnvironmentDisplayRoadmap(ClientData clientData,
                        Tcl_Interp *interp, int argc,
                        Tcl_Obj *CONST objv[])
{
   char temp[MAX_FILENAME];
   int val;
   char temp2[MAX_ANY_NAME];
   //printf("It is here");
   strcpy( temp, Tcl_GetString( objv[1]) );
   sscanf(Tcl_GetString( objv[2]),"%d",&val);
   strcpy( temp2, Tcl_GetString( objv[3]) );
   //printf("buttu\n");

   //printf("The name of the expected roadmap is %s value is%d\n",temp,val);
   return(thevizEnvironment->DisplayRoadmap(temp,val,temp2,interp));
}
int vizEnvironmentHideRoadmap(ClientData clientData,
                        Tcl_Interp *interp, int argc,
                        Tcl_Obj *CONST objv[])
{
   char temp[MAX_FILENAME];
   strcpy( temp, Tcl_GetString( objv[1]) );
   thevizEnvironment->HideRoadmap(temp,interp);
   return TCL_OK;


}


/*************************************************************************/
int vizEnvironmentGetNumberActors(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] )
{
	sprintf(interp->result, "%d", thevizEnvironment->getNumberActors());
	return TCL_OK;
}

int vizEnvironmentGetNumberRobots(ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{	
	Tcl_Obj *resultPtr;
	int result;
	result = thevizEnvironment->getNumberRobots();
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}

int vizEnvironmentSetPickedActorIndex(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] )
{
	sprintf(interp->result, "%d",
		thevizEnvironment->setPickedActorIndex(atoi(argv[1])));
	return TCL_OK;
}
int vizEnvironmentGetPickedActorIndex(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] )
{
	sprintf(interp->result, "%d",
		thevizEnvironment->getPickedActorIndex());
	return TCL_OK;
}

int vizEnvironmentPickActor (ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	char temp[MAX_ANY_NAME];
   	int result;
   	strncpy( temp, Tcl_GetString( objv[1]), MAX_ANY_NAME-1 );
   	result = thevizEnvironment->PickActor(temp, interp);
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;

}

int vizEnvironmentAddActor(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] )
{
       //cout << "Renderer " << argv[2] << endl << flush ;
	sprintf(interp->result, "%d",
		thevizEnvironment->AddActor(argv[1], argv[2],argv[3], interp));
	return TCL_OK;
}
int vizEnvironmentDeleteActor(ClientData clientData, Tcl_Interp *interp,
		int argc, char *argv[] )
{
	sprintf(interp->result, "%d",
		thevizEnvironment->DeleteActor());
	return TCL_OK;
}

int vizEnvironmentOpen( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	char temp[MAX_FILENAME], temp2[MAX_ANY_NAME], temp3[MAX_ANY_NAME];
   	char temp1[MAX_FILENAME];
   	int result;
   	strcpy( temp, Tcl_GetString( objv[1]) );
   	strcpy( temp1, Tcl_GetString( objv[2]) );
   	strcpy( temp2, Tcl_GetString( objv[3]) );
   	strcpy( temp3, Tcl_GetString( objv[4]) );
        cout << "Opening the environemnt " << temp << endl << flush;
   	result = thevizEnvironment->ReadFromFile(temp,temp1,temp2,temp3,interp);
   	//result = thevizEnvironment->ReadFromFile(temp,temp2,interp);
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
        if(result==TCL_ERROR) {
            delete thevizEnvironment;
            thevizEnvironment = new vizEnvironment;
            thevizEnvironment->Initialize(); }

	return TCL_OK;
}
int vizEnvironmentModOpen ( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	char temp[MAX_FILENAME], temp1[MAX_FILENAME];
	char temp3[MAX_FILENAME], temp2[MAX_ANY_NAME];
	char temp4[MAX_FILENAME];
   	int result;
   	strcpy( temp,  Tcl_GetString( objv[1]) );
   	strcpy( temp1, Tcl_GetString( objv[2]) );
   	strcpy( temp2, Tcl_GetString( objv[3]) );
   	strcpy( temp3, Tcl_GetString( objv[4]) );
   	strcpy( temp4, Tcl_GetString( objv[5]) );
   	result = thevizEnvironment->ReadFromModFile
		(temp, temp1, temp2,temp3,temp4,interp);
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int vizEnvironmentGetFileName ( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int  length;
	char tempStr[MAX_FILENAME];
	
   	strncpy(tempStr, thevizEnvironment->getDataFileName(), MAX_FILENAME-1);
	length = strlen(tempStr);
//	printf("\n vizEnvironmentGetFileName %s",tempStr );
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetStringObj(resultPtr, tempStr, length);
	return TCL_OK;
}


int vizEnvironmentSave( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	char temp[MAX_FILENAME], temp1[MAX_FILENAME],  temp2[MAX_FILENAME];
   	int result;
   	strcpy( temp,Tcl_GetString(objv[1]));
	strcpy( temp1, Tcl_GetString( objv[2]) );
   	strcpy( temp2, Tcl_GetString( objv[3]) );
   	/* also takes backup in mod and init */
/*	result = thevizEnvironment->SaveToModFile(temp1, temp2);
        
	if (result == TCL_OK)  */
		result = thevizEnvironment->SaveToFile(temp);
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}

int vizEnvironmentClose( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
        int result;
    	result = thevizEnvironment->Close();
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int vizEnvironmentRandomizeColor( ClientData clientData,
                        Tcl_Interp *interp, int objc,
                        Tcl_Obj *CONST objv[])
{
        Tcl_Obj *resultPtr;
        int result;
        result = thevizEnvironment->RandomizeColor();
        resultPtr = Tcl_GetObjResult(interp);
        Tcl_SetIntObj(resultPtr, result);
        return TCL_OK;
}


/*************************************************************************/
int QueryGetPickedRobotIndex( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
        int result;
    	result = theQuery->getPickedRobotIndex();
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int QuerySetPickedRobotIndex( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
        int result, temp;
	result = Tcl_GetIntFromObj( interp, objv[1], &temp);
	if (result != TCL_OK)
		return result;
	result = theQuery->setPickedRobotIndex(temp);
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int QueryAddGoal( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
        int result;
	//printf("\n QueryAddGoal" );
	result = theQuery->addGoal();
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int QueryDeleteGoal( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
        int result;
	//printf("\n QueryDeleteGoal" );
	result = theQuery->deleteGoal();
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}

int QueryGetPickedConfigIndex( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
        int result;
	result = theQuery->getSelectedConfigIndex();
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int QuerySetPickedConfigIndex( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
        int result, temp;
	result = Tcl_GetIntFromObj( interp, objv[1], &temp);
	if (result != TCL_OK)
		return result;
	result = theQuery->setSelectedConfigIndex(temp);
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int QueryGetConfigValue( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
	int temp[4];
	float result;
	Tcl_GetIntFromObj( interp,objv[1],&temp[0]);
	Tcl_GetIntFromObj( interp,objv[2],&temp[1]);
	Tcl_GetIntFromObj( interp,objv[3],&temp[2]);
	Tcl_GetIntFromObj( interp,objv[4],&temp[3]);
	/*printf("QueryGetConfigValue for robot[%d] goal[%d]",temp[0],temp[1] );*/
	result = theQuery->Robots[temp[0]]->getGoalPosition(temp[1],
   		temp[2]*3 + temp[3]);
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetDoubleObj(resultPtr, (double)result);
	return TCL_OK;
}

int QuerySetConfigValue( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
	int tempi[2];
	double tempf[6];

	Tcl_GetIntFromObj( interp,objv[1],&tempi[0]);
	Tcl_GetIntFromObj( interp,objv[2],&tempi[1]);

	Tcl_GetDoubleFromObj( interp,objv[3],&tempf[0]);
	Tcl_GetDoubleFromObj( interp,objv[4],&tempf[1]);
	Tcl_GetDoubleFromObj( interp,objv[5],&tempf[2]);
	Tcl_GetDoubleFromObj( interp,objv[6],&tempf[3]);
	Tcl_GetDoubleFromObj( interp,objv[7],&tempf[4]);
	Tcl_GetDoubleFromObj( interp,objv[8],&tempf[5]);

	/* robot, goal index, x, y, z,r p,y */
	theQuery->Robots[tempi[0]]->setGoalPosition(tempi[1], 
		(float)tempf[0],(float)tempf[1],(float)tempf[2],
		(float)tempf[3],(float)tempf[4],(float)tempf[5]);

	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, 0);
	return TCL_OK;
}
int QueryGetRobotFileName( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int  length;
	char tempStr[MAX_FILENAME];
	
   	strcpy(tempStr, theQuery->getRobotFileName());
	length = strlen(tempStr);
	//printf("\n QueryGetRobotFileName %s",tempStr );
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetStringObj(resultPtr, tempStr, length);
	return TCL_OK;
	
}
int QueryGetNumberRobotGoals( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int result, temp;
	result = Tcl_GetIntFromObj( interp, objv[1], &temp);
	if (result != TCL_OK)
		return result;
	result = theQuery->Robots[temp]->getNoOfGoalPositions();	
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int QueryGetGoalName( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int  temp[2], length;
   	char tempStr[MAX_ANY_NAME];
	Tcl_GetIntFromObj( interp, objv[1], &temp[0]);
	Tcl_GetIntFromObj( interp, objv[2], &temp[1]);
   	strcpy(tempStr, theQuery->Robots[temp[0]]->getGoalName(temp[1]));
	length = strlen(tempStr);
	//printf("\n QueryGetGoalName %s",tempStr );
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetStringObj(resultPtr, tempStr, length);
	return TCL_OK;
}
int QueryGetFileName( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int length;
	char tempStr[MAX_FILENAME];
   	strcpy(tempStr, theQuery->getFileName());
	length = strlen(tempStr);
	//printf("\n QueryGetFileName %s",tempStr );
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetStringObj(resultPtr, tempStr, length);
	return TCL_OK;
}
int QueryGetNumberRobots( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
	int result;
	result = theQuery->getNumberRobots();
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result );
	return TCL_OK;
}
int QueryGetRobotName( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int result, temp, length;
	char tempStr[MAX_FILENAME];
	result = Tcl_GetIntFromObj( interp, objv[1], &temp);
	if (result != TCL_OK)
		return result;
   	strcpy(tempStr, theQuery->getRobotName(temp));
	length = strlen(tempStr);
	//printf("\n QueryGetRobotName %s",tempStr );
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetStringObj(resultPtr, tempStr, length);
	return TCL_OK;
}
int QueryGetCurrentConfiguration( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int result;
	//printf("\n QueryGetCurrentConfiguration" );
	result = theQuery->setActorsCurrentConfiguration();
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int QueryInitialize( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
        int result;
    	result = theQuery->Initialize(thevizEnvironment);
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int QueryOpen( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	char temp[MAX_FILENAME];
   	int result;
   	strcpy( temp, Tcl_GetString( objv[1]) );
   	result = theQuery->ReadFromFile(temp);
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int QuerySave( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	char temp[MAX_FILENAME];
   	int result;
   	strcpy( temp, Tcl_GetString( objv[1]) );
   	//printf("\n QuerySave to %s",temp );
	result = theQuery->SaveToFile(temp);
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int QueryClose( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
	theQuery->Close();
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, 0);
	return TCL_OK;
}
/*************************************************************************/
int PathOpen( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	char temp[MAX_FILENAME];
   	int result;
   	strcpy( temp, Tcl_GetString( objv[1]) );
	result = thePath->ReadFromFile(temp);
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}

int PathInitialize( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int result;
	result= thePath->Initialize(thevizEnvironment);
	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int ShowSwapVolume( ClientData clientData,
                        Tcl_Interp *interp, int argc,char *argv[])
                        
{
        Tcl_Obj *resultPtr;
        int result;
	sprintf(interp->result, "%d",
            thePath->ShowSwapVolume(argv[1], interp));
	return TCL_OK;
        
}

int HideSwapVolume( ClientData clientData,
                        Tcl_Interp *interp, int argc,char *argv[])

{
        Tcl_Obj *resultPtr;
        int result;
        sprintf(interp->result, "%d",
            thePath->HideSwapVolume(argv[1], interp));
        return TCL_OK;

}


int PathClose( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int result;
	result= thePath->Close();
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}

int PathGetCurrentFrame( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int result;
   	result = thePath->getCurrentFrame();
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}

int PathSetDirection( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int result, temp;
	result = Tcl_GetIntFromObj( interp, objv[1], &temp);
	if (result != TCL_OK)
		return result;
   	result = thePath->setPlayDirection(temp);
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}
int PathGoToStep( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int result, index;
	result =  Tcl_GetIntFromObj( interp, objv[1], &index);
	if (result != TCL_OK)
		return result;
   	result = thePath->GoToStep(index);
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}

int PathGetNumberFrames( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int result;
   	result = thePath->getNoOfFrames();
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetIntObj(resultPtr, result);
	return TCL_OK;
}


/**************************************************************************/
/*                  General Purpose functions                             */
/**************************************************************************/
/* this function is called at path generation time,  needs current */
/* environment name, and current path, query or roadmap file names */
int GetFileNames( ClientData clientData,
			Tcl_Interp *interp, int objc,
			Tcl_Obj *CONST objv[])
{
	Tcl_Obj *resultPtr;
   	int  length;
	char tempStr[MAX_FILENAME];
	int result, temp;
	result = Tcl_GetIntFromObj( interp, objv[1], &temp);
	if (result != TCL_OK)
		return result;
	switch (temp)
	{
		case ENVIRONMENT : 
			strcpy(tempStr, thevizEnvironment->getDataFileName());	
			break;
		case ROADMAP : strcpy(tempStr, "Untitled.map");
			/* need to insert roadmaps name */
			/* strcpy(tempStr, theRoadmap>getFileName(); */
			break;
		case QUERY: strcpy(tempStr, theQuery->getFileName());
			break;
		case PATH:strcpy(tempStr, thePath->getPathFileName());
			break;
		default:strcpy(tempStr, "Untitled");
			result = TCL_ERROR;
			break;
	}
	length = strlen(tempStr);
	//printf("\n GetFileNames %s",tempStr );
 	resultPtr = Tcl_GetObjResult(interp);
	Tcl_SetStringObj(resultPtr, tempStr, length);
	return TCL_OK;
}

/***********************************************************************/
/*    CheckValidFile - safety feature for use in opening files         */
/* 	used by vizEnvironments, Paths, Queries				     */
/***********************************************************************/

int CheckValidFile(char *filename) 
{
	FILE *fileptr;
	int returnVal;
	if ( (fileptr = fopen(filename,"r")) == NULL)
		returnVal = 0;
	else 
	{
		returnVal = 1;
		fclose(fileptr);
	}
	return returnVal;
}
/***********************************************************************/
/*    updateStatus - used by modules wishing to change status message  */
/***********************************************************************/
int updateStatus(int messageNo)
{
	char functCall[MAX_ANY_NAME];
	sprintf(functCall, "UpdateStatus %d", messageNo);
	Tcl_Eval(myInterp,functCall); 
	return TCL_OK;
}

/***********************************************************************/
/*    updateCursor - used by modules wishing to update cursor          */
/***********************************************************************/
int updateCursor(int cursorType)
{
	char functCall[MAX_ANY_NAME];
	sprintf(functCall, "UpdateCursor %d", cursorType);
	Tcl_Eval(myInterp,functCall ); 
	return TCL_OK;
}
/***********************************************************************/
/*    sendUserMessage- used by modules wishing to display message      */
/***********************************************************************/
int sendUserMessage(int messageType) {
	char functCall[MAX_ANY_NAME];
	sprintf(functCall, "SendUserMessage %d", messageType);
	Tcl_Eval(myInterp,functCall ); 
	return TCL_OK;
}

