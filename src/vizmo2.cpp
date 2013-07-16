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
#include "Plum/MapObj/CfgModel.h"
#include "Plum/PlumObject.h" 
#include "Plum/GLModel.h"

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
OBPRMView_Robot* GetRobot(){return (OBPRMView_Robot*)(GetVizmo().GetRobot()->GetModel());}
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

void 
vizmo_obj::Clean(){

  if(m_Robot != NULL){
    delete m_Robot->GetModel(); 
    delete m_Robot->GetLoader(); 
    delete m_Robot;
  }

  if(m_Qry != NULL){
    delete m_Qry->GetModel(); 
    delete m_Qry->GetLoader(); 
    delete m_Qry;
  }

  if(m_Path != NULL){
    delete m_Path->GetModel(); 
    delete m_Path->GetLoader(); 
    delete m_Path;
  }

  if(m_debug != NULL){
    delete m_debug->GetModel(); 
    delete m_debug->GetLoader(); 
    delete m_debug;
  }

  if(m_Env != NULL){
    delete m_Env->GetModel(); 
    delete m_Env;
  }      

  if(m_map != NULL){
    delete m_map->GetModel(); 
    delete m_map;
  }
  m_Robot = m_Qry = m_Path = m_debug = m_Env = m_map = NULL;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

vizmo::vizmo()
{
   m_obj.m_show_Qry=false; 
   m_obj.m_show_Path=false;       
   m_obj.m_show_Debug=false;       
   m_obj.m_show_Map=false; 

   query_changed = false;
}

vizmo::~vizmo(){}

//////////////////////////////////////////////////////////////////////
// Core 
//////////////////////////////////////////////////////////////////////
void 
vizmo::GetAccessFiles(const string& _filename){
  
  int pos = _filename.rfind('.');
  string name = _filename.substr(0, pos);
  
  //test if files exist
  string mapname = name + ".map";
  string envname = "";
  if(FileExists(mapname)){ 
    m_obj.m_MapFile = mapname;
    MapModel<CfgModel,EdgeModel> headerParser; 
    headerParser.SetFilename(mapname); 
    headerParser.ParseHeader(); 
    envname = headerParser.GetEnvFileName();
  }
  else 
    m_obj.m_MapFile = "";
  
  if(envname.empty()) 
    envname = name + ".env";
    
  if(FileExists(envname)) 
    m_obj.m_EnvFile = envname;
  else  
    m_obj.m_EnvFile = "";
   
  //guess path and query file name
  if(FileExists(name+".path"))
    m_obj.m_PathFile = name + ".path";
  else 
    m_obj.m_PathFile = "";
  
  if(FileExists(name + ".vd"))
    m_obj.m_DebugFile = name + ".vd";
  else 
    m_obj.m_DebugFile = "";
   
  if(FileExists(name + ".query"))
    m_obj.m_QryFile = name + ".query";
  else 
    m_obj.m_QryFile = "";
}

bool 
vizmo::InitVizmoObject(){ 

  //Delete old stuff
  m_Plum.Clean();
  m_obj.Clean();

  //Create environment first
  string envFilename = m_obj.m_EnvFile; 
  if(envFilename.empty() == false){
    if(CreateEnvObj(m_obj, envFilename) == false){ 
      cout << "Failed to create Environment." << endl; 
      return false;
    }
    cout << "Load Environment File : "<< envFilename << endl;
  }

  //Create robot
  if(CreateRobotObj(m_obj) == false) 
    return false;

  //Create map
  string mapFilename = m_obj.m_MapFile; 
  if(mapFilename.empty() == false){   
    if(CreateMapObj(m_obj, mapFilename) == false){ 
      cout << "Failed to create Map." << endl; 
      return false;
    }
    cout << "Load Map File : " << mapFilename << endl;
  }
   
  //Create path
  string pathFilename = m_obj.m_PathFile;
  if(pathFilename.empty() == false){
    if(CreatePathObj(m_obj, pathFilename) == false) 
      return false;
    cout << "Load Path File : " << pathFilename << endl;
  }

  //Create debug
  string debugFilename = m_obj.m_DebugFile; 
  if(debugFilename.empty() == false){
    if(CreateDebugObj(m_obj, debugFilename) == false) 
      return false;
    cout << "Load Debug File : " << debugFilename << endl;
  }

  //Create qry
  string qryFilename = m_obj.m_QryFile; 
  if(qryFilename.empty() == false){
    if(CreateQueryObj(m_obj, qryFilename) == false) 
      return false;
    cout << "Load Query File : " << qryFilename << endl;
  }

  //create bbx
  //if(!CreateBBoxObj(m_obj)){
  //  return false;}

  //Add all of them into plum
  m_Plum.AddPlumObject(m_obj.m_Robot);
  m_Plum.AddPlumObject(m_obj.m_Env);
  m_Plum.AddPlumObject(m_obj.m_Path);
  m_Plum.AddPlumObject(m_obj.m_debug);
  m_Plum.AddPlumObject(m_obj.m_Qry);
  m_Plum.AddPlumObject(m_obj.m_map);

  //let plum do what he needs to do
  if(!m_Plum.ParseFile())
    return false;
   
  if(m_Plum.BuildModels() != MODEL_OK)  
    return false;

  //Put robot in start cfg, if availiable
  if(m_obj.m_Robot != NULL)
    PlaceRobot();

  //Init. variables used to change color of env. objects
  mR = mG = mB = 0;

  //Set up visibility
  ShowRoadMap(m_obj.m_show_Map);
  ShowPathFrame(m_obj.m_show_Path);
  ShowDebugFrame(m_obj.m_show_Debug);
  ShowQueryFrame(m_obj.m_show_Qry);

  return true;
}

void 
vizmo::RefreshEnv(){
   
  if(m_obj.m_Env == NULL) 
    return;
  GLModel* m = m_obj.m_Env->GetModel();
  m->SetRenderMode(SOLID_MODE);
}

//////////////////////////////////////////////////
// Collision Detection related functions
//////////////////////////////////////////////////

//* Node_CD is called from roadmap.cpp
//* receives the cfg of the node moved
//* which will be tested for collision
void vizmo::Node_CD(CfgModel *cfg){

   //cfg->coll = false; //used to write message in CfgModel::GetInfo()
   m_cfg = cfg;

   int dof = CfgModel::m_dof;
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
      m_objName = ((GLModel*)(*i))->GetName();
   }
   EnvModel* env = (EnvModel*)m_obj.m_Env->GetModel();
   //CEnvLoader* envLoader=(CEnvLoader*)m_obj.m_Env->GetLoader();
   if(env != NULL){ //previously checked if loader was null 
      //int MBnum = envLoader->GetNumberOfMultiBody();
      int MBnum = env->GetNumMultiBodies();

      OBPRMView_Robot* robot=(OBPRMView_Robot*)m_obj.m_Robot->GetModel();

      list<GLModel*> robotList,modelList;
      //obtain robot model	  
      robot->GetChildren(modelList);
      MultiBodyModel* robotModel = (MultiBodyModel*)modelList.front();

      //If we'll test a node, copy CfgModel to CD class
      if(m_IsNode){
         int dof = CfgModel::m_dof;
         CD.CopyNodeCfg(m_nodeCfg, dof);
      }

      if(m_objName != "Node"){
         m_IsNode = false;
      }

      bool b = false;
      b = CD.IsInCollision(MBnum, env, robotModel, robot);
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

bool 
vizmo::SaveEnv(const char* _filename){

  EnvModel* env = (EnvModel*)m_obj.m_Env->GetModel();
  env = (EnvModel*)m_obj.m_Env->GetModel();
  env->SaveFile(_filename);
  return 1; //Why? 
}

void vizmo::SaveQryCfg(char ch){

  typedef vector<gliObj>::iterator GIT;
  string name;
  GLModel * gl;

  OBPRMView_Robot* robot=(OBPRMView_Robot*)m_obj.m_Robot->GetModel();

  for(GIT ig= GetSelectedItem().begin();ig!=GetSelectedItem().end();ig++)
  {
    gl=(GLModel *)(*ig);
    vector<string> info=gl->GetInfo();
    name = info.front();
  }

  if(name == "Robot"){
    //to store a single cfg
    vector<vector<double> > cfg;

    int dof = CfgModel::m_dof;
    if(m_obj.m_Qry != NULL){
      //get original Cfgs from QueryLoader
      CQueryLoader * q=(CQueryLoader*)m_obj.m_Qry->GetLoader();

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
   int dof = CfgModel::m_dof;
   vector<double *> cfg;
   FILE *qryFile;

   OBPRMView_Robot* robot=(OBPRMView_Robot*)m_obj.m_Robot->GetModel();
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

void 
vizmo::ShowRoadMap(bool _show){
   
  m_obj.m_show_Map = _show;  
  
  if(m_obj.m_map == NULL) 
    return;
  
  GLModel* m = m_obj.m_map->GetModel();
  if(_show)
    m->SetRenderMode(SOLID_MODE);
  else
    m->SetRenderMode(INVISIBLE_MODE);   
}

void 
vizmo::ShowPathFrame(bool _show){

  m_obj.m_show_Path = _show;
  
  if(m_obj.m_Path == NULL) 
    return;
  
  GLModel* m = m_obj.m_Path->GetModel();
  if(_show)
    m->SetRenderMode(SOLID_MODE);
  else
    m->SetRenderMode(INVISIBLE_MODE);
}

void 
vizmo::ShowDebugFrame(bool _show){
  
  m_obj.m_show_Debug = _show;
  
  if(m_obj.m_debug == NULL) 
    return;
  
  GLModel* m = m_obj.m_debug->GetModel();
  if(_show)
    m->SetRenderMode(SOLID_MODE);
  else
    m->SetRenderMode(INVISIBLE_MODE);
}

void 
vizmo::ShowQueryFrame(bool _show){
  
  m_obj.m_show_Qry = _show;
   
  if(m_obj.m_Qry==NULL) 
    return;
   
  GLModel* m = m_obj.m_Qry->GetModel();
  if(_show)
    m->SetRenderMode(SOLID_MODE);
  else 
    m->SetRenderMode(INVISIBLE_MODE);
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

   OBPRMView_Robot* robot=(OBPRMView_Robot*)m_obj.m_Robot->GetModel();
   robot->BackUp();   

   for(GIT ig= GetSelectedItem().begin();ig!=GetSelectedItem().end();ig++)
   {
      GLModel *model=(GLModel *)(*ig);
      if(status==0)
         model->SetRenderMode(SOLID_MODE);
      else if(status==1)
         model->SetRenderMode(WIRE_MODE);
      else if(status==2){
         model->SetRenderMode(INVISIBLE_MODE);
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
            model->m_rGBA[0]=mR;
            model->m_rGBA[1]=mG;
            model->m_rGBA[2]=mB;

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

   //CEnvLoader* envLoader=(CEnvLoader*)m_obj.m_Env->GetLoader();
   //int MBnum = envLoader->GetNumberOfMultiBody();
   EnvModel* envModel = (EnvModel*)m_obj.m_Env->GetModel();
   int MBnum = envModel->GetNumMultiBodies();

   const CMultiBodyInfo * mbi;
   CMultiBodyInfo *mbiTmp;
   mbiTmp = new CMultiBodyInfo [MBnum];
   mbi = envModel->GetMultiBodyInfo();

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

   //envLoader->DecreaseNumMB();
   //envLoader->SetNewMultiBodyInfo(mbiTmp);
   envModel->DecreaseNumMB();
   envModel->SetNewMultiBodyInfo(mbiTmp);

   //////////////////////////////////////////////////////////
   //  Recreate MBModel: some elements could've been deleted
   //////////////////////////////////////////////////////////
   //CEnvModel* env=(CEnvModel*)m_obj.m_Env->GetModel();
   envModel->DeleteMBModel(mbl);
}

  void vizmo::Animate(int frame){
    if( m_obj.m_Robot==NULL || m_obj.m_Path==NULL)
      return;
    CPathLoader* ploader=(CPathLoader*)m_obj.m_Path->GetLoader();
    OBPRMView_Robot* rmodel=(OBPRMView_Robot*)m_obj.m_Robot->GetModel();

    //Get Cfg
    vector<double> dCfg=ploader->GetConfiguration(frame);

    //reset robot's original position
    ResetRobot();

    rmodel->Configure(dCfg);
  }

  void vizmo::AnimateDebug(int frame){
    if( m_obj.m_Robot==NULL || m_obj.m_debug==NULL)
      return;
    m_obj.m_show_Debug = true;
    DebugModel* dmodel=(DebugModel*)m_obj.m_debug->GetModel();

    dmodel->ConfigureFrame(frame);
  }

int vizmo::GetPathSize(){ 
   if(m_obj.m_Path==NULL) 
     return 0; 
   CPathLoader* ploader=(CPathLoader*)m_obj.m_Path->GetLoader();
   return ploader->GetPathSize();
}

int vizmo::GetDebugSize(){ 
   if(m_obj.m_debug==NULL) 
     return 0; 
   CDebugLoader* dloader=(CDebugLoader*)m_obj.m_debug->GetLoader();
   return dloader->GetDebugSize();
}

void 
vizmo::ChangeNodesSize(float _s, string _str){
  
  if(m_obj.m_Robot==NULL)
    return;
  
  if(m_obj.m_map==NULL && m_obj.m_debug==NULL) 
    return;

  typedef MapModel<CfgModel,EdgeModel> MM;
  typedef CCModel<CfgModel,EdgeModel> CC;
  typedef vector<CC*>::iterator CCIT;

  if(m_obj.m_map!=NULL){
    MM* mmodel =(MM*)m_obj.m_map->GetModel();
    vector<CC*>& cc=mmodel->GetCCModels();
    for(CCIT ic=cc.begin(); ic!=cc.end(); ic++){
      CC::Shape shape=CC::Point;
      if(_str=="Robot") 
        shape=CC::Robot;
      else if(_str=="Box") 
        shape=CC::Box;
      (*ic)->ScaleNode(_s, shape);
    }
  }
  if(m_obj.m_debug!=NULL){
    DebugModel* dmodel = ((DebugModel*)m_obj.m_debug->GetModel()); 
    vector<CC*>& cc=dmodel->GetMapModel()->GetCCModels();
    for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
      CC::Shape shape = CC::Point;
      if(_str == "Box")
        shape = CC::Box;  
      (*ic)->ScaleNode(_s, shape);
    }
  }
}

//Changing edge thickness: step 2
//This function calls ScaleEdges in CCModel.h 
void
vizmo::ChangeEdgeThickness(size_t _t){
    
  if(m_obj.m_Robot == NULL) 
    return; 
  if(m_obj.m_map == NULL && m_obj.m_debug == NULL)
    return; 
   
  typedef MapModel<CfgModel, EdgeModel> MM; 
  typedef CCModel<CfgModel, EdgeModel> CC; 
  typedef vector<CC*>::iterator CCIT;
  
  if(m_obj.m_map != NULL){ 
    MM* mmodel = (MM*)m_obj.m_map->GetModel(); 
    vector<CC*> cc = mmodel->GetCCModels(); 
    for(CCIT ic=cc.begin(); ic!=cc.end(); ic++)
      (*ic)->ScaleEdges(_t); 
  }
    
  if(m_obj.m_debug != NULL){
    DebugModel* dmodel = ((DebugModel*)m_obj.m_debug->GetModel()); 
    vector<CC*>& cc = dmodel->GetMapModel()->GetCCModels(); 
    for(CCIT ic=cc.begin(); ic!=cc.end(); ic++)
      (*ic)->ScaleEdges(_t); 
  }
}

void vizmo::ChangeNodesShape(string _s){
  if(m_obj.m_Robot==NULL)  
    return;

  if(m_obj.m_map==NULL && m_obj.m_debug==NULL) 
    return; 

  if(_s == "Robot")
    CfgModel::m_shape = CfgModel::Robot; 
  if(_s == "Box")
    CfgModel::m_shape = CfgModel::Box; 
  else   
    CfgModel::m_shape = CfgModel::Point;

  typedef MapModel<CfgModel,EdgeModel> MM;
  typedef CCModel<CfgModel,EdgeModel> CC;
  typedef vector<CC*>::iterator CCIT;

  if(m_obj.m_map!=NULL){ 
    MapModel<CfgModel,EdgeModel>* mmodel =(MM*)m_obj.m_map->GetModel();
    vector<CC*>& cc=mmodel->GetCCModels();
    for(CCIT ic=cc.begin(); ic!=cc.end(); ic++){
      CC::Shape shape=CC::Point;
      if(_s=="Robot") 
        shape=CC::Robot;
      else if(_s=="Box") 
        shape=CC::Box;
      (*ic)->ChangeShape(shape);
    }
  }
  if(m_obj.m_debug!=NULL){
    DebugModel* dmodel = (DebugModel*)m_obj.m_debug->GetModel(); 
    vector<CC*>& cc=dmodel->GetMapModel()->GetCCModels();
    for(CCIT ic=cc.begin();ic!=cc.end();ic++){
      CC::Shape shape = CC::Point;
      if(_s=="Robot")
        shape=CC::Robot;
      else if(_s=="Box")
        shape=CC::Box;
      (*ic)->ChangeShape(shape);
    }
  } 
}

void vizmo::ChangeCCColor(double _r, double _g, double _b, string _s){

  if(m_obj.m_Robot == NULL)
    return;

  if(m_obj.m_map == NULL && m_obj.m_debug == NULL)
    return;

  typedef MapModel<CfgModel,EdgeModel> MM;
  typedef CCModel<CfgModel,EdgeModel> CC;
  typedef vector<CC*>::iterator CCIT; 

  //change color of one CC at a time
  vector<gliObj>& sel = GetVizmo().GetSelectedItem();
  typedef vector<gliObj>::iterator SI;
  int m_i;
  string m_sO;
  for(SI i = sel.begin(); i!= sel.end(); i++){
    GLModel *gl = (GLModel*)(*i);
    m_sO = gl->GetName();
  }
  string m_s="NULL";
  size_t position = m_sO.find("CC",0);
  if(position != string::npos){
    m_s = m_sO.substr(position+2, m_sO.length());
  }   

  if(m_obj.m_map!=NULL){
    MapModel<CfgModel,EdgeModel>* mmodel =(MM*)m_obj.m_map->GetModel();
    vector<CC*>& cc=mmodel->GetCCModels();
    if(m_s != "NULL"){
      for(CCIT ic = cc.begin(); ic!= cc.end(); ic++){
        if(StringToInt(m_s, m_i)){
          if(m_i == (*ic)->GetID()){
            (*ic)->DrawSelect();
            (*ic)->SetColorChanged(true);
            (*ic)->SetColor(_r, _g, _b, 1); 
            (*ic)->DrawRobotNodes((*ic)->m_renderMode);
          }
        }
      }
    }
    else if(m_s == "NULL" && oneColor){
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
        (*ic)->SetColorChanged(true); 
        (*ic)->SetColor(_r, _g, _b, 1); 
        (*ic)->DrawRobotNodes((*ic)->m_renderMode);
      }
      oneColor = false;
    }
    else{
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
        (*ic)->SetColorChanged(true);
        _r = ((float)rand())/RAND_MAX; 
        _g = ((float)rand())/RAND_MAX; 
        _b = ((float)rand())/RAND_MAX; 
        (*ic)->SetColor(_r, _g, _b, 1); 
        (*ic)->DrawRobotNodes((*ic)->m_renderMode);
      }
    }
  }
  if(m_obj.m_debug!=NULL){
    DebugModel* dmodel = (DebugModel*)m_obj.m_debug->GetModel(); 
    vector<CC*>& cc=dmodel->GetMapModel()->GetCCModels();
    if(m_s != "NULL"){
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
        if(StringToInt(m_s, m_i)){
          if(m_i == (*ic)->GetID()){
            (*ic)->DrawSelect();
            (*ic)->SetColorChanged(true);
            (*ic)->SetColor(_r, _g, _b, 1); 
            (*ic)->DrawRobotNodes((*ic)->m_renderMode);
          }
        }
      }
    }
    else if(m_s == "NULL" && oneColor){
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
        (*ic)->SetColorChanged(true);
        (*ic)->SetColor(_r, _g, _b, 1); 
        (*ic)->DrawRobotNodes((*ic)->m_renderMode);
      }
      oneColor = false;
    }
    else{
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
        (*ic)->SetColorChanged(true); 
        _r = ((float)rand())/RAND_MAX; 
        _g = ((float)rand())/RAND_MAX; 
        _b = ((float)rand())/RAND_MAX; 
        (*ic)->SetColor(_r, _g, _b, 1); 
        (*ic)->DrawRobotNodes((*ic)->m_renderMode);
      }
    }
  }
}

void vizmo::UpdateSelection(){


   if( m_obj.m_Robot==NULL ) 
     return;

   if( m_obj.m_map==NULL && m_obj.m_debug==NULL) 
     return;

   typedef MapModel<CfgModel,EdgeModel> MM;
   typedef CCModel<CfgModel,EdgeModel> CC;
   typedef vector<CC*>::iterator CCIT;

   //change color of one CC at a time
   vector<gliObj>& sel = GetVizmo().GetSelectedItem();
   typedef vector<gliObj>::iterator SI;
   int m_i;
   string m_sO;
   for(SI i = sel.begin(); i!= sel.end(); i++){
      GLModel *gl = (GLModel*)(*i);
      m_sO = gl->GetName();

      string m_s="NULL";
      size_t position = m_sO.find("Node",0);
      if(position != string::npos){
         m_s = m_sO.substr(position+4, m_sO.length());
      }   

      if(m_obj.m_map!=NULL){
        MapModel<CfgModel,EdgeModel>* mmodel = (MM*)m_obj.m_map->GetModel();
        vector<CC*>& cc = mmodel->GetCCModels();
        if(m_s != "NULL"){
          for(CCIT ic=cc.begin();ic!=cc.end();ic++){
            typedef map<CC::VID, CfgModel>::iterator NIT;
            for(NIT i = (*ic)->GetNodesInfo().begin(); i!=(*ic)->GetNodesInfo().end(); i++)
              if(StringToInt(m_s, m_i)){
                if(m_i == i->second.GetIndex()){   
                  (*ic)->DrawSelect();
                  (*ic)->SetColorChanged(true); 
                  (*ic)->RebuildAll();            
                }
              }
          }
        }
      }
      
      if(m_obj.m_debug!=NULL){
        DebugModel* dmodel = (DebugModel*)m_obj.m_debug->GetModel(); 
        vector<CC*>& cc = dmodel->GetMapModel()->GetCCModels();
        if(m_s != "NULL"){
          for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
            typedef map<CC::VID, CfgModel>::iterator NIT;
            for(NIT i = (*ic)->GetNodesInfo().begin(); i != (*ic)->GetNodesInfo().end(); i++)
              if(StringToInt(m_s, m_i)){
                if(m_i == i->second.GetIndex()){   
                  (*ic)->DrawSelect();
                  (*ic)->SetColorChanged(true);
                  (*ic)->RebuildAll();            
                  (*ic)->DrawRobotNodes(GL_RENDER);
                  (*ic)->DrawSelect();
                }
              }
          }
        }
      }
   }
}


void vizmo::ChangeNodeColor(double _r, double _g, double _b, string _s){

   if(m_obj.m_Robot == NULL) 
     return;

   if(m_obj.m_map == NULL) 
     return;

   typedef MapModel<CfgModel,EdgeModel> MM;
   typedef CCModel<CfgModel,EdgeModel> CC;
   typedef vector<CC*>::iterator CCIT; 

   //change color of one CC at a time
   vector<gliObj>& sel = GetVizmo().GetSelectedItem();
   typedef vector<gliObj>::iterator SI;
   int m_i;
   string m_sO;
   for(SI i = sel.begin(); i!= sel.end(); i++){
      GLModel *gl = (GLModel*)(*i);
      m_sO = gl->GetName();

      string m_s="NULL";
      size_t position = m_sO.find("Node",0);
      if(position != string::npos){
         m_s = m_sO.substr(position+4, m_sO.length());
      }   

      MapModel<CfgModel,EdgeModel>* mmodel =(MM*)m_obj.m_map->GetModel();
      vector<CC*>& cc=mmodel->GetCCModels();
      if(m_s != "NULL"){
         for(CCIT ic=cc.begin();ic!=cc.end();ic++){
            typedef map<CC::VID, CfgModel>::iterator NIT;
            for(NIT i = (*ic)->GetNodesInfo().begin(); i != (*ic)->GetNodesInfo().end(); i++)
               if(StringToInt(m_s, m_i)){
                  if(m_i == i->second.GetIndex()){  
                     (*ic)->RebuildAll();
                     i->second.m_rGBA[0] = _r;
                     i->second.m_rGBA[1] = _g;
                     i->second.m_rGBA[2] = _b;
                     (*ic)->DrawRobotNodes((*ic)->m_renderMode);
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
   if( m_obj.m_map==NULL ) 
     return;

   typedef MapModel<CfgModel,EdgeModel> MM;
   typedef CCModel<CfgModel,EdgeModel> CC;
   typedef vector<CC*>::iterator CCIT; 

   //change color
   MapModel<CfgModel,EdgeModel>* mmodel =(MM*)m_obj.m_map->GetModel();
   vector<CC*>& cc=mmodel->GetCCModels();
   for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
      float r = ((float)rand())/RAND_MAX; 
      float g = ((float)rand())/RAND_MAX; 
      float b = ((float)rand())/RAND_MAX; 
      (*ic)->SetColor(r,g,b,1);
      (*ic)->DrawRobotNodes((*ic)->m_renderMode);
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

   EnvModel* env=(EnvModel*)m_obj.m_Env->GetModel();
   env->ChangeColor();

}

double vizmo::GetEnvRadius(){ 
   if(m_obj.m_Env!=NULL ){
      EnvModel* env=(EnvModel*)m_obj.m_Env->GetModel();
      return env->GetRadius();
   }
   return 200;
}

bool 
vizmo::CreateEnvObj(vizmo_obj& _obj, const string& _fname){
   
  int pos = _fname.rfind('/');
  string dir = _fname.substr(0, pos);
  //EnvModel* envModel = new EnvModel(_fname);
  EnvModel* envModel = new EnvModel();
  envModel->SetFilename(_fname); 
  envModel->SetModelDataDir(dir); 

  if(!envModel) 
    return false;
   
  _obj.m_Env = new PlumObject(envModel, NULL);
  return _obj.m_Env;
}

bool 
vizmo::CreateMapObj(vizmo_obj& _obj, const string& _fname){

  MapModel<CfgModel, EdgeModel>* mmodel = new MapModel<CfgModel, EdgeModel>(); 
  mmodel->SetFilename(_fname); 

  if(mmodel == NULL)
    return false;
  if(_obj.m_Robot != NULL)
    mmodel->SetRobotModel((OBPRMView_Robot*)_obj.m_Robot->GetModel());
   
  _obj.m_map = new PlumObject(mmodel, NULL); 
  return (_obj.m_map != NULL);
}

bool 
vizmo::CreatePathObj(vizmo_obj& _obj, const string& _fname){

  CPathLoader* ploader = new CPathLoader();
  PathModel* pmodel = new PathModel();
  
  if(ploader == NULL || pmodel == NULL) 
    return false;
  ploader->SetFilename(_fname);
  pmodel->SetPathLoader(ploader);
  
  if(_obj.m_Robot != NULL)
    pmodel->SetModel((OBPRMView_Robot *)_obj.m_Robot->GetModel());
  
  _obj.m_Path = new PlumObject(pmodel,ploader);
  return (_obj.m_Path!=NULL);
}

bool 
vizmo::CreateDebugObj(vizmo_obj& _obj, const string& _fname){

  CDebugLoader* dloader = new CDebugLoader();
  DebugModel* dmodel = new DebugModel(); 
  
  //dloader->SetFilename(_fname);
  //DebugModel* dmodel = new DebugModel((MapModel<CfgModel, EdgeModel>*)((_obj.m_map)->m_model));
  //DebugModel* dmodel = new DebugModel(); 
  if(dloader == NULL || dmodel == NULL) 
    return false;
  
  dloader->SetFilename(_fname);
  dmodel->SetDebugLoader(dloader);
  
  if(_obj.m_Robot != NULL)
    dmodel->SetModel((OBPRMView_Robot *)_obj.m_Robot->GetModel());
  
  _obj.m_debug = new PlumObject(dmodel, dloader);
  return (_obj.m_debug != NULL);
}

bool 
vizmo::CreateQueryObj(vizmo_obj& _obj, const string& _fname){

  CQueryLoader* qloader = new CQueryLoader();
  CQueryModel* qmodel = new CQueryModel();
  
  if(qloader == NULL || qmodel == NULL) 
    return false;
   
  qloader->SetFilename(_fname);
  qmodel->SetQueryLoader(qloader);
  
  if(_obj.m_Robot != NULL)
    qmodel->SetModel((OBPRMView_Robot *)_obj.m_Robot->GetModel());
  
  _obj.m_Qry = new PlumObject(qmodel, qloader);
  return (_obj.m_Qry != NULL);
}

bool 
vizmo::CreateRobotObj(vizmo_obj& _obj){

  if(m_obj.m_Env == NULL) 
    return true; //can't build

  EnvModel* envModel = (EnvModel*)m_obj.m_Env->GetModel(); 
  OBPRMView_Robot* r = new OBPRMView_Robot(envModel);
  
  if(r == NULL)  
    return false;
  
  _obj.m_Robot = new PlumObject(r,NULL);
  return (_obj.m_Robot != NULL);
}

void 
vizmo::PlaceRobot(){

  OBPRMView_Robot* r = (OBPRMView_Robot*)m_obj.m_Robot->GetModel();
  
  if(r != NULL){
    vector<double> cfg;
    if(m_obj.m_Qry != NULL){ 
      CQueryLoader* q = (CQueryLoader*)m_obj.m_Qry->GetLoader();
      cfg = q->GetStartGoal(0);
    }
    else if(m_obj.m_Path != NULL){
      CPathLoader* p = (CPathLoader*)m_obj.m_Path->GetLoader();
      cfg = p->GetConfiguration(0);
    }
    else{
      EnvModel* envModel = (EnvModel*)m_obj.m_Env->GetModel();
      int d = envModel->GetDOF();
      cfg = vector<double>(d);
    }
    
    if(m_obj.m_debug != NULL)
      r->SetRenderMode(INVISIBLE_MODE);
    
    if(!cfg.empty()){
      r->Configure(cfg);
      //copy initial cfg. to OBPRMView_Robot
      r->InitialCfg(cfg);
    }
  }
}

void vizmo::ResetRobot(){
/*
   OBPRMView_Robot * r=(OBPRMView_Robot*)m_obj.m_Robot->GetModel();
   if( r!=NULL )
      r->RestoreInitCfg();*/
}


void vizmo::getRoboCfg(){
   vector<gliObj>& sel=GetSelectedItem();
   typedef vector<gliObj>::iterator SIT;
   for(SIT i=sel.begin();i!=sel.end();i++){
      GLModel * gl=(GLModel *)(*i);
      info=gl->GetInfo();
   }
}

int vizmo::getNumJoints(){

   OBPRMView_Robot * r=(OBPRMView_Robot*)m_obj.m_Robot->GetModel();
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

bool 
vizmo::FileExists(const string& _filename) const{
  
  ifstream fin(_filename.c_str());
  bool result = fin.good();
  fin.close();
  return result;
}

void vizmo::setCommLine(QStringList comm){
   m_commandList = comm;
}

bool 
vizmo::EnvChanged(){
   
  m_envChanged = false;

  EnvModel* envModel = (EnvModel*)m_obj.m_Env->GetModel();
  int numBod = envModel->GetNumMultiBodies();
  const CMultiBodyInfo* mbi = envModel->GetMultiBodyInfo();
  vector<MultiBodyModel *> mbm = envModel->GetMultiBodies();

  for(int i = 0; i < numBod; i++){
    Quaternion qtmp2 = mbm[i]->q();
    EulerAngle e;
    convertFromQuaternion(e, qtmp2);

    if( ( (mbi[i].m_pBodyInfo[0].m_X != mbm[i]->tx())||
              (mbi[i].m_pBodyInfo[0].m_Y != mbm[i]->ty())||
              (mbi[i].m_pBodyInfo[0].m_Z != mbm[i]->tz()) ) ||
          ( (mbi[i].m_pBodyInfo[0].m_Alpha!= e.alpha()) ||
            (mbi[i].m_pBodyInfo[0].m_Beta != e.beta()) ||
            (mbi[i].m_pBodyInfo[0].m_Beta != e.gamma()) ) ){

        m_envChanged = true;
        break;
    }
  }

  return m_envChanged;
}

void vizmo::changeQryStatus(bool status){
   query_changed = status;
}

void 
vizmo::SetMapObj(MapModel<CfgModel,EdgeModel>* _mm){
  
  m_obj.m_map = new PlumObject(_mm, NULL); 
  m_Plum.AddPlumObject(m_obj.m_map);
}
