#ifndef _ACTORPROPSDIALOG_H
#define _ACTORPROPSDIALOG_H

//***************************************************************
// Vizmo 3D - A Visualization Tool for Motion Modelling
// Author: Renu Isaac
// Filename : ActorPropsDialog.h
// Handles all interaction for the ActorProperties dialog and has
// wrapper code for functions called.
//**************************************************************

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <tcl.h>
#include <tk.h>

#define MAX_ACTORNAME_LENGTH 20
#define MAX_NUMACTORS 200
#define TRUE 1
#define FALSE 0

class ActorPropertiesDialog 
{
	private:
		char **ActorList;
		int noOfActors;
		int SelectedActorIndex;
		int red,green,blue;
		int isRobot;
		float x,y,z;
		float roll, pitch, yaw;
		float scaleX, scaleY, scaleZ;
	public:
		ActorPropertiesDialog *myself;
		ActorPropertiesDialog();
		~ActorPropertiesDialog();
		void Initialize();
		void OnOkClicked();
		void OnCancelClicked();
		int getCurrentColor(int);
		int getIsRobot();
		float getOrientationValue(int , int);
		int getNumberActors();
		char *getActorName(int i);
		int setPickedActorIndex(int);
};

#endif
