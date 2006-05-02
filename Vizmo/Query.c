//***************************************************************
// Vizmo 3D - A Visualization Tool for Motion Modelling
// Author: Renu Isaac
// Filename : Query.C
// Purpose : definition of class which handles the user's query
// 	creation and specifications
//**************************************************************
#include "Query.h"
 
Query::Query()
{
	noOfRobots = 0;
	theEnv = NULL; 
	SelectedRobotIndex = 0;
	SelectedConfigIndex = 0;
	pickedRobot = NULL;
        strcpy(DataFileName, "Untitled.cfg");
  
}

Query::~Query()
{
}

int Query::Initialize(vizEnvironment *anEnv)
{
	int numActors, i, isR;
	vizRobotObject *tempRobot;
	//printf("\nQuery::Initialize -> started");
	theEnv = anEnv;
	if (Robots.GetCount() > 0)
		Robots.DeleteObjects();
	noOfRobots = 0;
	strcpy(DataFileName, "Untitled.cfg");

	/* find number of robots and initialize the robot list here*/
	numActors = theEnv->getNumberActors();
	for (i=0;i<numActors;i++)
	{
		isR = (theEnv->Actors[i])->isRobot();
		if ( isR == TRUE) 
		{
			tempRobot = new vizRobotObject;
			tempRobot->setActorIndex(i);
			tempRobot->setName((theEnv->Actors[i])->getName());
			Robots.Insert(tempRobot);
			noOfRobots++;
		}
	}
	SelectedRobotIndex = 0;
	SelectedConfigIndex = 0;
	pickedRobot = Robots[SelectedRobotIndex];
	//printf("\nQuery::Initialize -> ended");
	return TCL_OK;
}

int Query::Close()
{
	/* check if data not saved, confirm saving and destroy query */
	//printf("\nQuery::Close -> started");
	theEnv = NULL; 
	if (Robots.GetCount() > 0)
		Robots.DeleteObjects();
	noOfRobots = 0;
	SelectedRobotIndex = 0;
	SelectedConfigIndex = 0;
	strcpy(DataFileName, "Untitled.cfg");
	pickedRobot = NULL;
	//printf("\nQuery::Close -> ended");
	return TCL_OK;
}

int Query::ReadFromFile(char *filename)
{
	/* read from a .cfg file */
	FILE *cfgFile;
	float tempf[6];
	int i, j, index, noOfGoals, noRobots;
	vizRobotObject *tempRobot;
	//printf ("\n Query::ReadFromFile->begin");
	if (( cfgFile = fopen(filename, "r")) == NULL)
	{
		sendUserMessage(INVALID_FILE);
		return TCL_OK;
	}
	strncpy(DataFileName, filename, MAX_FILENAME-1);
	//printf("\nReading from file %s", DataFileName);
	noRobots = 1;
	noOfGoals = 2;
	/*should ideally read noOfRobots and if it doesn't match with current environment - exit*/
	/*fscanf(cfgFile, "%d\n", &noOfRobots);*/
	if (noRobots != noOfRobots)
	{
		sendUserMessage(INVALID_FILE);
		fclose(cfgFile);
		Initialize(theEnv);
		return TCL_OK;
	}
	
	for (i = 0; i < noOfRobots; i++)
	{
		/*should read index and noOfGoals for each robot */
		index = 0;
		/*fscanf(cfgFile, "%d\n", &index);*/
		j = 0;
		while ( (Robots[j]->getActorIndex() != index) &&(j < noOfRobots))
			j++;
		if ( (Robots[j]->getActorIndex() == index) && 
			(theEnv->Actors[index]->isRobot()))
			tempRobot = Robots[j];
		else
		{	
			sendUserMessage(INVALID_FILE);
			fclose(cfgFile);
			Initialize(theEnv);
			return TCL_OK;
		}
		/*fscanf(envFile, "%d\n", &noOfGoals);*/
		/* delete any current goals, also decrements count upto default 2 */
		for ( j = 2; j < tempRobot->getNoOfGoalPositions(); j++)
			tempRobot->deleteGoalPosition(j);	
		
      		for ( j=0; j< noOfGoals; j++)
      		{
      			fscanf(cfgFile, "%f %f %f %f %f %f\n", 
				&tempf[0],&tempf[1],&tempf[2],
				&tempf[3],&tempf[4],&tempf[5]);
			if (j>1)
				/* only need to create a new goal if more than 2*/
				tempRobot->addGoalPosition();

         		tempRobot->setGoalPosition(j, 
				tempf[0], tempf[1], tempf[2],
				tempf[3],tempf[4],tempf[5]);
			tempRobot->setGoalName(j,"posj");
      		}
	}
       /* first element added */
       setPickedRobotIndex(0);
	setSelectedConfigIndex(0);
	pickedRobot = Robots[SelectedRobotIndex];
   	fclose(cfgFile);
   	//printf("\n Query::ReadFromFile-> ended");
	return TCL_OK;
}


int Query::SaveToFile(char *filename)
{
	/* save to a .cfg file */
	FILE *cfgFile;
	float tempf[6];
	int i, j;
	vizRobotObject *tempRobot;
	int noOfGoals;
	//printf ("\n Query::SaveToFile->begin");
	cfgFile = fopen(filename, "w");
	strcpy(DataFileName, filename);

	/*should ideally write noOfRobots */
	/*fprintf(cfgFile, "%d\n", noOfRobots); */

	for (i = 0; i < noOfRobots; i++)
	{
		tempRobot = Robots[i];

		/*should write index and noOfGoals for each robot */
		/*fprintf(cfgFile, "%d\n", tempRobot->getActorIndex());*/
		noOfGoals = tempRobot->getNoOfGoalPositions();
		/*fprintf(cfgFile, "%d\n",noOfGoals );*/

      		for ( j = 0; j < noOfGoals; j++)
      		{
			tempf[0] = tempRobot->getGoalPosition(j,0);
			tempf[1] = tempRobot->getGoalPosition(j,1);
			tempf[2] = tempRobot->getGoalPosition(j,2);
			tempf[3] = tempRobot->getGoalPosition(j,3);
			tempf[4] = tempRobot->getGoalPosition(j,4);
			tempf[5] = tempRobot->getGoalPosition(j,5);
			fprintf(cfgFile, "%f %f %f %f %f %f\n",
				tempf[0],tempf[1],tempf[2],
				tempf[3],tempf[4],tempf[5]);
      		}
	}
        fclose(cfgFile);
   	//printf("\n Query::SaveToFile-> ended");
	return TCL_OK;
}

int Query::setActorsCurrentConfiguration()
{
	int i;
	int tempP[3], tempO[3];
	for ( i= 0 ; i < 3; i++)
	{
		tempP[i] = theEnv->Actors[pickedRobot->getActorIndex()]->getPosition(i);
		tempO[i] = theEnv->Actors[pickedRobot->getActorIndex()]->getOrientation(i);
	}
	pickedRobot->setGoalPosition(SelectedConfigIndex,
		tempP[0],tempP[1],tempP[2],
		tempO[0],tempO[1],tempO[2]);
	return TCL_OK;
	
}
/************************************************************/
// WRAPPER CODE 
/************************************************************/

int Query::setPickedRobotIndex(int index)
{
	SelectedRobotIndex = index;
	//printf("Selected robot is %d", SelectedRobotIndex);
	pickedRobot = Robots[SelectedRobotIndex];
	return TCL_OK;
} 

int Query::getPickedRobotIndex(void)
{
	return SelectedRobotIndex;
} 

int  Query::setSelectedConfigIndex(int index)
{
	SelectedConfigIndex = index;
	return TCL_OK;
}
int  Query::getSelectedConfigIndex()
{
	return SelectedConfigIndex;
}
int  Query::addGoal()
{
        //printf("Adding the goal to %d\n", SelectedConfigIndex);
	Robots[SelectedRobotIndex]->addGoalPosition(SelectedConfigIndex);
	//Robots[SelectedRobotIndex]->addGoalPosition();
	SelectedConfigIndex ++;
	return TCL_OK;
}
int  Query::deleteGoal()
{
        //printf("Deleting the goal from %d\n",SelectedConfigIndex);
	Robots[SelectedRobotIndex]->deleteGoalPosition(SelectedConfigIndex);
	if (SelectedConfigIndex > 0)
		SelectedConfigIndex--;
	return TCL_OK;
}

char* Query::getRobotName(int i)
{
	if (i >= 0 && i < noOfRobots)
	{
		/* return the name of actor at index i*/
		return Robots[i]->getName();
	} else
		return NULL;
}

char*  Query::getFileName()
{
	return DataFileName;
}
char*  Query::getRobotFileName()
{
	return theEnv->Actors[pickedRobot->getActorIndex()]->getBYUFilename();
}

int Query::getNumberRobots()
{
	return noOfRobots;
}


