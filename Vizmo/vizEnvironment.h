#ifndef _ENVIRONMENT_H
#define _ENVIRONMENT_H

//***************************************************************
// Vizmo 3D - A Visualization Tool for Motion Modelling
// Author: Renu Isaac
// Filename : environ.h
// Purpose : declaration of class which handles the environment
// 	that is displayed by the viewer.
//**************************************************************

// vtk includes 
// #include "vtk.h"

// standard definitions/includes for all files
#include "vizIncl.h"
#include "vizList.h"
#include "vizAbstractObj.h"
#include "vizObject.h"

#include "vtkActor.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkBYUReader.h"
#include "vtkTclUtil.h"
#include "vtkPolyData.h"
#include "vtkSphereSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkCellArray.h"
#include "vtkFloatPoints.h"

#include "vizRoadmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector.h>

//typedef vector <vtkActor *> ActorList;


class vizEnvironment
{
	public:
		int noOfActors;
		int SelectedActorIndex;
		vizObject *pickedActor;
   		char DataFileName[MAX_FILENAME];
		List<vizObject> Actors;
 		vizRoadmap rdmp;



		vizEnvironment();
      		~vizEnvironment(void);
      		int Initialize(void);
		int Close(void);
      		int ReadFromFile(char *filename, char *path, char *Rend,char *XViewRend, Tcl_Interp *interp);
		int ReadFromModFile(char *modfilename, char *inifilename,
				char *dirname,char *Rend,char *XRendererName, Tcl_Interp *interp);
      		int SaveToFile(char *filename);
		int SaveToModFile(char *modfilename, char *inifilename);
		int AddActor(char *filename, char *RendererName, char *XRendererName,Tcl_Interp *interp);
		int DeleteActor();
		int PickActor(char *ptrActor,Tcl_Interp *interp);
      		int setOrientationValue(int confType, float v1, float v2, float v3);

		/* utility wrapper code */
		int setPickedActorIndex(int index);
		int getPickedActorIndex(void);
		char* getActorName(int i);
		char* getActorFileName(int i);
		int getNumberActors();
		int getNumberRobots();
		float getOrientationValue( int i, int j);
		int getIsRobot();
		int getIsSolid();
		int getCurrentColor(int hue);
		char *getDataFileName();
                int RandomizeColor();

		void ActorPropertiesDialogOnModifyClicked(int isRobot,
			int isSolid,
			float r, float g, float b, float posx, float posy,
			float posz, float orx, float ory, float orz, 
			float sfx, float sfy, float sfz);
		void ActorPropertiesDialogOnCloseClicked(void);
         	int DisplayRoadmap(char *file,int value,char *Rend,Tcl_Interp *interp);
		int HideRoadmap(char *Rend,Tcl_Interp *interp);
		int ReadRoadmap(char *file,int value);

	private:
 
 

};

#endif
