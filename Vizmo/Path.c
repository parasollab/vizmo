//***************************************************************
// Vizmo 3D - A Visualization Tool for Motion Modelling
// Author: Renu Isaac
// Filename : path.h
// Purpose : definition of class which handles the path loading
// 	and display.
//**************************************************************

// vtk includes
// #include "vtk.h"

// standard definitions/includes for all files
#include "vizmo.h"
#include <string.h>
#include <stdio.h>
#include "Path.h"
#include "OBPRM.h"
#include "util.h"
//#include "collision_check.h"

typedef double t44[4][4];                                                       

void
RPY_Config_To_Transform(double Q[6], t44 T) {
 
  double a,b,c,tmp1,tmp2;
  double ca,cb,cc,sa,sb,sc;
 
  c=Q[3]*TWOPI;
  b=Q[4]*TWOPI;
  a=Q[5]*TWOPI;
 
  ca=cos(a);
  cb=cos(b);
  cc=cos(c);
  sa=sin(a);
  sb=sin(b);
  sc=sin(c);
 
  tmp1=ca*sb;
  tmp2=sa*sb;
 
  T[0][0] = ca*cb;
  T[0][1] = tmp1*sc - sa*cc;
  T[0][2] = tmp1*cc + sa*sc;
  T[1][0] = sa*cb;
  T[1][1] = tmp2*sc + ca*cc;
  T[1][2] = tmp2*cc - ca*sc;
  T[2][0] = -sb;
  T[2][1] = cb*sc;
  T[2][2] = cb*cc;
  T[0][3] = Q[0];
  T[1][3] = Q[1];
  T[2][3] = Q[2];
  T[3][0] = T[3][1] = T[3][2] = 0.0;
  T[3][3] = 1.0;
}
                         
void
RPY_Config_To_Transform(double Q[6], double T[12]) {
 
    t44 Tmp;
    RPY_Config_To_Transform(Q, Tmp);
 
    T[0] = Tmp[0][0]; T[1] = Tmp[0][1]; T[2] = Tmp[0][2];  T[3] = Q[0];
    T[4] = Tmp[1][0]; T[5] = Tmp[1][1]; T[6] = Tmp[1][2];  T[7] = Q[1];
    T[8] = Tmp[2][0]; T[9] = Tmp[2][1]; T[10] = Tmp[2][2]; T[11] = Q[2];
}   
Path::Path()
{
        noOfFrames = 0;
   	currentFrame = 0;
   	noTryConnect = 100;
   	noOfRobots = 0;
   	playDirection = FORWARD;
   	theEnv = NULL;
        strcpy( pathFileName, "Untitled.path");
  
}

Path::~Path(void)
{
	if (Configurations.GetCount() > 0)
   		Configurations.DeleteObjects();
}

int Path::Initialize(vizEnvironment *env)
{
   noOfFrames = 0;
   currentFrame = 0;
   noTryConnect = 100;
   noOfRobots = 0;
   playDirection = FORWARD;
   theEnv = env;
   	
   strcpy( pathFileName, "Untitled.path");
   strcpy( modFileName, "Untitled.mod");
   strcpy( initFileName, "Untitled.init");
   strcpy( mapFileName, "Untitled.map");
   strcpy( cfgFileName, "Untitled.cfg");
   if (Configurations.GetCount() > 0)
   	Configurations.DeleteObjects();
   return TCL_OK;
}

int Path::Close()
{
   //printf("\nPath::Close -> started");	
   currentFrame = 0;
   if (Configurations.GetCount() > 0)
   {
	/* reset positions */
   	GoToStep(currentFrame);
   	Configurations.DeleteObjects();
   }
   noTryConnect = 100;
   noOfFrames = 0;
   noOfRobots = 0;
   theEnv = NULL;
   playDirection = FORWARD;
   
   
   strcpy( pathFileName, "Untitled.path");
   strcpy( modFileName, "Untitled.mod");
   strcpy( initFileName, "Untitled.init");
   strcpy( mapFileName, "Untitled.map");
   strcpy( cfgFileName, "Untitled.cfg");
   
  
   return TCL_OK;
}

int Path::ReadFromFile(char *filename)
{
	/* load the path from the specified file */
	FILE *pathFile;
	float tempf[3];
	float conf[6];
	goalPosition *tempConfig;
        char line[256];
	vtkMatrix4x4 *tempMatrix;
	int i,j,row,col, actualNumRobots;
	char buffer[MAX_ANY_NAME];
	float *startConfig;
        vector <Vector6D> linkPos;
        Cfg cfg;
        int  version;
         char tmpLine[256];

	//printf ("\n Path::ReadFromFile->begin");
	if (( pathFile = fopen(filename, "r")) == NULL )
	{
		sendUserMessage(INVALID_FILE);
		return TCL_OK;
	}
	strcpy( pathFileName, filename);
	noOfFrames = 1;
	noOfRobots = 1;
	/*should ideally read noOfRobots */
	/*fscanf(pathFile, "%d\n", &noOfRobots);*/
	fgets(buffer,MAX_ANY_NAME-1,pathFile );
        if( !strncmp(buffer,"VIZMO_PATH_FILE",10))
        {
          //printf("Using Vizmo format \n");
          fscanf(pathFile,"%d",&noOfRobots);
          noOfRobots=theEnv->obprmEnv.GetMultiBody(
                          theEnv->obprmEnv.GetRobotIndex())->GetFreeBodyCount();
          actualNumRobots = 0;
          for( i = 0; i < theEnv->getNumberActors(); i++)
          {
                if ((theEnv->Actors[i])->isRobot() == TRUE)
                {
                        robotIndex[actualNumRobots++] = i;
                        /* move robots back to where they were in the mod file */
                        startConfig = theEnv->Actors[i]->getLastSavedLocation();
                        theEnv->Actors[i]->setPosition(startConfig[0],
				startConfig[1],startConfig[2]);
                        theEnv->Actors[i]->setOrientation(startConfig[3],
				startConfig[4],startConfig[5]);
                }
         }
        sscanf(buffer,"%s %s  %s %d",tmpLine,tmpLine,tmpLine,&version);
        if(version==20001125) {
          fclose(pathFile);
            ifstream _is(filename);        
              _is.getline(line,256,'\n'); // read header
              _is.getline(line,256,'\n'); // read number of robots
              _is >> noOfFrames; 
        cout << "NumFrames = " << noOfFrames << endl;
         cout << "Num Links= " << theEnv->obprmEnv.GetMultiBody(0)->GetFreeBodyCount() << endl;               
              for ( i = 0; i < noOfFrames; i++)
              {
                 cfg.Read(_is); 
                 cfg.printLinkConfigurations(&(theEnv->obprmEnv),linkPos);
                 for(j=0;j<linkPos.size();j++)
                      printf("%lf %lf %lf %lf %lf %lf\n",
                                linkPos[j][0],linkPos[j][1],linkPos[j][2],
                                linkPos[j][3],linkPos[j][4],linkPos[j][5]);

                 for(j=0;j<linkPos.size();j++) {
                    tempConfig=new goalPosition("noname",
                           linkPos[j][0],linkPos[j][1],linkPos[j][2],
                           linkPos[j][3]*360,linkPos[j][4]*360,
                           linkPos[j][5]*360);
                    Configurations.Insert(tempConfig);
                 }
              }
              cout << "Finished\n";
            _is.close();
            return TCL_OK;
           } 
        else {
           if ( actualNumRobots != noOfRobots)
           {
                   printf("\nInconsistent path file - incorrect number robots");
                return 0;
           }
           /* reads two lines of junk if a PV path file */
           //printf("\n Reading configurations from file :");

           /*should read noOfFrames */
           fscanf(pathFile, "%d", &noOfFrames);
           //printf("No# frames =%d\n", noOfFrames);
           for ( i = 0; i < noOfFrames; i++)
           {
                   for ( j = 0; j < noOfRobots; j++)
                   {
                           fscanf(pathFile, "%f %f %f %f %f %f\n", &conf[0],
				   &conf[1],&conf[2],&conf[3],&conf[4],&conf[5]);
                           //printf( "%f %f %f %f %f %f\n", conf[0],
			   //	conf[1],conf[2],conf[3],conf[4],conf[5]);
   /*
                             conf[3]=conf[3]*180/M_PI;
                             conf[4]=conf[4]*180/M_PI;
                             conf[5]=conf[5]*180/M_PI;
   */
                            conf[3]=conf[3]*360;
                             conf[4]=conf[4]*360;
                             conf[5]=conf[5]*360;
                            tempConfig=new goalPosition( "noname",
				   conf[0],conf[1],conf[2],conf[3],conf[4],
 				   conf[5]);
   
                           Configurations.Insert(tempConfig);
   
                           /* read in the three zeros!!! PV quirk*/
                   }
   
           }
           fclose(pathFile);
         }
        /* reset to start position */
        currentFrame = 0;
        GoToStep(currentFrame);
        //printf ("\n Path::ReadFromFile->ended");
        return TCL_OK;


        }
        if( strncmp(buffer,"LYMB_TRANSFORMS_FILE",10))
        {
          sscanf(line,"%d",&noOfRobots);
          //printf("new version\n");
	  fgets(buffer,MAX_ANY_NAME-1,pathFile );

        }
	actualNumRobots = 0;
	for( i = 0; i < theEnv->getNumberActors(); i++)
	{
		if ((theEnv->Actors[i])->isRobot() == TRUE)
		{
			robotIndex[actualNumRobots++] = i;
			/* move robots back to where they were in the mod file */
			startConfig = theEnv->Actors[i]->getLastSavedLocation();
			theEnv->Actors[i]->setPosition(startConfig[0],startConfig[1],startConfig[2]);
			theEnv->Actors[i]->setOrientation(startConfig[3],startConfig[4],startConfig[5]);
		}
	}
	if ( actualNumRobots != noOfRobots)
	{
		printf("\nInconsistent path file - incorrect number robots");
		return 0;
	}

	/* reads two lines of junk if a PV path file */
	fgets(buffer,MAX_ANY_NAME-1,pathFile );
	//printf(buffer);
	
	tempMatrix = vtkMatrix4x4::New();
	//printf("\n Reading matrices from file :");

	/*should read noOfFrames */
	fscanf(pathFile, "%d\n", &noOfFrames);
	//printf("No# frames =%d\n", noOfFrames);
	for ( i = 0; i < noOfFrames; i++)
	{
		for ( j = 0; j < noOfRobots; j++)
		{
			for (col = 0 ; col < 4; col++)
			{
				for (row = 0; row < 4; row++)
				{
					/*read each matrix in */
					fscanf(pathFile, "%f ", &tempf[0]);
					//printf("%f ", tempf[0]);
					tempMatrix->SetElement(row, col, tempf[0]); 
				}
				//printf("\n");
			}
			fscanf(pathFile,"\n");
			//printf("\n");
			tempConfig = theEnv->Actors[robotIndex[j]]->TransformActor(tempMatrix);
			
			Configurations.Insert(tempConfig);
	
			/* read in the three zeros!!! PV quirk*/
			fscanf(pathFile, "%f %f %f\n", 
				&tempf[0],&tempf[1],&tempf[2]);
		}	

	} 
	tempMatrix->Delete();;
	fclose(pathFile);
	/* reset to start position */
	currentFrame = 0;
	GoToStep(currentFrame);
	//printf ("\n Path::ReadFromFile->ended");
	return TCL_OK;
}

/* this may not be needed if paths can only be edited by OBPRM*/
int Path::SaveToFile(char *filename)
{
    strcpy( pathFileName, filename);
    return TCL_OK;
}


int Path::getCurrentFrame(void) const
{
	return currentFrame;
}
int Path::setCurrentFrame(int frameNo)
{
	if ((frameNo >= 0) && (frameNo < noOfFrames))
   	{
		currentFrame = frameNo;
      		return TCL_OK;
   	}
   	else
   		return TCL_ERROR;
}
int Path::getNoOfFrames(void) const
{
	return noOfFrames;
}

int Path::setNoOfFrames(int noFrames)
{
	if ( noFrames >= 0 )
	{
   		noOfFrames = noFrames;
      		return TCL_OK;
   	}
   	else
   		return TCL_ERROR;
}
int Path::getPlayDirection(void) const 
{
	return playDirection;
}
int Path::setPlayDirection(int direction)
{
	playDirection = direction;
   	return TCL_OK;
}

void Path::collision(int index)
{
  int i;
  return ; 
  /*
  Config aa;
  aa.conf[0]=theEnv->Actors[robotIndex[index]]->getPosition(0);
  aa.conf[1]=theEnv->Actors[robotIndex[index]]->getPosition(1);
  aa.conf[2]=theEnv->Actors[robotIndex[index]]->getPosition(2);
  aa.conf[3]=theEnv->Actors[robotIndex[index]]->getOrientation(0);
  aa.conf[4]=theEnv->Actors[robotIndex[index]]->getOrientation(1);
  aa.conf[5]=theEnv->Actors[robotIndex[index]]->getOrientation(2);
  for(i=0;i<theEnv->noOfActors;i++)
  {
   if(i==robotIndex[index]) continue;
   theEnv->Actors[i]->printConfiguration();
   if(collision_check(&theEnv->Actors[robotIndex[index]]->part,
			&theEnv->Actors[i]->part, &aa))
   {
        theEnv->Actors[i]->setCollisionFlag();
	theEnv->Actors[robotIndex[index]]->setCollisionFlag();
        //printf("collision  \7");
   }
   else   {
        theEnv->Actors[i]->resetCollisionFlag();
        theEnv->Actors[robotIndex[index]]->resetCollisionFlag();
   }

  } */
}
int Path::GoToStep(int index)
{
	goalPosition* currConfig;
	int startIndex, i;
	vtkMatrix4x4 *identity = vtkMatrix4x4::New();
//  	printf("\n Path::GoToStep %d", index);
        t44 salak;
    
       vtkMatrix4x4 *t=vtkMatrix4x4::New();
       if(index<0) index=0;
       if(index>(noOfFrames-1)) index=noOfFrames-1;

        
	if ( (index>=0) && (index < noOfFrames)) 
	{
		currentFrame = index;
		startIndex = currentFrame * noOfRobots;
		for ( i=0; i < noOfRobots; i++)
		{
			theEnv->Actors[robotIndex[i]]->getActor()->SetUserMatrix(identity);
			currConfig = Configurations[startIndex+i];
			//printf("\n Robot[%d]GoToStep %f %f %f %f %f %f\n", i, currConfig->pos[0],
			//				currConfig->pos[1],
			//				currConfig->pos[2],
			//				currConfig->pos[3],
			//				currConfig->pos[4],
			//				currConfig->pos[5]); 
			theEnv->Actors[robotIndex[i]]->setOrientation(currConfig->pos[3],
							currConfig->pos[4],
							currConfig->pos[5]);	
			theEnv->Actors[robotIndex[i]]->setPosition(currConfig->pos[0], currConfig->pos[1],
							currConfig->pos[2]);
			/*
			theEnv->Actors[robotIndex[i]]->setOrientation(0,0,0);
			theEnv->Actors[robotIndex[i]]->setPosition(0,0,0);
			theEnv->Actors[robotIndex[i]]->getActor()->SetUserMatrix(identity);
			
*/
                       theEnv->Actors[robotIndex[i]]->getActor()->GetMatrix(t); 
                 //cout << "user matrix \n" << t << "\n" << flush;
                        double z[12];
                        double tz[6];
			tz[0]=currConfig->pos[0];
			tz[1]=currConfig->pos[1];
			tz[2]=currConfig->pos[2];
			tz[3]=currConfig->pos[3]/360.0;
			tz[4]=currConfig->pos[4]/360.0;
			tz[5]=currConfig->pos[5]/360.0;
                        RPY_Config_To_Transform(tz,salak);
                        int l,m;
                        /*for(l=0;l<4;l++) for(m=0;m<4;m++)
                          t->SetElement(l,m,salak[l][m]);
theEnv->Actors[robotIndex[i]]->getActor()->SetUserMatrix(t); */
                        RPY_Config_To_Transform(tz,z);
                        
                        //printf("Matrix z=\n %f %f %f %f \n %f %f %f %f \n %f %f %f %f\n",z[0],z[1],z[2],z[3],z[4],z[5],z[6],z[7],z[8],z[9],z[10],z[11]);
               collision(i);
                        
		}
	}	
        t->Delete();
    
	return TCL_OK;
}

char *Path::getPathFileName()
{
	return (char *)pathFileName;
}

int Path::setPathFileName(char *newName)
{
	strncpy(pathFileName,newName, MAX_FILENAME-1);
	return 0;
}


int Path::ShowSwapVolume(char *RendererName,Tcl_Interp *interp)
{
 int i,j,k=0;
  vtkRenderer *ren;
  int err;
  //cout << "Getting render" << RendererName <<flush;
  ren = (vtkRenderer *) vtkTclGetPointerFromObject
                (RendererName, "vtkRenderer", interp,err);

 for(i=0;i<noOfFrames;i++)
   for(j=0;j<noOfRobots;j++)
   {
  //cout << "Adding " << k << endl <<flush;
    vtkActor *act=vtkActor::New();
      goalPosition* currConfig = Configurations[k++];
     
         act->RotateZ(currConfig->pos[5]);
         act->RotateY(currConfig->pos[4]);
         act->RotateX(currConfig->pos[3]);

       act->SetPosition(currConfig->pos[0], currConfig->pos[1],
                                                        currConfig->pos[2]);
       act->SetMapper(theEnv->Actors[robotIndex[j]]->actor->GetMapper());
  //cout << "Getting adding" << endl<<flush;
       //cout << ren << endl << flush;
       act->GetProperty()->SetOpacity(0.5);
       ren->AddActor(act);
       SwapVolumeActors.push_back(act);
    }
 return 1;
}
int Path::HideSwapVolume(char *RendererName,Tcl_Interp *interp)
{
  int i;
  vtkRenderer *ren;
  int err;
  ren = (vtkRenderer *) vtkTclGetPointerFromObject
                (RendererName, "vtkRenderer", interp,err);

  //cout << "Removing " << ren << endl << flush;
  for(i=0;i<SwapVolumeActors.size();i++)
  {
    ren->RemoveActor(SwapVolumeActors[i]);
     SwapVolumeActors[i]->Delete();

  }
  for(i=0;i<SwapVolumeActors.size();i++)
  {
    SwapVolumeActors.pop_back();
  }
  return 0;
}
