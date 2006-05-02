#ifndef _QUERY_H
#define _QUERY_H

//***************************************************************
// Vizmo 3D - A Visualization Tool for Motion Modelling
// Author: Renu Isaac
// Filename : Query.h
// Purpose : declaration of class which handles the user's query
// 	creation and specifications
//**************************************************************

// standard definitions/includes for all files
#include "vizIncl.h"
#include "vizEnvironment.h"
#include "vizList.h"
#include "vizRobotObject.h"


class Query
{
	public:
		vizEnvironment *theEnv;
		int noOfRobots;
		int SelectedRobotIndex;
		int SelectedConfigIndex;
		vizRobotObject *pickedRobot;
   		char DataFileName[MAX_FILENAME];
		List<vizRobotObject> Robots;

		Query();
      		~Query(void);
      		int Initialize(vizEnvironment *);
		int Close(void);
      		int ReadFromFile(char *filename);
      		int SaveToFile(char *filename);
		
		/* utility wrapper code */
		int setPickedRobotIndex(int index);
		int setSelectedConfigIndex(int index);
		int getPickedRobotIndex(void);
		int getSelectedConfigIndex(void);
		int addGoal();
		int deleteGoal();
		char* getRobotName(int i);
		char* getFileName();
      		char* getRobotFileName();
		int getNumberRobots();
		float getOrientationValue( int i, int j);
		int setActorsCurrentConfiguration();
};		
#endif
