#ifndef _VIZROBOTOBJECT_H
#define _VIZROBOTOBJECT_H

/* #include "vtk.h" */
// try specific headers of classes you're using


#include "vtkTclUtil.h"
#include <tcl.h>
#include "vizIncl.h"
#include "vizBasics.h"
#include "vizAbstractObj.h"
#include "vizList.h"
#include "goalPosition.h"

 // **************** vizRobotObject ************
 // Abstract base class of vizRobotObject
 // ***********************************************
 class vizRobotObject : public vizAbstractObj
 {
 public:
    	// constructor and destructor
    	vizRobotObject();
    	~vizRobotObject();

    	// access methods
    	char* getName();
	char *getGoalName(int index);
    	float getGoalPosition(int pos, int index);
      	int getNoOfGoalPositions(void);
	int getActorIndex(void);

	void setName(char *name);
    	void setGoalName(int goalPos,char *newname);
	void setNoOfGoalPositions(int num);
      	void setGoalPosition( int index,  float x, float y, float z,
			float orx, float op, float oy);
	void addGoalPosition(int indx);
	void addGoalPosition();
	void deleteGoalPosition( int index);
	void setActorIndex(int index);

 protected:

    	char name[100];
	int actorIndex;
    	int noOfGoalPositions;
      	List <goalPosition> goalPositions;
 };

#endif
