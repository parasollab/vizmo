#include <stdio.h>
#include "vizObject.h"
//#include "read_byu.h"
//#include "collision_check.h"
#include <vtkTransformPolyDataFilter.h>
  
//Constructor and Destructor
vizObject::vizObject(char *byufile)
{
	int i;
        float dummy=1.0;

  	// Initialize instance variables
  	strncpy(name,"Noname",99);
  	strncpy(byuFileName,byufile,99);
	//printf("\n vizObject::vizObject -> Actor created from file %s",byuFileName);
  	robotFlag = 0;
  	solidFlag= 1;
   	
  	for ( i=0; i< 3 ; i++)
	{
		scaleFactor[i] = 1.0;
		color[i] = 0.8;
		orientation[i] = 0.0;
		position[i] = 0.0;
      		lastsaved[i] = 0.0;
		lastsaved[i+3]= 0.0;
	}
	actor = NULL;
  	ren = NULL;
  	//renx = NULL;

  	// Reading byu file with vtkBYUReader
	
  	byu = vtkBYUReader::New();
  	byu->SetGeometryFileName(byuFileName);
	byu->Update();

	
	polyData = vtkPolyData::New();
  	polyData->CopyStructure(byu->GetOutput());

  	
	mapper = vtkPolyDataMapper::New();
  	mapper->SetInput(polyData);

	
	property = vtkProperty::New();
	property->SetSpecularColor(1,1,1);
    	property->SetSpecular(0.3);
    	property->SetSpecularPower(20);
    	property->SetAmbient(0.2);
    	property->SetDiffuse(0.8);
	property->SetColor(color[0], color[1], color[2]);

  	actor = vtkActor::New();
  	actor->SetMapper(mapper);
  	actor->SetProperty(property);

/*
	actor->SetOrigin(GetCenterofGravity());
*/
        float *f=GetCenterofGravity();
        vtkTransform *v=vtkTransform::New();
        v->Translate(-f[0],-f[1],-f[2]);
        vtkTransformPolyDataFilter *filt= vtkTransformPolyDataFilter::New();
        filt->SetTransform(v);
        filt->SetInput(polyData);
        mapper->SetInput(filt->GetOutput());
      
      
	//printf("\n vizObject::vizObject -> ended");
        //printf("Now reading from OBPRM\n");
/*
        Read_Part_From_BYU_File(0, byuFileName,&part);
        for (i=0; i<part.nvert; i++) {
           part.vert[i].lvert.x-=f[0],
           part.vert[i].lvert.y-=f[1],
           part.vert[i].lvert.z-=f[2];
  }

        Compute_Part_Triangle_Radius(&part);
	Compute_Part_Property(&part);
	Norma_Area_of_Triangles_of_Part(&part);
        part.root=NULL;
	Init_Spherical_Approx(part.nvert,&dummy);*/
}

/* traverses all the vertices to compute the center of gravity */
float * vizObject::GetCenterofGravity(void)
{
	int noVertices, i;
	float *points;
	float sumx, sumy, sumz;
	static float cog[3];
	noVertices = polyData->GetNumberOfPoints();
	//printf("\n noVertices = %d", noVertices);
	sumx = 0.0;
        sumy = 0.0;
        sumz = 0.0;
	for ( i=0; i <noVertices; i++)
	{
		points = polyData->GetPoint(i);
		sumx += points[0];
        	sumy += points[1];
        	sumz += points[2];
	}
	cog[0] = sumx /noVertices; 
	cog[1] = sumy /noVertices; 
	cog[2] = sumz /noVertices; 
	//printf("\n Actor new COG = %f %f %f",cog[0] , cog[1], cog[2]);
	return (cog);
}
vizObject::~vizObject()
{
	//printf("Deleting actor\n ");
	ren->RemoveActor(actor);
	//renx->RemoveActor(actor);
  	actor->Delete();
	/* explicitly delete all the other objects created */
	property->Delete();
	mapper->Delete();
	polyData->Delete();
	byu->Delete();
}

char* vizObject::getName() 
{
	return name;
}
char* vizObject::getBYUFilename()
{
	return byuFileName;
}
int vizObject::isRobot()
{
	return robotFlag;
}
int vizObject::isSolid()
{
        return solidFlag;
}

float vizObject::getScaleFactor(int index)
{
	return scaleFactor[index];
}
float vizObject::getOrientation(int index)
{
	return orientation[index];
} 
float vizObject::getColor(int index)
{
	return color[index];
}
float vizObject::getPosition(int index)
{
	return position[index];
}


vtkActor* vizObject::getActor()
{
	return actor;
}
void vizObject::setName(char *newName)
{
	strncpy(name,newName,99);
}	
void vizObject::setRobot()
{
	robotFlag = 1;
}
void vizObject::setObstacle()
{
	robotFlag = 0;
}
void vizObject::setSolid()
{
        solidFlag = 1;
	actor->GetProperty()-> SetRepresentationToSurface();
}

void vizObject::setWired()
{
        solidFlag = 0;
	actor->GetProperty()-> SetRepresentationToWireframe();
}


    	    		    	   	    	
void vizObject::setScaleFactor( float sx, float sy, float sz)
{
	scaleFactor[0] = sx;
	scaleFactor[1] = sy;
	scaleFactor[2] = sz;
	/* scaling the object should be done independently 
	and stored to a new .g file which is reread */
}
void vizObject::setColor(float r, float g, float b)
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
	actor->GetProperty()->SetColor(r,g,b);
}
void vizObject::setOrientation(float dx, float dy, float dz)
{
        float *a;
	orientation[0] = dx;
	orientation[1] = dy;
	orientation[2] = dz;
        a=actor->GetOrientation();
/*
        printf("orientation %f %f %f\n",orientation[0],
			orientation[1],orientation[2]);
        printf("En Early %f %f %f\n",a[0],a[1],a[2]);
*/
	/* must set orientation and update the display */
	//actor->SetPosition(0,0,0);
	actor->SetOrientation(orientation[0], 
				orientation[1], 
				orientation[2]);

        //return;
        a=actor->GetOrientation();
        //printf("original %f %f %f\n",a[0],a[1],a[2]);
	actor->SetOrientation(0,0,0);
        a=actor->GetOrientation();
        //printf("Before %f %f %f\n",a[0],a[1],a[2]);
         actor->RotateZ(orientation[2]);
         actor->RotateY(orientation[1]);
         actor->RotateX(orientation[0]);
        a=actor->GetOrientation();
        //printf("After %f %f %f\n",a[0],a[1],a[2]);

}

void vizObject::setPosition( float x, float y, float z)
{
	position[0] = x;
	position[1] = y;
	position[2] = z;
	/* must set position  and update the display */
        //printf("Setting position to %f %f %f\n",x,y,z);
	actor->SetPosition(position[0], position[1], position[2]);
}
void vizObject::saveCurrentLocation()
{
	lastsaved[0] = position[0];
	lastsaved[1] = position[1];
	lastsaved[2] = position[2];
	lastsaved[3] = orientation[0];
	lastsaved[4] = orientation[1];
	lastsaved[5] = orientation[2];
}
float *vizObject::getLastSavedLocation()
{
	return lastsaved;
}

void vizObject::addOrientation(float dx, float dy, float dz)
{
	float *currOrientation;
	/* must add the new orientation and update the current orientation */
	//actor->AddOrientation(dx, dy, dz);
      
	//currOrientation = actor->GetOrientation();
	setOrientation(orientation[0]+ dx, orientation[1]+ dy, orientation[2]+ dz);
       

	//orientation[1] = currOrientation[1];
	//orientation[2] = currOrientation[2];
}
void vizObject::addPosition( float x, float y, float z)
{
	float *currPosition;
	/* must add the given position and update the current position */
	actor->AddPosition(x, y, z);
	currPosition = actor->GetPosition();
	position[0] = currPosition[0];
	position[1] = currPosition[1];
	position[2] = currPosition[2];
}      	
/*
void vizObject::setRenderer(vtkRenderer *r,vtkRenderer *r2)
{
	ren =r; 
        //renx= r2;
	renx->AddActor(actor); 
	//ren->AddActor(actor); 
}
*/

void vizObject::setRenderer(char *Ren,char *Renx, Tcl_Interp *interp)
{
        //printf("ren=%s renx=%s\n",Ren,Renx);
        int err;
	ren = (vtkRenderer *) vtkTclGetPointerFromObject
		(Ren, "vtkRenderer", interp,err);
//        renx= (vtkRenderer *) vtkTclGetPointerFromObject
         //       (Renx, "vtkRenderer", interp);

        //renx->AddActor(actor);
        ren->AddActor(actor);

 
}

void vizObject::printConfiguration(void)
{
  int i;
  for(i=0;i<3;i++)
   printf("%f ",position[i]);
  for(i=0;i<3;i++)
   printf("%f ",orientation[i]);
  printf("\n");


}
goalPosition* vizObject::TransformActor(vtkMatrix4x4 *currentMatrix)
{
	/* must set position  and orientation and the user matrix and
	update the display */
	float *currPosition;
	float *currOrientation;
	vtkTransform *tempTransform=vtkTransform::New();
	goalPosition *currConfig;
	vtkMatrix4x4 *newMatrix=vtkMatrix4x4::New();
	actor->SetUserMatrix(currentMatrix);
	actor->GetMatrix(newMatrix);
	tempTransform->SetMatrix(*newMatrix);
	currPosition = tempTransform->GetPosition();
	currOrientation =tempTransform->GetOrientation();
/*
	printf("\n Actor's new position is  %f %f %f %f %f %f\n",
		currPosition[0],currPosition[1],currPosition[2],
		currOrientation[0],currOrientation[1],currOrientation[2]);
*/
       tempTransform->Delete();
       newMatrix->Delete();
	currConfig = new goalPosition( "noname",currPosition[0],
		currPosition[1],currPosition[2],currOrientation[0],
		currOrientation[1],currOrientation[2]);
	return currConfig;
}
void vizObject::setCollisionFlag(void)
{
  collisionFlag=1;
  actor->GetProperty()->SetColor(1-color[0],1-color[1],1-color[2]);
}
void vizObject::resetCollisionFlag(void)
{
  collisionFlag=0;
  actor->GetProperty()->SetColor(color[0],color[1],color[2]);
}

int vizObject::inCollision(void)
{
  return collisionFlag;
}


