#ifndef _VIZOBJECT_H
#define _VIZOBJECT_H

//#include "const.h"
//#include "struct.h"
//#include "sphere.h"
//#include "util.h"
/* #include "vtk.h" */
// try specific headers of classes you're using


#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkBYUReader.h"
#include "vtkTclUtil.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkMatrix4x4.h"
#include "vtkCellArray.h"
#include "vtkFloatPoints.h"
#include <tcl.h>
#include "vizBasics.h"
#include "vizAbstractObj.h"
#include "goalPosition.h"
#include <vtkTransform.h>

typedef struct {
  float conf[6];
} part_data;

 // **************** vizVisibleObject ************
 // Abstract base class of vizObject
 // ***********************************************
 class vizObject : public vizAbstractObj
 {
 public:
    	// constructor and destructor
    	vizObject(char *byufile);
    	~vizObject();

    	// access methods
    	char* getName();
    	char* getBYUFilename();
    	int isRobot();
    	int isSolid();
    	float getScaleFactor(int index);
	float getOrientation(int index);
    	float getColor(int index);
    	float getPosition(int index);
	vtkActor* getActor();
	float *GetCenterofGravity(void);
	float *getLastSavedLocation();
        part_data part;

    	void setName(char *newName);
    	void setRobot();
    	void setObstacle();
        void setSolid();
        void setWired();

    	void setScaleFactor( float sx, float sy, float sz);
    	void setColor(float r, float g, float b);
    	void setOrientation(float dx, float dy, float dz);
    	void setPosition( float x, float y, float z);
	void addOrientation(float dx, float dy, float dz);
	void addPosition( float x, float y, float z);
	goalPosition *TransformActor(vtkMatrix4x4 *currentMatrix);
	void setRenderer(char *RenName,char *RenXname, Tcl_Interp *interp);
	void saveCurrentLocation();
        void printConfiguration(void);
        void setCollisionFlag(void);
        void resetCollisionFlag(void);
        int inCollision(void);
      	vtkActor *actor;
 protected:
        int collisionFlag;
    	char name[100];
    	char byuFileName[100];
    	int  robotFlag;
    	int  solidFlag;
    	float scaleFactor[3];
	float color[3];
	float orientation[3];
    	float position[3];
	float lastsaved[6];
    	vtkRenderer *ren;
    	vtkRenderer *renx;
	vtkBYUReader *byu;
	vtkPolyData *polyData;
	vtkPolyDataMapper *mapper;
	vtkProperty *property;
 };

#endif
