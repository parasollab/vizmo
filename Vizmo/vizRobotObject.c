//***************************************************************
// Vizmo 3D - A Visualization Tool for Motion Modelling
// Author: Renu Isaac
// Filename : vizRobotObject.C
// Purpose : declaration of class which handles the robot objects
// 		during generation of queries
//**************************************************************

#include "vizRobotObject.h"

vizRobotObject::vizRobotObject()
{
//	strcpy(name, "Robot0");
//	goalPosition *temp;
//	/* add a start and end position as default */
//	temp = new goalPosition("pos0", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
//	goalPositions.Insert(temp);
//	temp = new goalPosition("pos1", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
//	goalPositions.Insert(temp);
//	noOfGoalPositions = 2;

}

vizRobotObject::~vizRobotObject()
{
	noOfGoalPositions = 0;
	goalPositions.DeleteObjects();
}

char *vizRobotObject::getName()
{
	return name;
}
char *vizRobotObject::getGoalName(int goalPos)
{
	if ( goalPos < noOfGoalPositions)
		return goalPositions[goalPos]->name;
	else return "posi";
}
void vizRobotObject::setGoalName(int goalPos, char *newname)
{
	strcpy(goalPositions[goalPos]->name, newname);
}
void vizRobotObject::setName(char* newName)
{
	strcpy(name, newName);
}
float vizRobotObject::getGoalPosition(int goalPos, int index)
{
	if ( goalPos < noOfGoalPositions && index < 6)
		return goalPositions[goalPos]->pos[index];
	else return 0.0;
}
int vizRobotObject::getNoOfGoalPositions(void)
{
	return noOfGoalPositions;
}
void vizRobotObject::setNoOfGoalPositions(int num)
{
	noOfGoalPositions = num;
}
void  vizRobotObject::setGoalPosition( int index,  float x, float y, float z,
			float orx, float op, float oy)
{
	if ( index <  noOfGoalPositions)
		goalPositions[index]->setInfo( x, y, z,orx, op, oy);
}
void  vizRobotObject::addGoalPosition(int indx )
{
	goalPosition *temp;
	//printf("\n vizRobotObject::addGoalPosition(%d)",indx);
	/* add a start and end position as default */
	temp = new goalPosition("pos0", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
	goalPositions.Insert(temp,indx);
	noOfGoalPositions++;
	
}
void  vizRobotObject::addGoalPosition()
{
        goalPosition *temp;
        //printf("\n vizRobotObject::addGoalPosition()");
        /* add a start and end position as default */
        temp = new goalPosition("pos0", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        goalPositions.Insert(temp);
        noOfGoalPositions++;

}

void  vizRobotObject::deleteGoalPosition( int index)
{
	goalPosition *temp;
	//printf("\n vizRobotObject::deleteGoalPosition at index %d", index);
	if ( index > 1 && index < noOfGoalPositions)
	{
		temp =  goalPositions[index];
		goalPositions.Remove(index);
		delete temp;
		noOfGoalPositions--;
	}
//        else
        //printf("Could not delete the first or the last\n");
}
void  vizRobotObject::setActorIndex(int index)
{
	actorIndex = index;
}
int vizRobotObject::getActorIndex(void)
{
	return actorIndex;
}
