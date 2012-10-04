// vizmo2.cpp: implementation of the vizmo class.
//
//////////////////////////////////////////////////////////////////////

#include "vizmo2.h"
#include <limits.h>

#include <ctype.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;
//////////////////////////////////////////////////////////////////////
// Include Plum headers
#include <MapObj/Cfg.h>
#include <MapObj/MapLoader.h>
#include <EnvObj/MovieBYULoader.h>
#include <PlumObject.h>
#include <PlumUtil.h>

#include "../Plum/GLModel.h"

using namespace plum;

//////////////////////////////////////////////////////////////////////
// Include OBPRMViewer headers
#include "EnvObj/PathLoader.h"
#include "EnvObj/PathModel.h"
#include "EnvObj/DebugLoader.h"
#include "EnvObj/DebugModel.h"
#include "EnvObj/BoundingBoxesModel.h"
#include "EnvObj/BoundingBoxParser.h"
#include "EnvObj/Robot.h"
#include "EnvObj/QueryLoader.h"
#include "EnvObj/QueryModel.h"



//////////////////////////////////////////////////////////////////////
//Define Camera singleton
gliCamera* return_camera;
gliCamera* GetCamera(){ return return_camera; }
void SetCamera(gliCamera* s_camera){return_camera = s_camera;}
//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
//Define Vizmo singleton
vizmo g_vizmo2;
vizmo& GetVizmo(){ return g_vizmo2; }
//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
//Define Robot function
OBPRMView_Robot* GetRobot(){return (OBPRMView_Robot*)(GetVizmo().GetRobot()->getModel());}
//////////////////////////////////////////////////////////////////////


#ifdef USE_PHANTOM
//////////////////////////////////////////////////////////////////////
//Define Phantom Manager singleton
PhantomManager g_phantomManager;
PhantomManager& GetPhantomManager(){ return g_phantomManager; }
//////////////////////////////////////////////////////////////////////




// PhantomManager
void exitHandler()
{
   hdStopScheduler();
   hdUnschedule(gSchedulerCallback);

   // Free the haptics device
   if (ghHD != HD_INVALID_HANDLE)
   {
      hdDisableDevice(ghHD);
      ghHD = HD_INVALID_HANDLE;
   }
}

/*******************************************************************************
  Client callback.
  Use this callback synchronously.
  Gets data, in a thread safe manner, that is constantly being modified by the 
  haptics thread. 
 *******************************************************************************/
HDCallbackCode HDCALLBACK DeviceStateCallback(void *pUserData)
{
   DeviceDisplayState *pDisplayState =
      static_cast<DeviceDisplayState *>(pUserData);

   hdGetDoublev(HD_CURRENT_POSITION, pDisplayState->position);
   hdGetDoublev(HD_CURRENT_FORCE, pDisplayState->force);
   hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, pDisplayState->rotation);
   hdGetDoublev(HD_CURRENT_VELOCITY, pDisplayState->velocity);

   // execute this only once.
   return HD_CALLBACK_DONE;
}

/*******************************************************************************
  Main callback that calculates and sets the force.
 *******************************************************************************/
HDCallbackCode HDCALLBACK MainCallback(void *data)
{
   HHD hHD = hdGetCurrentDevice();

   hdBeginFrame(hHD);


   hduVector3Dd pos;
   hdGetDoublev(HD_CURRENT_POSITION,pos);

   hduVector3Dd forceVeci;
   forceVeci[0] = 0;
   forceVeci[1] = 0;
   forceVeci[2] = 0;
   hdSetDoublev(HD_CURRENT_FORCE, forceVeci);

   if(GetPhantomManager().CDOn){
      if(GetPhantomManager().fpos.size() > 0){
         {
            if(GetPhantomManager().Collision > .5 && GetPhantomManager().validpos.size() > 0){

               hduVector3Dd forceVec;
               forceVec[0] = GetPhantomManager().phantomforce*(GetPhantomManager().validpos[0] - GetPhantomManager().fpos[0]);
               forceVec[1] = GetPhantomManager().phantomforce*(GetPhantomManager().validpos[1] - GetPhantomManager().fpos[1]);
               forceVec[2] = GetPhantomManager().phantomforce*(GetPhantomManager().validpos[2] - GetPhantomManager().fpos[2]);
               GetCamera()->ReverseTransform(forceVec[0],forceVec[1],forceVec[2]);
               double R = pow(pow(forceVec[0],2)+pow(forceVec[1],2)+pow(forceVec[2],2),.5);
               if(R < 1)
                  hdSetDoublev(HD_CURRENT_FORCE, forceVec);
               else{
                  forceVec[0]/=R;forceVec[1]/=R;forceVec[2]/=R;
                  hdSetDoublev(HD_CURRENT_FORCE, forceVec);
               }

            }


            if(fabs(GetPhantomManager().Collision) < .5){

               GetPhantomManager().validpos = GetPhantomManager().fpos;
            }
            GetPhantomManager().proceed = false;
         }
      }
   }

   hdEndFrame(hHD);

   HDErrorInfo error;
   if (HD_DEVICE_ERROR(error = hdGetError()))
   {
      hduPrintError(stderr, &error, "Error during scheduler callback");
      if (hduIsSchedulerError(&error))
      {
         return HD_CALLBACK_DONE;
      }
   }

   return HD_CALLBACK_CONTINUE;
}

/*******************************************************************************
  Schedules the force callback.
 *******************************************************************************/
void PhantomManager::ScheduleForceCallback()
{
   std::cout << "haptics callback" << std::endl;
   gSchedulerCallback = hdScheduleAsynchronous(
         MainCallback, 0, HD_DEFAULT_SCHEDULER_PRIORITY);

   HDErrorInfo error;
   if (HD_DEVICE_ERROR(error = hdGetError()))
   {
      hduPrintError(stderr, &error, "Failed to initialize haptic device");
      fprintf(stderr, "\nPress any key to quit.\n");
      getchar();
      exit(-1);
   }

}

/*********************************************************************************
  Constructor
 *********************************************************************************/
PhantomManager::PhantomManager(){
   CDOn = false;
   proceed = false;
   phantomforce = .5;   
   Collision = -1;
   UseFeedback = true;
   initPhantom();
}

/*********************************************************************************
  Initialize Phantom Device
 *********************************************************************************/
void PhantomManager::initPhantom(){


   HDErrorInfo error;

   printf("initializing Phantom\n");

   atexit(exitHandler);

   // Initialize the device.  This needs to be called before any other
   // actions on the device are performed.
   ghHD = hdInitDevice(HD_DEFAULT_DEVICE);
   if (HD_DEVICE_ERROR(error = hdGetError()))
   {
      hduPrintError(stderr, &error, "Failed to initialize haptic device");
      fprintf(stderr, "\nPress any key to quit.\n");
      getchar();
      exit(-1);
   }

   printf("Found device %s\n",hdGetString(HD_DEVICE_MODEL_TYPE));

   // Create a haptic context for the device.  The haptic context maintains 
   // the state that persists between frame intervals and is used for
   // haptic rendering.
   //ghHLRC = hlCreateContext(ghHD);
   //hlMakeCurrent(ghHLRC);

   hdEnable(HD_FORCE_OUTPUT);
   hdEnable(HD_MAX_FORCE_CLAMPING);

   hdStartScheduler();
   if (HD_DEVICE_ERROR(error = hdGetError()))
   {
      hduPrintError(stderr, &error, "Failed to start scheduler");
      fprintf(stderr, "\nPress any key to quit.\n");
      getchar();
      exit(-1);
   }


   // Get the workspace dimensions.
   HDdouble maxWorkspace[6];
   hdGetDoublev(HD_MAX_WORKSPACE_DIMENSIONS, maxWorkspace);

   // Low/left/back point of device workspace.
   hduVector3Dd LLB(maxWorkspace[0], maxWorkspace[1], maxWorkspace[2]);
   // Top/right/front point of device workspace.
   hduVector3Dd TRF(maxWorkspace[3], maxWorkspace[4], maxWorkspace[5]);


   ScheduleForceCallback();
}

/**************************************************************************
  Get Position of End-Effector
 **************************************************************************/
hduVector3Dd PhantomManager::getEndEffectorPosition(){
   DeviceDisplayState state;
   hdScheduleSynchronous(DeviceStateCallback, &state,
         HD_MIN_SCHEDULER_PRIORITY);
   return state.position;
}

/**************************************************************************
  Get Rotation 
 **************************************************************************/
hduVector3Dd PhantomManager::getRotation(){
   DeviceDisplayState state;
   hdScheduleSynchronous(DeviceStateCallback, &state,
         HD_MIN_SCHEDULER_PRIORITY);
   return state.rotation;
}


/**************************************************************************
  Get Velocity of End-Effector
 **************************************************************************/
hduVector3Dd PhantomManager::getVelocity(){
   DeviceDisplayState state;
   hdScheduleSynchronous(DeviceStateCallback, &state,
         HD_MIN_SCHEDULER_PRIORITY);
   return state.velocity;
}

///////////////////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////////////
// vizmo_obj

void vizmo_obj::Clean(){
   if(m_Robot!=NULL){
      delete m_Robot->getModel(); delete m_Robot->getLoader(); delete m_Robot;
   }

   if( m_BBox!=NULL ){
      delete m_BBox->getModel(); delete m_BBox->getLoader(); delete m_BBox;
   }

   if( m_Qry!=NULL ){
      delete m_Qry->getModel(); delete m_Qry->getLoader(); delete m_Qry;
   }

   if( m_Path!=NULL ){
      delete m_Path->getModel(); delete m_Path->getLoader(); delete m_Path;
   }

   if( m_Debug!=NULL ){
      delete m_Debug->getModel(); delete m_Debug->getLoader(); delete m_Debug;
   }

   if( m_Env!=NULL ){
      delete m_Env->getModel(); delete m_Env->getLoader(); delete m_Env;
   }

   if( m_Map!=NULL ){
      delete m_Map->getModel(); delete m_Map->getLoader(); delete m_Map;
   }
   m_Robot=m_BBox=m_Qry=m_Path=m_Debug=m_Env=m_Map=NULL;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

vizmo::vizmo()
{
   m_obj.m_show_BBox=true;
   m_obj.m_show_Qry=false; 
   m_obj.m_show_Path=false;       
   m_obj.m_show_Debug=false;       
   m_obj.m_show_Map=false; 

   query_changed = false;
}

vizmo::~vizmo()
{
}

//////////////////////////////////////////////////////////////////////
// Core 
//////////////////////////////////////////////////////////////////////
void vizmo::GetAccessFiles(const string& filename)
{
   int pos=filename.rfind('.');
   string name=filename.substr(0,pos);
   //test if files exist
   string envname; //env file name
   string mapname=name+".map";
   if( FileExits(mapname) ){ //check map and get env file name
      m_obj.m_MapFile=mapname;
      //parse header of map to get env filename
      CMapHeaderLoader maploader;
      maploader.SetDataFileName(mapname);
      maploader.ParseFile();
      envname=maploader.GetEnvFileName();
   }
   else m_obj.m_MapFile="";

   if(envname.empty()) 
     envname=name+".env";
   if(FileExits(envname)){ 
     m_obj.m_EnvFile=envname;}
   else 
     m_obj.m_EnvFile="";
   //guess path and query file name
   if(FileExits(name+".path")){
     m_obj.m_PathFile=name+".path";}
   else 
     m_obj.m_PathFile="";
   if(FileExits(name+".vd")){
     m_obj.m_DebugFile=name+".vd";}
   else 
     m_obj.m_DebugFile="";
   if(FileExits(name+".query")){
     m_obj.m_QryFile=name+".query";}
   else 
     m_obj.m_QryFile="";
}

bool vizmo::InitVizmoObject()
{
   //delete old stuff
   m_Plum.Clean();
   m_obj.Clean();

   //create env first
   string name;
   name=m_obj.m_EnvFile; //FindName("env",filenames);
   if(!name.empty()){
      if(!CreateEnvObj(m_obj,name)) 
        return false;
      cout<<"Load Environment File : "<<name<<endl;
   }

   //create robot
   if(!CreateRobotObj(m_obj)) 
     return false;

   //create map
   name=m_obj.m_MapFile;//FindName("map",filenames); 
   if(!name.empty()){
      if(!CreateMapObj(m_obj,name)) 
        return false;
      cout<<"Load Map File : "<<name<<endl;
   }

   //create path
   name=m_obj.m_PathFile;//FindName("path",filenames);
   if(!name.empty()){
      if(!CreatePathObj(m_obj,name)) 
        return false;
      cout<<"Load Path File : "<<name<<endl;
   }

   //create debug
   name=m_obj.m_DebugFile;//FindName("path",filenames);
   if(!name.empty()){
      if(!CreateDebugObj(m_obj,name)) 
        return false;
      cout<<"Load Debug File : "<<name<<endl;
   }

   //create qry
   name=m_obj.m_QryFile;//FindName("query",filenames);
   if(!name.empty()){
      if(!CreateQueryObj(m_obj,name)) 
        return false;
      cout<<"Load Query File : "<<name<<endl;
   }

   //create bbx
   if(!CreateBBoxObj(m_obj)){
     return false;}

   //add all of them into plum

   m_Plum.AddPlumObject(m_obj.m_BBox);
   m_Plum.AddPlumObject(m_obj.m_Robot);
   m_Plum.AddPlumObject(m_obj.m_Env);
   m_Plum.AddPlumObject(m_obj.m_Path);
   m_Plum.AddPlumObject(m_obj.m_Debug);
   m_Plum.AddPlumObject(m_obj.m_Qry);
   m_Plum.AddPlumObject(m_obj.m_Map);


   //let plum do what he needs to do
   if(m_Plum.ParseFile()==CPlumState::PARSE_ERROR){
     return false;}
   if(m_Plum.BuildModels()!=CPlumState::BUILD_MODEL_OK){
     return false;}

   //put robot in start cfg, if availiable
   PlaceRobot();

   // Init. variables used to change color of env. objects
   mR = mG = mB = 0;

   //setup visibility
   ShowRoadMap(m_obj.m_show_Map);
   ShowPathFrame(m_obj.m_show_Path);
   ShowDebugFrame(m_obj.m_show_Debug);
   ShowQueryFrame(m_obj.m_show_Qry);
   ShowBBox(m_obj.m_show_BBox);

   return true;
}

void vizmo::RefreshEnv()
{
   if(m_obj.m_Env==NULL) 
     return;
   CGLModel *m=m_obj.m_Env->getModel();

   m->SetRenderMode(CPlumState::MV_SOLID_MODE);
}


//////////////////////////////////////////////////
// Collision Detection related functions
//////////////////////////////////////////////////

//* Node_CD is called from roadmap.cpp
//* receives the cfg of the node moved
//* which will be tested for collision
void vizmo::Node_CD(CCfg *cfg){

   //cfg->coll = false; //used to write message in CCfg::GetInfo()
   m_cfg = cfg;

   int dof = CCfg::m_dof;
   m_IsNode = true;
   vector<double> dataCfg;
   dataCfg = cfg->GetDataCfg();
   m_nodeCfg = new double[dof];

   for(int i=0;i<dof;i++){
      m_nodeCfg[i] = dataCfg[i];
   }
}


void vizmo::TurnOn_CD(){
   
    string m_objName;

   vector<gliObj>& sel=GetVizmo().GetSelectedItem();
   typedef vector<gliObj>::iterator OIT;
   for(OIT i=sel.begin();i!=sel.end();i++){
      m_objName = ((CGLModel*)(*i))->GetName();
   }
   CEnvModel* env=(CEnvModel*)m_obj.m_Env->getModel();
   CEnvLoader* envLoader=(CEnvLoader*)m_obj.m_Env->getLoader();
   if(envLoader != NULL){
      int MBnum = envLoader->GetNumberOfMultiBody();

      OBPRMView_Robot* robot=(OBPRMView_Robot*)m_obj.m_Robot->getModel();

      list<CGLModel*> robotList,modelList;
      //obtain robot model	  
      robot->GetChildren(modelList);
      MultiBodyModel * robotModel = (MultiBodyModel*)modelList.front();

      //If we'll test a node, copy Cfg to CD class
      if(m_IsNode){
         int dof = CCfg::m_dof;
         CD.CopyNodeCfg(m_nodeCfg, dof);
      }

      if(m_objName != "Node"){
         m_IsNode = false;
      }

      bool b = false;
      b = CD.IsInCollision(MBnum, env, envLoader, robotModel, robot);
      is_collison = b;

      if (b){

         if(m_cfg != NULL){
            m_cfg->m_coll = true;
         }
#ifdef USE_PHANTOM
         GetPhantomManager().Collision = 1;
#endif
      }
      else{

         if(m_cfg != NULL){
            m_cfg->m_coll = false;
         }

#ifdef USE_PHANTOM
         GetPhantomManager().Collision = 0;
#endif
      }
   }


}


////////////////////////////////////////////////////
// Saving files related functions
//
// bool vizmo::SaveEnv(const char *filename)
//
// bool SaveQry(const char *filename)
// which uses 
// SaveQryStart() and SaveQryGoal()
// 
///////////////////////////////////////////////////

bool vizmo::SaveEnv(const char *filename)
{
   CEnvModel* env=(CEnvModel*)m_obj.m_Env->getModel();
   env=(CEnvModel*)m_obj.m_Env->getModel();

   env->SaveFile(filename);
   return 1;
}

void vizmo::SaveQryCfg(char ch){

  typedef vector<gliObj>::iterator GIT;
  string name;
  CGLModel * gl;

  OBPRMView_Robot* robot=(OBPRMView_Robot*)m_obj.m_Robot->getModel();

  for(GIT ig= GetSelectedItem().begin();ig!=GetSelectedItem().end();ig++)
  {
    gl=(CGLModel *)(*ig);
    vector<string> info=gl->GetInfo();
    name = info.front();
  }

  if(name == "Robot"){
    //to store a single cfg
    vector<vector<double> > cfg;

    int dof = CCfg::m_dof;
    if(m_obj.m_Qry != NULL){
      //get original Cfgs from QueryLoader
      CQueryLoader * q=(CQueryLoader*)m_obj.m_Qry->getLoader();

      unsigned int iQSize = q->GetQuerySize();

      for( unsigned int iQ=0; iQ<iQSize; iQ++ ){
        vector<double> Cfg(dof);
        Cfg = q->GetStartGoal(iQ); 
        cfg.push_back(Cfg);
      }
    }
    else{
      vector<double> c = robot->getFinalCfg();
      cfg.push_back(c);
      cfg.push_back(c);
    }

    robot->SaveQry(cfg, ch);
  }

}

bool vizmo::SaveQry(const char *filename){
   int dof = CCfg::m_dof;
   vector<double *> cfg;
   FILE *qryFile;

   OBPRMView_Robot* robot=(OBPRMView_Robot*)m_obj.m_Robot->getModel();
   vector<vector<double> > vSG = robot->getNewStartAndGoal();

   if(!vSG.empty()){
      //open file
      if((qryFile = fopen(filename, "a")) == NULL){
         cout<<"Couldn't open the file"<<endl;
         return 0;
      }
      //get values
      typedef vector<vector<double> >::iterator IC;
      for(IC ic=vSG.begin(); ic!=vSG.end(); ic++){
         for(int i=0; i<dof; i++){
            fprintf(qryFile, "%2f ", (*ic)[i]);
         }
         fprintf(qryFile, "\n");
      }
      fclose(qryFile);
   }
   return 1;
}

void vizmo::ShowRoadMap( bool bShow ){
   m_obj.m_show_Map=bShow;
   if( m_obj.m_Map==NULL ) 
     return;
   CGLModel* m=m_obj.m_Map->getModel();

   if( bShow )
      m->SetRenderMode(CPlumState::MV_SOLID_MODE);
   else
      m->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);   

}

void vizmo::ShowPathFrame( bool bShow ){
   m_obj.m_show_Path=bShow;
   if( m_obj.m_Path==NULL ) 
     return;
   CGLModel * m=m_obj.m_Path->getModel();

   if( bShow )
      m->SetRenderMode(CPlumState::MV_SOLID_MODE);
   else
      m->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);
}

void vizmo::ShowDebugFrame( bool bShow ){
   m_obj.m_show_Debug=bShow;
   if( m_obj.m_Debug==NULL ) 
     return;
   CGLModel * m=m_obj.m_Debug->getModel();

   if( bShow )
      m->SetRenderMode(CPlumState::MV_SOLID_MODE);
   else
      m->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);
}

void vizmo::ShowQueryFrame(bool bShow){
   m_obj.m_show_Qry=bShow;
   if(m_obj.m_Qry==NULL) 
     return;
   CGLModel* m = m_obj.m_Qry->getModel();
   if (bShow)
      m->SetRenderMode(CPlumState::MV_SOLID_MODE);
   else 
      m->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);
} 

// Code to show or hide bouding box!!!!
// BSS

void vizmo::ShowBBox(bool bShow){
   m_obj.m_show_BBox=bShow;
   if(m_obj.m_BBox==NULL) 
     return;
   CGLModel * m=m_obj.m_BBox->getModel();
   if(bShow)
      m->SetRenderMode(CPlumState::MV_SOLID_MODE);
   else
      m->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);
}

// Code To change the appearance of the env.. 
// BSS
void vizmo::ChangeAppearance(int status)
{
   // status 0 = solid
   // status 1 = wire
   // status 2 = delete
   // status 3 = change color

   typedef vector<gliObj>::iterator GIT;

   OBPRMView_Robot* robot=(OBPRMView_Robot*)m_obj.m_Robot->getModel();
   robot->BackUp();   

   for(GIT ig= GetSelectedItem().begin();ig!=GetSelectedItem().end();ig++)
   {
      CGLModel *model=(CGLModel *)(*ig);
      if(status==0)
         model->SetRenderMode(CPlumState::MV_SOLID_MODE);
      else if(status==1)
         model->SetRenderMode(CPlumState::MV_WIRE_MODE);
      else if(status==2){
         model->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);
         MultiBodyModel* mbl=(MultiBodyModel*)(*ig);
         DeleteObject(mbl);
      }
      else if(status == 3){
         if((model->GetInfo()).front() == "Robot"){
            robot->SetColor(mR, mG, mB, 1.0);
            robot->keepColor(mR, mG, mB);
            robot->BackUp();
         }
         
         else if((model->GetInfo()).front() == "Node" || (model->GetInfo()).front() == "Edge"){
            model->m_RGBA[0]=mR;
            model->m_RGBA[1]=mG;
            model->m_RGBA[2]=mB;

         }
         
         else {
            model->SetColor(mR,mG,mB,1);
         }
      }
   }

   if(robot != NULL){
      robot->Restore();
   }   

   UpdateSelection();

}

void vizmo::DeleteObject(MultiBodyModel *mbl){

   CEnvLoader* envLoader=(CEnvLoader*)m_obj.m_Env->getLoader();
   int MBnum = envLoader->GetNumberOfMultiBody();

   const CMultiBodyInfo * mbi;
   CMultiBodyInfo *mbiTmp;
   mbiTmp = new CMultiBodyInfo [MBnum];
   mbi = envLoader->GetMultiBodyInfo();

   int j=0;
   for(int i=0; i<MBnum; i++){
      if( (mbi[i].m_pBodyInfo[0].m_strModelDataFileName !=
               mbl->GetMBinfo().m_pBodyInfo[0].m_strModelDataFileName) ||
            (mbi[i].m_pBodyInfo[0].m_X != mbl->GetMBinfo().m_pBodyInfo[0].m_X )||
            (mbi[i].m_pBodyInfo[0].m_Y != mbl->GetMBinfo().m_pBodyInfo[0].m_Y)||
            (mbi[i].m_pBodyInfo[0].m_Z != mbl->GetMBinfo().m_pBodyInfo[0].m_Z) ){

         mbiTmp[j] = mbi[i];
         j++;
      }
   }

   envLoader->DecreaseNumMB();
   envLoader->SetNewMultiBodyInfo(mbiTmp);

   //////////////////////////////////////////////////////////
   //  Recreate MBModel: some elements could've been deleted
   //////////////////////////////////////////////////////////
   CEnvModel* env=(CEnvModel*)m_obj.m_Env->getModel();
   env->DeleteMBModel(mbl);

}


  void vizmo::Animate(int frame){
    if( m_obj.m_Robot==NULL || m_obj.m_Path==NULL)
      return;
    CPathLoader* ploader=(CPathLoader*)m_obj.m_Path->getLoader();
    OBPRMView_Robot* rmodel=(OBPRMView_Robot*)m_obj.m_Robot->getModel();

    //Get Cfg
    vector<double> dCfg=ploader->GetConfiguration(frame);

    //reset robot's original position
    ResetRobot();

    rmodel->Configure(dCfg);
  }

  void vizmo::AnimateDebug(int frame){
    if( m_obj.m_Robot==NULL || m_obj.m_Debug==NULL)
      return;
    m_obj.m_show_Debug = true;
    DebugModel* dmodel=(DebugModel*)m_obj.m_Debug->getModel();

    dmodel->ConfigureFrame(frame);
  }

int vizmo::GetPathSize(){ 
   if(m_obj.m_Path==NULL) 
     return 0; 
   CPathLoader* ploader=(CPathLoader*)m_obj.m_Path->getLoader();
   return ploader->GetPathSize();
}

int vizmo::GetDebugSize(){ 
   if(m_obj.m_Debug==NULL) 
     return 0; 
   CDebugLoader* dloader=(CDebugLoader*)m_obj.m_Debug->getLoader();
   return dloader->GetDebugSize();
}

void vizmo::ChangeNodesSize(float s, string str){
  
  if(m_obj.m_Robot==NULL) 
    return;
  if(m_obj.m_Map==NULL && m_obj.m_Debug==NULL) 
    return;

  typedef CMapModel<CCfg,Edge> MM;
  typedef CCModel<CCfg,Edge> CC;
  typedef vector<CC*>::iterator CCIT;

  if(m_obj.m_Map!=NULL){
    MM* mmodel =(MM*)m_obj.m_Map->getModel();
    vector<CC*>& cc=mmodel->GetCCModels();
    for(CCIT ic=cc.begin(); ic!=cc.end(); ic++){
      CC::Shape shape=CC::Point;
      if(str=="Robot") 
        shape=CC::Robot;
      else if(str=="Box") 
        shape=CC::Box;
      (*ic)->scaleNode(s, shape);
    }
  }
  if(m_obj.m_Debug!=NULL){
    DebugModel* dmodel = ((DebugModel*)m_obj.m_Debug->getModel()); 
    vector<CC*>& cc=dmodel->GetMapModel()->GetCCModels();
    for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
      CC::Shape shape = CC::Point;
      if( str=="Robot")
        shape=CC::Robot;
      else if( str=="Box")
        shape=CC::Box;
      (*ic)->scaleNode(s, shape);
    }
  }
}

//Changing edge thickness: step 2
//This function calls ScaleEdges in CCModel.h 
void
vizmo::ChangeEdgeThickness(size_t _t){
    
  if(m_obj.m_Robot == NULL) 
    return; 
  if(m_obj.m_Map == NULL && m_obj.m_Debug == NULL)
    return; 
   
  typedef CMapModel<CCfg, Edge> MM; 
  typedef CCModel<CCfg, Edge> CC; 
  typedef vector<CC*>::iterator CCIT;
  
  if(m_obj.m_Map != NULL){ 
    MM* mmodel = (MM*)m_obj.m_Map->getModel(); 
    vector<CC*> cc = mmodel->GetCCModels(); 
    for(CCIT ic=cc.begin(); ic!=cc.end(); ic++)
      (*ic)->ScaleEdges(_t); 
  }
    
  if(m_obj.m_Debug != NULL){
    DebugModel* dmodel = ((DebugModel*)m_obj.m_Debug->getModel()); 
    vector<CC*>& cc = dmodel->GetMapModel()->GetCCModels(); 
    for(CCIT ic=cc.begin(); ic!=cc.end(); ic++)
      (*ic)->ScaleEdges(_t); 
  }
}

void vizmo::ChangeNodesShape(string s){
  if(m_obj.m_Robot==NULL) 
    return;

  if(m_obj.m_Map==NULL && m_obj.m_Debug==NULL) 
    return;


  typedef CMapModel<CCfg,Edge> MM;
  typedef CCModel<CCfg,Edge> CC;
  typedef vector<CC*>::iterator CCIT;

  if(m_obj.m_Map!=NULL){
    CMapModel<CCfg,Edge>* mmodel =(MM*)m_obj.m_Map->getModel();
    vector<CC*>& cc=mmodel->GetCCModels();
    for(CCIT ic=cc.begin(); ic!=cc.end(); ic++){
      CC::Shape shape=CC::Point;
      if(s=="Robot") 
        shape=CC::Robot;
      else if( s=="Box" ) 
        shape=CC::Box;
      (*ic)->changeShape(shape);
    }
  }
  if(m_obj.m_Debug!=NULL){
    DebugModel* dmodel = (DebugModel*)m_obj.m_Debug->getModel(); 
    vector<CC*>& cc=dmodel->GetMapModel()->GetCCModels();
    for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
      CC::Shape shape = CC::Point;
      if( s=="Robot")
        shape=CC::Robot;
      else if( s=="Box")
        shape=CC::Box;
      (*ic)->changeShape(shape);
    }
  }
}


void vizmo::ChangeCCColor(double r, double g, double b, string s){

  if( m_obj.m_Robot==NULL)
    return;

  if( m_obj.m_Map==NULL && m_obj.m_Debug==NULL)
    return;

  typedef CMapModel<CCfg,Edge> MM;
  typedef CCModel<CCfg,Edge> CC;
  typedef vector<CC*>::iterator CCIT; 

  //change color of one CC at a time
  vector<gliObj>& sel = GetVizmo().GetSelectedItem();
  typedef vector<gliObj>::iterator SI;
  int m_i;
  string m_sO;
  for(SI i = sel.begin(); i!= sel.end(); i++){
    CGLModel *gl = (CGLModel*)(*i);
    m_sO = gl->GetName();
  }
  string m_s="NULL";
  size_t position = m_sO.find("CC",0);
  if(position != string::npos){
    m_s = m_sO.substr(position+2, m_sO.length());
  }   

  if(m_obj.m_Map!=NULL){
    CMapModel<CCfg,Edge>* mmodel =(MM*)m_obj.m_Map->getModel();
    vector<CC*>& cc=mmodel->GetCCModels();
    if(m_s != "NULL"){
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
        CC::Shape shape=CC::Point;
        if(s=="Robot")
          shape=CC::Robot;
        else if(s=="Box")
          shape=CC::Box;
        if(StringToInt(m_s, m_i)){
          if(m_i == (*ic)->id){
            (*ic)->DrawSelect();
            (*ic)->newColor = true;
            (*ic)->changeColor(r, g, b, shape);
            (*ic)->DrawRobotNodes((*ic)->m_RenderMode);
          }
        }
      }
    }
    else if(m_s == "NULL" && oneColor){
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
        CC::Shape shape=CC::Point;
        if(s=="Robot")
          shape=CC::Robot;
        else if(s=="Box")
          shape=CC::Box;
        (*ic)->newColor = true;
        (*ic)->changeColor(r, g, b, shape);
        (*ic)->DrawRobotNodes((*ic)->m_RenderMode);
      }
      oneColor = false;
    }
    else{
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
        CC::Shape shape=CC::Point;
        if(s=="Robot")
          shape=CC::Robot;
        else if(s=="Box")
          shape=CC::Box;
        (*ic)->newColor = true;
        r = ((float)rand())/RAND_MAX; 
        g = ((float)rand())/RAND_MAX; 
        b = ((float)rand())/RAND_MAX; 
        (*ic)->changeColor(r, g, b, shape);
        (*ic)->DrawRobotNodes((*ic)->m_RenderMode);
      }
    }
  }
  if(m_obj.m_Debug!=NULL){
    DebugModel* dmodel = (DebugModel*)m_obj.m_Debug->getModel(); 
    vector<CC*>& cc=dmodel->GetMapModel()->GetCCModels();
    if(m_s != "NULL"){
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
        CC::Shape shape=CC::Point;
        if( s=="Robot")
          shape=CC::Robot;
        else if( s=="Box")
          shape=CC::Box;
        if(StringToInt(m_s, m_i)){
          if(m_i == (*ic)->id){
            (*ic)->DrawSelect();
            (*ic)->newColor = true;
            (*ic)->changeColor(r, g, b, shape);
            (*ic)->DrawRobotNodes((*ic)->m_RenderMode);
          }
        }
      }
    }
    else if(m_s == "NULL" && oneColor){
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
        CC::Shape shape=CC::Point;
        if(s=="Robot")
          shape=CC::Robot;
        else if(s=="Box")
          shape=CC::Box;
        (*ic)->newColor = true;
        (*ic)->changeColor(r, g, b, shape);
        (*ic)->DrawRobotNodes((*ic)->m_RenderMode);
      }
      oneColor = false;
    }
    else{
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
        CC::Shape shape=CC::Point;
        if(s=="Robot")
          shape=CC::Robot;
        else if(s=="Box")
          shape=CC::Box;
        (*ic)->newColor = true;
        r = ((float)rand())/RAND_MAX; 
        g = ((float)rand())/RAND_MAX; 
        b = ((float)rand())/RAND_MAX; 
        (*ic)->changeColor(r, g, b, shape);
        (*ic)->DrawRobotNodes((*ic)->m_RenderMode);
      }
    }
  }
}

void vizmo::UpdateSelection(){


   if( m_obj.m_Robot==NULL ) 
     return;

   if( m_obj.m_Map==NULL && m_obj.m_Debug==NULL) 
     return;

   typedef CMapModel<CCfg,Edge> MM;
   typedef CCModel<CCfg,Edge> CC;
   typedef vector<CC*>::iterator CCIT;

   //change color of one CC at a time
   vector<gliObj>& sel = GetVizmo().GetSelectedItem();
   typedef vector<gliObj>::iterator SI;
   int m_i;
   string m_sO;
   for(SI i = sel.begin(); i!= sel.end(); i++){
      CGLModel *gl = (CGLModel*)(*i);
      m_sO = gl->GetName();

      string m_s="NULL";
      size_t position = m_sO.find("Node",0);
      if(position != string::npos){
         m_s = m_sO.substr(position+4, m_sO.length());
      }   

      if(m_obj.m_Map!=NULL){
        CMapModel<CCfg,Edge>* mmodel = (MM*)m_obj.m_Map->getModel();
        vector<CC*>& cc = mmodel->GetCCModels();
        if(m_s != "NULL"){
          for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
            typedef map<CC::VID, CCfg>::iterator NIT;
            for(NIT i=(*ic)->m_Nodes.begin(); i!=(*ic)->m_Nodes.end(); i++)
              if(StringToInt(m_s, m_i)){
                if(m_i == i->second.GetIndex()){   
                  (*ic)->DrawSelect();
                  (*ic)->newColor = true;
                  (*ic)->ReBuildAll();            
                  (*ic)->DrawRobotNodes(GL_RENDER);
                  (*ic)->DrawSelect();
                }
              }
          }
        }
      }
      
      if(m_obj.m_Debug!=NULL){
        DebugModel* dmodel = (DebugModel*)m_obj.m_Debug->getModel(); 
        vector<CC*>& cc = dmodel->GetMapModel()->GetCCModels();
        if(m_s != "NULL"){
          for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
            typedef map<CC::VID, CCfg>::iterator NIT;
            for(NIT i=(*ic)->m_Nodes.begin(); i!=(*ic)->m_Nodes.end(); i++)
              if(StringToInt(m_s, m_i)){
                if(m_i == i->second.GetIndex()){   
                  (*ic)->DrawSelect();
                  (*ic)->newColor = true;
                  (*ic)->ReBuildAll();            
                  (*ic)->DrawRobotNodes(GL_RENDER);
                  (*ic)->DrawSelect();
                }
              }
          }
        }
      }
   }
}


void vizmo::ChangeNodeColor(double r, double g, double b, string s){

   if( m_obj.m_Robot==NULL ) 
     return;

   if( m_obj.m_Map==NULL ) 
     return;

   typedef CMapModel<CCfg,Edge> MM;
   typedef CCModel<CCfg,Edge> CC;
   typedef vector<CC*>::iterator CCIT; 

   //change color of one CC at a time
   vector<gliObj>& sel = GetVizmo().GetSelectedItem();
   typedef vector<gliObj>::iterator SI;
   int m_i;
   string m_sO;
   for(SI i = sel.begin(); i!= sel.end(); i++){
      CGLModel *gl = (CGLModel*)(*i);
      m_sO = gl->GetName();

      string m_s="NULL";
      size_t position = m_sO.find("Node",0);
      if(position != string::npos){
         m_s = m_sO.substr(position+4, m_sO.length());
      }   

      CMapModel<CCfg,Edge>* mmodel =(MM*)m_obj.m_Map->getModel();
      vector<CC*>& cc=mmodel->GetCCModels();
      if(m_s != "NULL"){
         for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
            CC::Shape shape=CC::Point;
            if( s=="Robot" ) 
              shape=CC::Robot;
            else if( s=="Box" ) 
              shape=CC::Box;
            typedef map<CC::VID, CCfg>::iterator NIT;
            for(NIT i=(*ic)->m_Nodes.begin();i!=(*ic)->m_Nodes.end();i++)
               if(StringToInt(m_s, m_i)){
                  if(m_i == i->second.GetIndex()){  
                     (*ic)->ReBuildAll();
                     i->second.m_RGBA[0]=r;
                     i->second.m_RGBA[1]=g;
                     i->second.m_RGBA[2]=b;
                     (*ic)->DrawRobotNodes((*ic)->m_RenderMode);
                     (*ic)->DrawSelect();
                  }
               }
         }
      }
   }
}


void vizmo::ChangeNodesRandomColor(){
   if( m_obj.m_Robot==NULL ) 
     return;
   if( m_obj.m_Map==NULL ) 
     return;

   typedef CMapModel<CCfg,Edge> MM;
   typedef CCModel<CCfg,Edge> CC;
   typedef vector<CC*>::iterator CCIT; 

   //change color
   CMapModel<CCfg,Edge>* mmodel =(MM*)m_obj.m_Map->getModel();
   vector<CC*>& cc=mmodel->GetCCModels();
   for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
      float r = ((float)rand())/RAND_MAX; 
      float g = ((float)rand())/RAND_MAX; 
      float b = ((float)rand())/RAND_MAX; 
      (*ic)->SetColor(r,g,b,1);
      (*ic)->DrawRobotNodes((*ic)->m_RenderMode);
   }
}

bool vizmo::StringToInt(const string &s, int &i){
   istringstream myStream(s);

   if (myStream>>i)
      return true;
   else
      return false;
}

void vizmo::envObjsRandomColor(){

   CEnvModel* env=(CEnvModel*)m_obj.m_Env->getModel();
   env->ChangeColor();

}

double vizmo::GetEnvRadius(){ 
   if(m_obj.m_Env!=NULL ){
      CEnvModel* env=(CEnvModel*)m_obj.m_Env->getModel();
      return env->GetRadius();
   }
   return 200;
}

///////////////////////////////////////////////////////////////////////////////
// Protected Functions
///////////////////////////////////////////////////////////////////////////////
bool vizmo::CreateEnvObj( vizmo_obj& obj, const string& fname )
{
   int pos=fname.rfind('/');
   string dir=fname.substr(0,pos);
   ///////////////////////////////////////////////////////////////////////
   //create environment
   obj.m_Env=createEnvObj(fname , dir);

   return (obj.m_Env!=NULL);
}

bool vizmo::CreateMapObj( vizmo_obj& obj, const string& fname )
{
   CMapLoader<CCfg,Edge> * mloader=new CMapLoader<CCfg,Edge>();
   CMapModel<CCfg,Edge> * mmodel = new CMapModel<CCfg,Edge>();
   if (mloader==NULL || mmodel==NULL) 
      return false;
   mloader->SetDataFileName(fname);
   mmodel->SetMapLoader(mloader);
   if(obj.m_Robot != NULL){
      mmodel->SetRobotModel( (OBPRMView_Robot*)obj.m_Robot->getModel() );
   }
   obj.m_Map = new PlumObject(mmodel, mloader); 
   return (obj.m_Map != NULL);
}

bool vizmo::CreatePathObj( vizmo_obj& obj, const string& fname )
{
   CPathLoader* ploader=new CPathLoader();
   PathModel* pmodel=new PathModel();
   if( ploader==NULL || pmodel==NULL ) 
     return false;
   ploader->SetDataFileName(fname);
   pmodel->SetPathLoader(ploader);
   if( obj.m_Robot!=NULL )
      pmodel->SetModel((OBPRMView_Robot *)obj.m_Robot->getModel());
   obj.m_Path=new PlumObject(pmodel,ploader);
   return (obj.m_Path!=NULL);
}

bool vizmo::CreateDebugObj( vizmo_obj& obj, const string& fname )
{
   CDebugLoader * dloader=new CDebugLoader();
   DebugModel * dmodel=new DebugModel();
   if( dloader==NULL || dmodel==NULL ) 
     return false;
   dloader->SetDataFileName(fname);
   dmodel->SetDebugLoader(dloader);
   if( obj.m_Robot!=NULL )
      dmodel->SetModel((OBPRMView_Robot *)obj.m_Robot->getModel());
   obj.m_Debug=new PlumObject(dmodel,dloader);
   return (obj.m_Debug!=NULL);
}

bool vizmo::CreateQueryObj( vizmo_obj& obj, const string& fname )
{
   CQueryLoader * qloader=new CQueryLoader();
   CQueryModel * qmodel=new CQueryModel();
   if( qloader==NULL || qmodel==NULL ) 
     return false;
   qloader->SetDataFileName(fname);
   qmodel->SetQueryLoader(qloader);
   if( obj.m_Robot!=NULL )
      qmodel->SetModel((OBPRMView_Robot *)obj.m_Robot->getModel());
   obj.m_Qry=new PlumObject(qmodel,qloader);
   return (obj.m_Qry!=NULL);
}

bool vizmo::CreateBBoxObj( vizmo_obj& obj )
{
   if( obj.m_Map==NULL ) 
     return true; //can't build
   CMapHeaderLoader * maploader=(CMapHeaderLoader*)obj.m_Map->getLoader();
   if( maploader->ParseHeader()==false ) 
     return false;
   const string command=maploader->GetPreamble();
   CBoundingBoxParser * bloader=new CBoundingBoxParser();
   CBoundingBoxesModel * bmodel=new CBoundingBoxesModel();
   if( bloader==NULL || bmodel==NULL ){ 
      return false;}
   //well, this time we view filename as command
   bloader->SetDataFileName(command);
   bmodel->SetBBXParser(bloader);
   obj.m_BBox=new PlumObject(bmodel,bloader);
   return (obj.m_BBox!=NULL);
}

bool vizmo::CreateRobotObj( vizmo_obj& obj )
{
   if( m_obj.m_Env==NULL ) 
     return true; //can't build

   CEnvLoader* envLoader=(CEnvLoader*)m_obj.m_Env->getLoader();
   OBPRMView_Robot * r=new OBPRMView_Robot(envLoader);
   if( r==NULL ) 
     return false;
   obj.m_Robot=new PlumObject(r,NULL);
   return (obj.m_Robot!=NULL);
}

void vizmo::PlaceRobot()
{
   OBPRMView_Robot * r=(OBPRMView_Robot*)m_obj.m_Robot->getModel();
   if( r!=NULL ){
      vector<double> cfg;
      if( m_obj.m_Qry!=NULL ){//check query loader
         CQueryLoader * q=(CQueryLoader*)m_obj.m_Qry->getLoader();
         cfg=q->GetStartGoal(0);
      }
      else if( m_obj.m_Path!=NULL ){//check path loader
         CPathLoader * p=(CPathLoader*)m_obj.m_Path->getLoader();
         cfg=p->GetConfiguration(0);
      }
      else {
         CEnvLoader* envLoader=(CEnvLoader*)m_obj.m_Env->getLoader();
         int d = envLoader->getDOF();
         cfg = vector<double>(d);
      }
      if(m_obj.m_Debug!=NULL){
        r->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);
      }
      if(!cfg.empty()){
         r->Configure(cfg);
         //copy initial cfg. to OBPRMView_Robot
         r->InitialCfg(cfg);
      }
   }
}

void vizmo::ResetRobot(){
/*
   OBPRMView_Robot * r=(OBPRMView_Robot*)m_obj.m_Robot->getModel();
   if( r!=NULL )
      r->RestoreInitCfg();*/
}


void vizmo::getRoboCfg(){
   vector<gliObj>& sel=GetSelectedItem();
   typedef vector<gliObj>::iterator SIT;
   for(SIT i=sel.begin();i!=sel.end();i++){
      CGLModel * gl=(CGLModel *)(*i);
      info=gl->GetInfo();
   }
}

int vizmo::getNumJoints(){

   OBPRMView_Robot * r=(OBPRMView_Robot*)m_obj.m_Robot->getModel();
   if( r!=NULL )
      return r->getNumJoints();
   return -1;
}
///////////////////////////////////////////////////////////////////////////////
// Private Functions
///////////////////////////////////////////////////////////////////////////////
/*

   string vizmo::FindName
   (const string & ext, const vector<string> & names) const
   {
   typedef vector<string>::const_iterator SIT;
   for( SIT is=names.begin();is!=names.end();is++ ){
   int pos=is->rfind(".");
   if(pos==string::npos) 
    continue; //not . found
   if( is->substr(pos+1,is->length())==ext )
   return *is;
   }
   return "";
   }
 */

bool vizmo::FileExits(const string& filename) const
{
   ifstream fin(filename.c_str());
   bool result=fin.good();
   fin.close();
   return result;
}

void vizmo::setCommLine(QStringList comm){
   m_commandList = comm;
}

bool vizmo::envChanged(){
   env_changed = false;

   CEnvLoader* envLoader=(CEnvLoader*)m_obj.m_Env->getLoader();
   int numBod = envLoader->GetNumberOfMultiBody();
   const CMultiBodyInfo * mbi;
   mbi = envLoader->GetMultiBodyInfo();

   CEnvModel* env=(CEnvModel*)m_obj.m_Env->getModel();
   vector<MultiBodyModel *> mbm = env->getMBody();


   for(int i=0; i<numBod; i++){

      Quaternion qtmp2;
      qtmp2 = mbm[i]->q();
      Matrix3x3 fm2 = qtmp2.getMatrix();
      Vector3d fv2 = qtmp2.MatrixToEuler(fm2);

      if( ( (mbi[i].m_pBodyInfo[0].m_X != mbm[i]->tx())||
               (mbi[i].m_pBodyInfo[0].m_Y != mbm[i]->ty())||
               (mbi[i].m_pBodyInfo[0].m_Z != mbm[i]->tz()) ) ||
            ( (mbi[i].m_pBodyInfo[0].m_Alpha!= fv2[0]) ||
              (mbi[i].m_pBodyInfo[0].m_Beta != fv2[1]) ||
              (mbi[i].m_pBodyInfo[0].m_Beta != fv2[2]) ) ){

         env_changed = true;
         break;
      }
   }

   return env_changed;
}

void vizmo::changeQryStatus(bool status){
   query_changed = status;
}


void vizmo::setMapObj(CMapLoader<CCfg,Edge> *ml, CMapModel<CCfg,Edge> * mm){
   
   m_obj.m_Map = new PlumObject(mm, ml); 
   m_Plum.AddPlumObject(m_obj.m_Map);

   //ShowRoadMap(true);

}
