#ifndef _PATH_H
#define _PATH_H

//***************************************************************
// Vizmo 3D - A Visualization Tool for Motion Modelling
// Author: Renu Isaac
// Filename : path.h
// Purpose : declaration of class which handles the path loading
// 	and display.
//**************************************************************

// vtk includes
#include "vtkMatrix4x4.h"

// standard definitions/includes for all files
// #include <unistd.h>
#include "vizIncl.h"
#include "vizList.h"
#include "vizEnvironment.h"
#include "goalPosition.h"
#include "vector.h"

#define CONTINUOUS 0
#define STEP	1
#define FORWARD 1
#define REVERSE 0
#define NORMAL_SPEED	0
#define HIGH_SPEED	1
#define NORMAL_STEPTIME 100
#define QUICK_STEPTIME 500

class Path
{
	protected:
		vizEnvironment *theEnv;
		int noOfFrames;
		int currentFrame;
      		int playMode;
      		int playSpeed;
      		int playDirection;
		int noOfRobots;
		int startFlag;
		int stopFlag;
		int stepTime;
      		char modFileName[MAX_FILENAME];
		char initFileName[MAX_FILENAME];
		char mapFileName[MAX_FILENAME];
		char cfgFileName[MAX_FILENAME];
		char pathFileName[MAX_FILENAME];
		int robotIndex[MAX_NUMACTORS];
		int noTryConnect;
		List <goalPosition> Configurations;
                vector <vtkActor*> SwapVolumeActors;

	
	public :
      		Path();
                int ShowSwapVolume (char *RendererName,Tcl_Interp *interp);
                int HideSwapVolume (char *RendererName,Tcl_Interp *interp);
   		~Path(void);
      		int Initialize(vizEnvironment *env);
		int Close();
		int ReadFromFile(char *filename);
   		int SaveToFile(char *filename);
		void collision(int index);
		int GoToStep(int index);
		
		/* utility wrapper code */
      		int getCurrentFrame(void) const;
      		int getNoOfFrames(void) const;
      		int getPlayDirection(void) const;
		char* getPathFileName();

		int setCurrentFrame(int);
      		int setNoOfFrames(int);
		int setPlayDirection(int);
		int setPathFileName(char *newName);

};

#endif

