//***************************************************************
// Vizmo 3D - A Visualization Tool for Motion Modelling
// Author: Renu Isaac
// Filename : ActorPropsDialog.C
// Handles all interaction for the ActorProperties dialog and has
// wrapper code for functions called.
//**************************************************************

#include "ActorPropsDialog.h"



ActorPropertiesDialog::ActorPropertiesDialog()
{
	/* constructor */
	myself = this;
}

ActorPropertiesDialog::~ActorPropertiesDialog()
{
	int i;
	/* destructor */
	for ( i=0 ; i < noOfActors; i++)
	{
		free(ActorList[i]);
	}
	free (ActorList);
}

void ActorPropertiesDialog::Initialize(void)
{
	int i;
	ActorList = (char**)malloc(sizeof(char*)*MAX_NUMACTORS);
	noOfActors = 3;
	for ( i=0 ; i < noOfActors; i++)
	{
		ActorList[i] = (char *)malloc(sizeof(char)*
			MAX_ACTORNAME_LENGTH);
		sprintf(ActorList[i],"Actor%d", i);
	}
	SelectedActorIndex = 0;
	red = green = blue = 50;
	isRobot = FALSE;
	x = y = z = 0.0;
	roll = pitch = yaw = 0.0;
	scaleX = scaleY = scaleZ = 1.0;
}
	

int ActorPropertiesDialog::setPickedActorIndex(int index)
{
	SelectedActorIndex = index;
	return index;
} 

char* ActorPropertiesDialog::getActorName(int i)
{
	/* return the list of actor names to be displayed */
	return ActorList[i];
}

int ActorPropertiesDialog::getNumberActors()
{
	return noOfActors;
}

float ActorPropertiesDialog::getOrientationValue( int i, int j)
{
	switch (i)
	{
		case 0:
			switch (j){
				case 0: return x;
				case 1: return y;
				case 2: return z;
			}break;
		case 1: 
			switch (j){
				case 0: return roll;
				case 1: return pitch;
				case 2: return yaw;
			}break;
		case 2:
			switch (j){
				case 0: return scaleX;
				case 1: return scaleY;
				case 2: return scaleZ;
			}break;
	}
	return 0.0;
}

int ActorPropertiesDialog::getIsRobot()
{
	/* current status of picked actor */
	return isRobot;
}

int  ActorPropertiesDialog::getCurrentColor(int hue)
{
	/* return the picked actors current color */
	switch (hue)
	{
		case 0:return red ;
		case 1:return green ;
		case 2:return blue ;
	}
	return 0;
}

void  ActorPropertiesDialog::OnOkClicked(void)
{
	/* transfer all values to main environment */
	/* maybe one set values function with list of parameters */
}

void  ActorPropertiesDialog::OnCancelClicked(void)
{
	/* do nothing for now */
}
