//***************************************************************
// Vizmo 3D - A Visualization Tool for Motion Modelling
// Author: Renu Isaac
// Filename : vizEnvironment.C
// Handles all interaction for the vizEnvironment object, as well
// as all required wrapper code
//**************************************************************

#include <stdio.h>
#include <string.h>
#include <Input.h>
#include "Environment.h"
#include "Contact.h"
#include "FreeBody.h"
#include "FixedBody.h"
                       
#include <Cfg.h>
 
#include "CfgManager.h"
#include "Cfg_free.h"
#include "Cfg_fixed_PRR.h"
#include "Cfg_free_serial.h"
                                  
#include "vizEnvironment.h"
//#include "read_byu.h"
//#include "fk.h"
#include "BasicDefns.h"
#define LINEMAX 256



Input input;


vizEnvironment::vizEnvironment()
{
  return;
	/* constructor */
}

vizEnvironment::~vizEnvironment()
{
}

int vizEnvironment::Close(void)
{

	// clear objects pointed to 
	Actors.DeleteObjects();
	return Initialize();
}

int vizEnvironment::RandomizeColor(void)
{
        // Randomly set objects
        int i;
        for(i=0;i<noOfActors;i++)
        Actors[i]->setColor((float) drand48(),
			(float) drand48(),(float)drand48());
        return TCL_OK;
}

int vizEnvironment::Initialize(void)
{
	SelectedActorIndex = 0;
	noOfActors = 0;
	strcpy(DataFileName, "Untitled.env");
	pickedActor = NULL;
	return TCL_OK;

}
template <class T> bool vizReadField (istream &_is, T *fred) {

  char c;
  char ThrowAwayLine[LINEMAX];

  while ( _is.get(c) )
    if (c == '#') {
        _is.getline(ThrowAwayLine,LINEMAX,'\n');
          //cout << "Read == " << ThrowAwayLine << "\n";
        if( (ThrowAwayLine[0]=='V') &&
            (ThrowAwayLine[1]=='I') &&
            (ThrowAwayLine[2]=='Z') &&
            (ThrowAwayLine[3]=='M') &&
            (ThrowAwayLine[4]=='O') )
	     {  //cout << "Bulduk ya" << ThrowAwayLine << "\n"  ;
               sprintf(fred[0],"#%s",ThrowAwayLine); cout << "Bulduk ya" << fred[0]  << "\n"; return true; 
             }

    }
    
    else if (c != '\n') {
        _is.putback(c);
        if (_is >> *fred ) return true;
        else               return false;
    }
  return false;
}


int vizEnvironment::ReadFromFile(char *filename, char *path,char *Rend,char *XViewRend, Tcl_Interp *interp)
{
        int i ,j, tempi, invalidFile = FALSE;
        int solidFlag;
        char initString[1000];
        float color[3]={.5,.5,.5};
        char temps[1024];
        char string[255],objectName[255]="Noname";
        char objectFile[1024];
        vizObject *tempActor;
        noOfActors=0;
        printf ("\nvizEnvironment::ReadFromFile->begin %s %s\n",Rend,XViewRend);
        ifstream _is(filename);
/*
        if (!_is) {
           //cout << "Can't open \"" << filename << "\"." << endl;
                sendUserMessage(INVALID_FILE);
                return TCL_ERROR;
        }
        input.Read(_is,ENV_VER_LEGACY); */
        input.envFile.PutValue(filename);
        input.Read(RETURN);
        printf("Body =%d\n",input.multibodyCount);
        for(int m=0;m<input.multibodyCount;m++)
        {
           for(i=0;i<input.BodyCount[m]; i++){   
               noOfActors++;
               sprintf(objectName,"Noname");                                
               color[0]=color[1]=color[2]=0.9;
               solidFlag=1;      
               for(j=0;j<input.comments[m][i].size();j++)
               {
                   printf("Comment=%s \n",input.comments[m][i][j]);
                   if( !strncmp(input.comments[m][i][j],"VIZMO_COLOR",11))
                   {
                   sscanf(input.comments[m][i][j],"%s %f %f %f",temps,
                                        &color[0],&color[1],&color[2]);
                   cout << "Color " << color[0] << " " << color[1] << " " << color[2] << endl<< flush;
                   }     else 
                   if( !strncmp(input.comments[m][i][j],"VIZMO_NAME",10))
                   {
                    sscanf(input.comments[m][i][j],"%s %s",temps,objectName);
                    cout << "ObjectName " << objectName<< endl;
                   } else 
                   if( !strncmp(input.comments[m][i][j],"VIZMO_SOLID",11))
                   {
                      sscanf(input.comments[m][i][j],"%s %d",temps,&solidFlag);
                   }
                      
               }
               if(input.isFree[m][i]) {
                  strcpy(temps,input.freebodyFileName[m][i]);
                  cout << "Loading  FreeBody  " ;
                  linkCount=input.BodyCount[m];

                  
               } else {
                  strcpy(temps,input.fixedbodyFileName[m][i]);
                  cout << "Loading  FixedBody ";
                }
               if(  temps[0]=='/')
                    sprintf(objectFile,"%s",temps);
               else
                    sprintf(objectFile,"%s/%s",path,temps);
                  cout <<  objectFile << endl<<flush;
               if (!CheckValidFile(objectFile))
                {invalidFile = TRUE; break;}

               if (!invalidFile) {
 
                  tempActor = new vizObject(objectFile);
                  Actors.Insert(tempActor);
                  if(input.isFree[m][i])  {
                        tempActor->setRobot();
                        tempActor->part.conf[0]=input.freebodyPosition[m][0];    
                        tempActor->part.conf[1]=input.freebodyPosition[m][1];    
                        tempActor->part.conf[2]=input.freebodyPosition[m][2];    
                  }
                  else {
                        tempActor->setObstacle();
                        tempActor->part.conf[0]=input.fixedbodyPosition[m][i][0];
                        tempActor->part.conf[1]=input.fixedbodyPosition[m][i][1];
                        tempActor->part.conf[2]=input.fixedbodyPosition[m][i][2];

                        tempActor->part.conf[3]=input.bodyOrientation[m][i][0];  
                        tempActor->part.conf[4]=input.bodyOrientation[m][i][1]; 
                        tempActor->part.conf[5]=input.bodyOrientation[m][i][2];   
                  }
              
                  tempActor->setSolid();
    
   
              //_is >> tempActor->part.conf[5];
    
              tempActor->setPosition(tempActor->part.conf[0],
                  tempActor->part.conf[1],
                  tempActor->part.conf[2]);
               tempActor->setOrientation(tempActor->part.conf[3],
                                         tempActor->part.conf[4],
                                  tempActor->part.conf[5]);
               tempActor->setName(objectName);
    
    
               tempActor->setScaleFactor(1,1,1);
               tempActor->setColor(color[0],color[1],color[2]);
               tempActor->setRenderer(Rend,XViewRend, interp);
               tempActor->saveCurrentLocation();
               if(solidFlag) tempActor->setSolid();
                     else tempActor->setWired();
 
 
             }
                                  
                
            }
       }
        if (!invalidFile)
        {
                /* first element added */
                //cout << "salak burada" << endl;
                setPickedActorIndex(0);
                strncpy(DataFileName, filename, MAX_FILENAME-1);
        }
        else
        {
                Close();
                sendUserMessage(INVALID_FILE);
        }
        //printf("\nvizEnvironment::ReadFromFile-> ended" );
        if(!input.cfgSet) {
            cout << "LinkCount = " << linkCount << endl;
            if(linkCount>1)
            {
             Cfg::CfgHelper = new Cfg_free_serial(linkCount-1);
            }
            else
             Cfg::CfgHelper = new Cfg_free();
         }
         obprmEnv.Get(&input);
         cout << "Robot Index=" <<obprmEnv.GetRobotIndex() << endl << flush;
         cout << "Num links =" <<obprmEnv.GetMultiBody(obprmEnv.GetRobotIndex() )->GetFreeBodyCount() << endl << flush;
        return TCL_OK;
                           

}

int vizEnvironment::ReadFromModFile(char *modfilename, char *inifilename,
				char *dataDir,
				char *Rend,char *XViewRend, Tcl_Interp *interp)
{
	FILE *modFile;
	FILE *iniFile;
	float tempf[3];
   	char temps[MAX_FILENAME];
	char datafile[MAX_FILENAME];
	int i, invalidFile = FALSE;
	int noOfRobots,  robotIndex;
   	vizObject *tempActor;
   	//printf ("\nvizEnvironment::ReadFromMODFile->begin");
	if ( (modFile = fopen(modfilename, "r")) == NULL)
	{
		sendUserMessage(INVALID_FILE);
		return TCL_ERROR;
	}
	
   	fscanf(modFile, "%d\n", &noOfActors);
   	for (i = 0; (i < noOfActors) && (!invalidFile) ; i++)
   	{
		sprintf(datafile,"%s", dataDir);
   		fscanf(modFile, "%s\n", temps);
		strncat(datafile,temps,MAX_FILENAME-strlen(datafile)-1);
		if ( !CheckValidFile(datafile))
			invalidFile = TRUE;
		if (!invalidFile)
		{
      			tempActor = new vizObject(datafile);
			Actors.Insert(tempActor);
			tempActor->setName(temps);
      			fscanf(modFile, "%f %f %f ", &tempf[0],&tempf[1],&tempf[2] );
      			tempActor->setPosition(tempf[0], tempf[1], tempf[2]);
	      		fscanf(modFile, "%f %f %f\n", &tempf[0], &tempf[1],&tempf[2] );
      			tempActor->setOrientation(tempf[0], tempf[1], tempf[2]);
      			tempActor->setRenderer(Rend,XViewRend, interp);
			tempActor->saveCurrentLocation();
		}
   	}
   	
   	fclose(modFile);
	if (!invalidFile)
	{
		if ( (iniFile = fopen(inifilename, "r")) == NULL)
			invalidFile = TRUE;
		if (!invalidFile )
		{
   			fscanf(iniFile, "%d\n", &noOfRobots);
   			for (i = 0; i < noOfRobots; i++)
   			{
				fscanf(iniFile, "%d\n", &robotIndex);
				tempActor = Actors[robotIndex];
				tempActor->setRobot();
			}
			fclose(iniFile);
			strncpy(DataFileName, modfilename, MAX_FILENAME-1);
		}
   	}
	if (!invalidFile)
	{
		/* first element added */
   		setPickedActorIndex(0);
	} 
	else 
	{
		Close();
		sendUserMessage(INVALID_FILE);	
	}
	//printf("\nvizEnvironment::ReadFromModFile-> ended" );
   	return TCL_OK;
}
int vizEnvironment::SaveToModFile(char *modfilename, char *inifilename)
{
	FILE *modFile;
	FILE *iniFile;
	int noOfRobots, noOfObstacles, i;
   	vizObject *tempActor;
	char temp[MAX_FILENAME];
	char *loc, *start;
   	//printf ("\nvizEnvironment::SaveToMODFile->begin");
	if ((modFile = fopen(modfilename, "w")) == NULL)
   	{
		sendUserMessage(NO_WRITE_PERMISSION);
		return TCL_ERROR;
	}
	fprintf(modFile, "%d\n", noOfActors);
   	for (i = 0; i < noOfActors; i++)
   	{
   		tempActor = Actors[i];
		strcpy(temp, tempActor->getBYUFilename());
		loc = temp; 
		start = temp;
		while ((loc = strpbrk(start, "/")) != NULL)
		{ 
			/*printf ("\n new string is %s", loc);*/
			start = ++loc;
		  	/*printf ("\n new start is %s", start);	*/
		}
		fprintf(modFile, "%s\n",start);
      		fprintf(modFile, "%f %f %f ", tempActor->getPosition(0),
			tempActor->getPosition(1), 
			tempActor->getPosition(2));
      		fprintf(modFile, "%f %f %f\n", tempActor->getOrientation(0),
				tempActor->getOrientation(1),
				tempActor->getOrientation(2));
		/* backup this location for path file display later */
		tempActor->saveCurrentLocation();

   	}
   	fclose(modFile);

	iniFile = fopen(inifilename, "w");
	noOfRobots = 0;
	noOfObstacles = 0;
	/* need to count the robots */
	for (i = 0; i < noOfActors; i++)
   		if (Actors[i]->isRobot())
			noOfRobots++;
		else 
			noOfObstacles++;
   	fprintf(iniFile, "%d\n", noOfRobots);
   	for (i = 0; i < noOfActors; i++)
   	{
		if (Actors[i]->isRobot() == TRUE)
		{
			tempActor = Actors[i];
			fprintf(iniFile, "%d\n", i);
		}
	}
	fprintf(iniFile, "%d\n", noOfObstacles);
   	for (i = 0; i < noOfActors; i++)
   	{
		if (Actors[i]->isRobot() == FALSE)
		{
			tempActor = Actors[i];
			fprintf(iniFile, "%d\n", i);
		}
	}
	fclose(iniFile);
   	//printf("\nvizEnvironment::WriteToModFile -> ended" );
   	return TCL_OK;
}

int vizEnvironment::SaveToFile(char *filename)
{
	FILE *envFile;
   	int i;
   	char temps[255];
  	 vizObject *tempActor;

	if ( (envFile = fopen(filename, "w")) == NULL)
	{
		sendUserMessage(NO_WRITE_PERMISSION);
		return TCL_ERROR;
	}

   	fprintf(envFile, "%d\n\n", noOfActors);
   	for (i = 0; i < noOfActors; i++)
   	{
   		tempActor = Actors[i];
                if(tempActor->isRobot())
                  fprintf(envFile, "MultiBody Active\n1\n");
                else
                  fprintf(envFile, "MultiBody Pasive\n1\n");

      		fprintf(envFile, "#VIZMO_COLOR %f %f %f\n",
			tempActor->getColor(0),
         		tempActor->getColor(1),
         		tempActor->getColor(2) );
                          

      		strcpy(temps,tempActor->getName());
      		fprintf(envFile, "#VIZMO_NAME %s\n", temps);
                fprintf(envFile, "#VIZMO_SOLID %d\n",tempActor->isSolid());
                if(tempActor->isRobot()) fprintf(envFile, "FreeBody 0  ");
                else fprintf(envFile, "FixedBody 0  ");
   		strcpy(temps,tempActor->getBYUFilename());
   		fprintf(envFile, "%s ", temps);
      		strcpy(temps,tempActor->getName());
      		fprintf(envFile, "%f %f %f ", tempActor->getPosition(0),
      			tempActor->getPosition(1),
               		tempActor->getPosition(2));
      		fprintf(envFile, "%f %f %f\n", tempActor->getOrientation(0),
      		 	tempActor->getOrientation(1),
             		tempActor->getOrientation(2));
                fprintf(envFile, "Connection\n0\n\n\n");
   	}
   	fclose(envFile);
	strncpy(DataFileName, filename, MAX_FILENAME-1);
	return TCL_OK;
}
int vizEnvironment::AddActor(char *filename, char * Rend,char *XViewRend,  Tcl_Interp *interp)
{
	vizObject* tempActor;
	//printf("\n vizEnvironment::AddActor -> ");
	//this function initializes the actor and also creates
	//the actor to be displayed in the render window
	if (!CheckValidFile(filename))
	{
		sendUserMessage(INVALID_FILE);
		return TCL_OK;
	}
	tempActor = new vizObject(filename);
	Actors.Insert(tempActor);
	tempActor->setRenderer(Rend,XViewRend, interp);
	//printf("\n vizEnvironment::AddActor -> actor inserted ");
	noOfActors++;
	if ( noOfActors == 1)
	{
		/* first element added */
		setPickedActorIndex(0);
	}
	return TCL_OK;
}
int vizEnvironment::DeleteActor()
{
	//printf("\n vizEnvironment::DeleteActor -> at pos %d ", SelectedActorIndex);
	vizObject* tempActor = Actors[SelectedActorIndex];
	Actors.Remove(SelectedActorIndex);
	delete tempActor;
	noOfActors--;
	if (SelectedActorIndex > 0)
		SelectedActorIndex--;
	pickedActor = Actors[SelectedActorIndex];
	return TCL_OK;
}


int vizEnvironment::PickActor(char *ptrActor,Tcl_Interp *interp)
{
	int i;
   vtkActor *myActor ;
  int err;
   myActor = (vtkActor *) vtkTclGetPointerFromObject
		(ptrActor, "vtkActor", interp,err);
	for ( i = 0; i < Actors.GetCount() ;i++)
   {
   	if (Actors[i]->getActor() == myActor)
          return setPickedActorIndex(i);
   }
   return SelectedActorIndex;
}
/************************************************************/
// WRAPPER CODE 
/************************************************************/


int vizEnvironment::setPickedActorIndex(int index)
{
	SelectedActorIndex = index;
	//printf("Selected actor is %d", SelectedActorIndex);
	pickedActor = Actors[SelectedActorIndex];
	return SelectedActorIndex;
} 

int vizEnvironment::getPickedActorIndex(void)
{
	return SelectedActorIndex;
} 

char* vizEnvironment::getActorName(int i)
{
	if (i >= 0 && i < noOfActors)
	{
		/* return the name of actor at index i*/
		return Actors[i]->getName();
	} else 
		return NULL;
}

char* vizEnvironment::getActorFileName(int i)
{
	if (i >= 0 && i < noOfActors)
	{
		/* return the name of actor at index i*/
		return Actors[i]->getBYUFilename();
	} else 
		return NULL;
}

int vizEnvironment::getNumberActors()
{
	return noOfActors;
}

int vizEnvironment::getNumberRobots()
{
	int i, sumRobots = 0;
	for (i=0; i<noOfActors; i++)
	{
		if (Actors[i]->isRobot())
			sumRobots++;
	} 
	return sumRobots;
}
float vizEnvironment::getOrientationValue( int i, int j)
{
	if (pickedActor != NULL)
	{
		switch (i)
		{
			case 0:return pickedActor->getPosition(j);

			case 1:return pickedActor->getOrientation(j);

			case 2:return pickedActor->getScaleFactor(j);

		}
	}
	return 0.0;
}
int vizEnvironment::setOrientationValue( int conftype, float a, float b, float c)
{
	if (pickedActor != NULL)
	{
		switch (conftype)
		{
			case 0:pickedActor->addPosition(a,b,c);
				break;
			case 1:	printf("Current Orientation is %f %f %f ", 
				pickedActor->getOrientation(0), 
				pickedActor->getOrientation(1),
				pickedActor->getOrientation(2));
				pickedActor->addOrientation(a,b,c);
				break;
		}
	}
	return TCL_OK;
}

int vizEnvironment::getIsRobot()
{
	if (pickedActor != NULL)
	{	
		/* current status of picked actor */
		return pickedActor->isRobot();
	}
	return FALSE;
}
int vizEnvironment::getIsSolid()
{
        if (pickedActor != NULL)
        {
                /* current status of picked actor */
                return pickedActor->isSolid();
        }
        return FALSE;
}


int vizEnvironment::getCurrentColor(int hue)
{
         //printf("returning the color\n");
	if (pickedActor != NULL)
	{
		/* return the picked actors current color */
		return pickedActor->getColor(hue) * 65535;
       
	}
	return 0;
}

char *vizEnvironment::getDataFileName()
{
	return (char *)DataFileName;
}
void  vizEnvironment::ActorPropertiesDialogOnModifyClicked( int isRobot,
	int isSolid,
	float r, float g, float b, 
	float posx, float posy, float posz, 
	float orx, float ory, float orz, 
	float sfx, float sfy, float sfz)
{
/*
	printf("\n Modifying values %d ,%f %f %f,%f %f %f,%f %f %f,%f %f %f\n",
		isRobot, r,  g,  b,  posx, posy, posz,  orx,  ory,  orz,
		sfx,  sfy,  sfz);
*/

	if (pickedActor != NULL)
	{
		/* transfer all values to main environment */
		if (isRobot)
			pickedActor->setRobot();
		else
			pickedActor->setObstacle();
                if(isSolid) pickedActor->setSolid();
                else pickedActor->setWired();
 
		/* save as value b/w 0 and 1 */
		pickedActor->setColor(r/65535.0, g/65535.0, b/65535.0);
		pickedActor->setPosition(posx,posy,posz);	
		pickedActor->setOrientation(orx,ory,orz);
		pickedActor->setScaleFactor(sfx, sfy, sfz);
	}
//        vtkMatrix4x4 *t=vtkMatrix::New();
 //            pickedActor->getActor()->GetMatrix(t);
        //cout << "Actor's matrix=\n" << t<< flush;
}

void  vizEnvironment::ActorPropertiesDialogOnCloseClicked(void)
{
	/* do nothing for now */
}
int vizEnvironment::ReadRoadmap(char *file,int type)
// type is not used for now
{
   FILE *in;
   int number;
   int t1,t2;
   char line[256];
   //GraphNode node;
   int i,j;
   
   char tagstring[100];

   rdmp.ReadRoadmap(file);
   //rdmp.DisplayCCStats();
   cout << "Env. reading roadmap\n";
   rdmp.CreateCCEdges();
   cout << "Env. read roadmap\n";
 

   return(TCL_OK);
}

 
 
int vizEnvironment::DisplayRoadmap(char *file,int type,char *Rend,Tcl_Interp *interp)
{
   int i;
   vtkRenderer *ren;
   int err;
   //Tcl_Obj *resultPtr
   cout << "In Env: Display\n";
   rdmp.Display(file,Rend,interp);
   return TCL_OK;
 
}
 
int vizEnvironment::HideRoadmap(char *Rend,Tcl_Interp *interp)
{
   vtkRenderer *ren;
   int i;
   int err;
   return(rdmp.Hide(Rend,interp));    
}
                                 

