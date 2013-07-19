#include "Vizmo.h"

#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#include "Plum/PlumObject.h"
#include "Plum/GLModel.h"
#include "Models/CfgModel.h"
#include "Models/PathModel.h"
#include "Models/QueryModel.h"
#include "Models/DebugModel.h"
#include "EnvObj/BoundingBoxesModel.h"
#include "EnvObj/BoundingBoxParser.h"
#include "EnvObj/RobotModel.h"
using namespace plum;

////////////////////////////////////////////////////////////////////////////////
//Define Vizmo singleton
////////////////////////////////////////////////////////////////////////////////
Vizmo vizmo;
Vizmo& GetVizmo(){return vizmo;}

////////////////////////////////////////////////////////////////////////////////
// VizmoObj
////////////////////////////////////////////////////////////////////////////////
Vizmo::VizmoObj::VizmoObj() :
  m_robotModel(NULL),
  m_envModel(NULL),
  m_mapModel(NULL), m_showMap(false),
  m_queryModel(NULL), m_showQuery(false),
  m_pathModel(NULL), m_showPath(false),
  m_debugModel(NULL) {
  }

Vizmo::VizmoObj::~VizmoObj() {
  Clean();
}

void
Vizmo::VizmoObj::Clean() {
  delete m_robotModel;
  delete m_envModel;
  delete m_mapModel;
  delete m_queryModel;
  delete m_pathModel;
  delete m_debugModel;
  m_robotModel = NULL;
  m_envModel = NULL;
  m_mapModel = NULL;
  m_queryModel = NULL;
  m_pathModel = NULL;
  m_debugModel = NULL;
  m_showMap = m_showQuery = m_showPath = false;
}

////////////////////////////////////////////////////////////////////////////////
// Vizmo
////////////////////////////////////////////////////////////////////////////////
Vizmo::Vizmo() {}

void
Vizmo::GetAccessFiles(const string& _filename){
  string name = _filename.substr(0, _filename.rfind('.'));

  //test if files exist
  string mapname = name + ".map";
  string envname = "";
  if(FileExists(mapname, false)){
    m_obj.m_mapFilename = mapname;
    MapModel<CfgModel,EdgeModel> headerParser(mapname);
    headerParser.ParseHeader();
    envname = headerParser.GetEnvFileName();
  }
  else
    m_obj.m_mapFilename = "";

  if(envname.empty())
    envname = name + ".env";

  m_obj.m_envFilename = FileExists(envname, false) ? envname : "";
  m_obj.m_queryFilename = FileExists(name+".query", false) ? name+".query" : "";
  m_obj.m_pathFilename = FileExists(name+".path", false) ? name+".path" : "";
  m_obj.m_debugFilename = FileExists(name+".vd", false) ? name+".vd" : "";
}

bool
Vizmo::InitVizmoObj(){

  //Delete old stuff
  m_plum.Clean();
  m_obj.Clean();

  //Create environment first
  if(m_obj.m_envFilename.empty()) {
    cerr << "Error::Vizmo must load an environment file. InitVizmoObj failed." << endl;
    return false;
  }

  m_obj.m_envModel = new EnvModel(m_obj.m_envFilename);
  m_plum.AddPlumObject(new PlumObject(m_obj.m_envModel));
  m_obj.m_robotModel = new RobotModel(m_obj.m_envModel);
  m_plum.AddPlumObject(new PlumObject(m_obj.m_robotModel));
  cout << "Load Environment File : "<< m_obj.m_envFilename << endl;

  //Create map
  if(!m_obj.m_mapFilename.empty()) {
    m_obj.m_mapModel = new MapModel<CfgModel, EdgeModel>(m_obj.m_mapFilename);
    m_obj.m_mapModel->SetRobotModel(m_obj.m_robotModel);
    m_plum.AddPlumObject(new PlumObject(m_obj.m_mapModel));
    cout << "Load Map File : " << m_obj.m_mapFilename << endl;
  }

  //Create qry
  if(!m_obj.m_queryFilename.empty()) {
    m_obj.m_queryModel = new QueryModel(m_obj.m_queryFilename);
    m_obj.m_queryModel->SetModel(m_obj.m_robotModel);
    m_plum.AddPlumObject(new PlumObject(m_obj.m_queryModel));
    cout << "Load Query File : " << m_obj.m_queryFilename << endl;
  }

  //Create path
  if(!m_obj.m_pathFilename.empty()) {
    m_obj.m_pathModel = new PathModel(m_obj.m_pathFilename);
    m_obj.m_pathModel->SetModel(m_obj.m_robotModel);
    m_plum.AddPlumObject(new PlumObject(m_obj.m_pathModel));
    cout << "Load Path File : " << m_obj.m_pathFilename << endl;
  }

  //Create debug
  if(!m_obj.m_debugFilename.empty()){
    m_obj.m_debugModel = new DebugModel(m_obj.m_debugFilename);
    m_obj.m_debugModel->SetModel(m_obj.m_robotModel);
    m_plum.AddPlumObject(new PlumObject(m_obj.m_debugModel));
    cout << "Load Debug File : " << m_obj.m_debugFilename << endl;
  }

  //let plum do what he needs to do
  if(!m_plum.ParseFile()) {
    cout << "Error reading some file." << endl;
    return false;
  }

  if(m_plum.BuildModels() != MODEL_OK) {
    cout << "Error building some model." << endl;
    return false;
  }

  //Put robot in start cfg, if availiable
  PlaceRobot();

  //Init. variables used to change color of env. objects
  mR = mG = mB = 0;

  return true;
}

void
Vizmo::RefreshEnv(){
  if(m_obj.m_envModel)
    m_obj.m_envModel->SetRenderMode(SOLID_MODE);
}

//////////////////////////////////////////////////
// Collision Detection related functions
//////////////////////////////////////////////////

//* Node_CD is called from roadmap.cpp
//* receives the cfg of the node moved
//* which will be tested for collision
void Vizmo::Node_CD(CfgModel *cfg){

  //cfg->coll = false; //used to write message in CfgModel::GetInfo()
  m_cfg = cfg;

  int dof = CfgModel::GetDOF();
  m_IsNode = true;
  vector<double> dataCfg;
  dataCfg = cfg->GetDataCfg();
  m_nodeCfg = new double[dof];

  for(int i=0;i<dof;i++){
    m_nodeCfg[i] = dataCfg[i];
  }
}

void Vizmo::TurnOn_CD(){

  string m_objName;

  vector<gliObj>& sel=GetVizmo().GetSelectedItem();
  typedef vector<gliObj>::iterator OIT;
  for(OIT i=sel.begin();i!=sel.end();i++){
    m_objName = ((GLModel*)(*i))->GetName();
  }
  EnvModel* env = m_obj.m_envModel;
  //CEnvLoader* envLoader=(CEnvLoader*)m_obj.m_envModel->GetLoader();
  if(env != NULL){ //previously checked if loader was null
    //int MBnum = envLoader->GetNumberOfMultiBody();
    int MBnum = env->GetNumMultiBodies();

    RobotModel* robot = m_obj.m_robotModel;

    list<GLModel*> robotList,modelList;
    //obtain robot model
    robot->GetChildren(modelList);
    MultiBodyModel* robotModel = (MultiBodyModel*)modelList.front();

    //If we'll test a node, copy CfgModel to CD class
    if(m_IsNode){
      int dof = CfgModel::GetDOF();
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
        m_cfg->SetInCollision(true);
      }
    }
    else{

      if(m_cfg != NULL){
        m_cfg->SetInCollision(false);
      }
    }
  }
}


////////////////////////////////////////////////////
// Saving files related functions
//
// bool Vizmo::SaveEnv(const char *filename)
//
// bool SaveQry(const char *filename)
// which uses
// SaveQryStart() and SaveQryGoal()
//
///////////////////////////////////////////////////

bool
Vizmo::SaveEnv(const char* _filename){
  m_obj.m_envModel->SaveFile(_filename);
  return true;
}

void Vizmo::SaveQryCfg(char ch){

  typedef vector<gliObj>::iterator GIT;
  string name;
  GLModel * gl;

  RobotModel* robot = m_obj.m_robotModel;

  for(GIT ig= GetSelectedItem().begin();ig!=GetSelectedItem().end();ig++)
  {
    gl=(GLModel *)(*ig);
    vector<string> info=gl->GetInfo();
    name = info.front();
  }

  if(name == "Robot"){
    //to store a single cfg
    vector<vector<double> > cfg;

    int dof = CfgModel::GetDOF();
    if(m_obj.m_queryModel != NULL){
      //get original Cfgs from QueryModel
      QueryModel* q = m_obj.m_queryModel;

      size_t iQSize = q->GetQuerySize();

      for(size_t iQ=0; iQ<iQSize; iQ++){
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

bool Vizmo::SaveQry(const char *filename){
  int dof = CfgModel::GetDOF();
  vector<double *> cfg;
  FILE *qryFile;

  RobotModel* robot = m_obj.m_robotModel;
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
Vizmo::ShowRoadMap(bool _show){
  m_obj.m_showMap = _show;

  if(m_obj.m_mapModel)
    m_obj.m_mapModel->SetRenderMode(_show ? SOLID_MODE : INVISIBLE_MODE);
}

void
Vizmo::ShowPathFrame(bool _show){
  m_obj.m_showPath = _show;

  if(m_obj.m_pathModel)
    m_obj.m_pathModel->SetRenderMode(_show ? SOLID_MODE : INVISIBLE_MODE);
}

void
Vizmo::ShowQueryFrame(bool _show){
  m_obj.m_showQuery = _show;

  if(m_obj.m_queryModel)
    m_obj.m_queryModel->SetRenderMode(_show ? SOLID_MODE : INVISIBLE_MODE);
}

// Code To change the appearance of the env..
// BSS
void Vizmo::ChangeAppearance(int status)
{
  // status 0 = solid
  // status 1 = wire
  // status 2 = delete
  // status 3 = change color

  typedef vector<gliObj>::iterator GIT;

  RobotModel* robot = m_obj.m_robotModel;
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

void Vizmo::DeleteObject(MultiBodyModel *mbl){

  EnvModel* envModel = m_obj.m_envModel;
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
  envModel->DeleteMBModel(mbl);
}

void Vizmo::Animate(int frame){
  if( m_obj.m_robotModel==NULL || m_obj.m_pathModel==NULL)
    return;

  PathModel* pathModel = m_obj.m_pathModel;
  RobotModel* rmodel = m_obj.m_robotModel;

  //Get Cfg
  vector<double> dCfg = pathModel->GetConfiguration(frame);

  rmodel->Configure(dCfg);
}

void Vizmo::AnimateDebug(int frame){
  if( m_obj.m_robotModel==NULL || m_obj.m_debugModel==NULL)
    return;
  DebugModel* debugModel = m_obj.m_debugModel;

  debugModel->ConfigureFrame(frame);
}

int Vizmo::GetPathSize(){
  if(m_obj.m_pathModel==NULL)
    return 0;

  PathModel* pathModel = m_obj.m_pathModel;
  return pathModel->GetPathSize();
}

int Vizmo::GetDebugSize(){
  if(m_obj.m_debugModel==NULL)
    return 0;
  DebugModel* debugModel = m_obj.m_debugModel;
  return debugModel->GetDebugSize();
}

void
Vizmo::ChangeNodesSize(float _s, string _str){

  if(m_obj.m_robotModel==NULL)
    return;

  if(m_obj.m_mapModel==NULL && m_obj.m_debugModel==NULL)
    return;

  typedef MapModel<CfgModel,EdgeModel> MM;
  typedef CCModel<CfgModel,EdgeModel> CC;
  typedef vector<CC*>::iterator CCIT;

  if(m_obj.m_mapModel!=NULL){
    MM* mmodel = m_obj.m_mapModel;
    vector<CC*>& cc=mmodel->GetCCModels();
    for(CCIT ic=cc.begin(); ic!=cc.end(); ic++){
      CfgModel::Shape shape=CfgModel::Point;
      if(_str=="Robot")
        shape=CfgModel::Robot;
      else if(_str=="Box")
        shape=CfgModel::Box;
      (*ic)->ScaleNode(_s, shape);
    }
  }
  if(m_obj.m_debugModel!=NULL){
    DebugModel* debugModel = m_obj.m_debugModel;
    vector<CC*>& cc=debugModel->GetMapModel()->GetCCModels();
    for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
      CfgModel::Shape shape = CfgModel::Point;
      if(_str == "Box")
        shape = CfgModel::Box;
      (*ic)->ScaleNode(_s, shape);
    }
  }
}

//Changing edge thickness: step 2
//This function calls ScaleEdges in CCModel.h
void
Vizmo::ChangeEdgeThickness(size_t _t){

  if(m_obj.m_robotModel == NULL)
    return;
  if(m_obj.m_mapModel == NULL && m_obj.m_debugModel == NULL)
    return;

  typedef MapModel<CfgModel, EdgeModel> MM;
  typedef CCModel<CfgModel, EdgeModel> CC;
  typedef vector<CC*>::iterator CCIT;

  if(m_obj.m_mapModel != NULL){
    MM* mmodel = m_obj.m_mapModel;
    vector<CC*> cc = mmodel->GetCCModels();
    for(CCIT ic=cc.begin(); ic!=cc.end(); ic++)
      (*ic)->ScaleEdges(_t);
  }

  if(m_obj.m_debugModel != NULL){
    DebugModel* debugModel = m_obj.m_debugModel;
    vector<CC*>& cc = debugModel->GetMapModel()->GetCCModels();
    for(CCIT ic=cc.begin(); ic!=cc.end(); ic++)
      (*ic)->ScaleEdges(_t);
  }
}

void Vizmo::ChangeNodesShape(string _s){
  if(m_obj.m_robotModel==NULL)
    return;

  if(m_obj.m_mapModel==NULL && m_obj.m_debugModel==NULL)
    return;

  if(_s == "Robot")
    CfgModel::SetShape(CfgModel::Robot);
  if(_s == "Box")
    CfgModel::SetShape(CfgModel::Box);
  else
    CfgModel::SetShape(CfgModel::Point);

  typedef MapModel<CfgModel,EdgeModel> MM;
  typedef CCModel<CfgModel,EdgeModel> CC;
  typedef vector<CC*>::iterator CCIT;

  if(m_obj.m_mapModel!=NULL){
    MapModel<CfgModel,EdgeModel>* mmodel = m_obj.m_mapModel;
    vector<CC*>& cc=mmodel->GetCCModels();
    for(CCIT ic=cc.begin(); ic!=cc.end(); ic++){
      CfgModel::Shape shape=CfgModel::Point;
      if(_s=="Robot")
        shape=CfgModel::Robot;
      else if(_s=="Box")
        shape=CfgModel::Box;
      (*ic)->ChangeShape(shape);
    }
  }
  if(m_obj.m_debugModel!=NULL){
    DebugModel* debugModel = m_obj.m_debugModel;
    vector<CC*>& cc=debugModel->GetMapModel()->GetCCModels();
    for(CCIT ic=cc.begin();ic!=cc.end();ic++){
      CfgModel::Shape shape = CfgModel::Point;
      if(_s=="Robot")
        shape=CfgModel::Robot;
      else if(_s=="Box")
        shape=CfgModel::Box;
      (*ic)->ChangeShape(shape);
    }
  }
}

void Vizmo::ChangeCCColor(double _r, double _g, double _b, string _s){

  if(m_obj.m_robotModel == NULL)
    return;

  if(m_obj.m_mapModel == NULL && m_obj.m_debugModel == NULL)
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

  if(m_obj.m_mapModel!=NULL){
    MapModel<CfgModel,EdgeModel>* mmodel = m_obj.m_mapModel;
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
  if(m_obj.m_debugModel!=NULL){
    DebugModel* debugModel = m_obj.m_debugModel;
    vector<CC*>& cc=debugModel->GetMapModel()->GetCCModels();
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

void Vizmo::UpdateSelection(){


  if( m_obj.m_robotModel==NULL )
    return;

  if( m_obj.m_mapModel==NULL && m_obj.m_debugModel==NULL)
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

    if(m_obj.m_mapModel!=NULL){
      MapModel<CfgModel,EdgeModel>* mmodel = m_obj.m_mapModel;
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

    if(m_obj.m_debugModel!=NULL){
      DebugModel* debugModel = m_obj.m_debugModel;
      vector<CC*>& cc = debugModel->GetMapModel()->GetCCModels();
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


void Vizmo::ChangeNodeColor(double _r, double _g, double _b, string _s){

  if(m_obj.m_robotModel == NULL)
    return;

  if(m_obj.m_mapModel == NULL)
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

    MapModel<CfgModel,EdgeModel>* mmodel = m_obj.m_mapModel;
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


void Vizmo::ChangeNodesRandomColor(){
  if(!m_obj.m_robotModel || !m_obj.m_mapModel)
    return;

  typedef CCModel<CfgModel,EdgeModel> CC;
  typedef vector<CC*>::iterator CCIT;

  //change color
  MapModel<CfgModel,EdgeModel>* mmodel = m_obj.m_mapModel;
  vector<CC*>& cc  =mmodel->GetCCModels();
  for(CCIT ic = cc.begin(); ic != cc.end(); ++ic){
    float r = drand48(), g = drand48(), b = drand48();
    (*ic)->SetColor(r, g, b, 1);
    (*ic)->DrawRobotNodes((*ic)->m_renderMode);
  }
}

bool Vizmo::StringToInt(const string &s, int &i){
  istringstream myStream(s);
  return myStream >> i;
}

void Vizmo::envObjsRandomColor(){
  m_obj.m_envModel->ChangeColor();
}

double Vizmo::GetEnvRadius(){
  return m_obj.m_envModel ? m_obj.m_envModel->GetRadius() : 200;
}

void
Vizmo::PlaceRobot(){
  RobotModel* r = m_obj.m_robotModel;
  if(r){
    vector<double> cfg;
    if(m_obj.m_queryModel)
      cfg = m_obj.m_queryModel->GetStartGoal(0);
    else if(m_obj.m_pathModel)
      cfg = m_obj.m_pathModel->GetConfiguration(0);
    else
      cfg = vector<double>(CfgModel::GetDOF());

    if(m_obj.m_debugModel || (m_obj.m_mapModel && !(m_obj.m_pathModel || m_obj.m_queryModel)))
      r->SetRenderMode(INVISIBLE_MODE);

    if(!cfg.empty()) {
      //copy initial cfg. to RobotModel
      r->Configure(cfg);
      r->InitialCfg(cfg);
    }
  }
}

void Vizmo::getRoboCfg(){
  vector<gliObj>& sel=GetSelectedItem();
  typedef vector<gliObj>::iterator SIT;
  for(SIT i=sel.begin();i!=sel.end();i++){
    GLModel * gl=(GLModel *)(*i);
    info=gl->GetInfo();
  }
}

int Vizmo::getNumJoints(){
  return m_obj.m_robotModel ? m_obj.m_robotModel->getNumJoints() : -1;
}
///////////////////////////////////////////////////////////////////////////////
// Private Functions
///////////////////////////////////////////////////////////////////////////////
/*

   string Vizmo::FindName
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
Vizmo::EnvChanged(){

  m_envChanged = false;

  EnvModel* envModel = m_obj.m_envModel;
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

void
Vizmo::SetMapObj(MapModel<CfgModel,EdgeModel>* _mm){
  m_obj.m_mapModel = _mm;
}
