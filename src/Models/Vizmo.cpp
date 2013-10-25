#include "Vizmo.h"

#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#include "CfgModel.h"
#include "DebugModel.h"
#include "Model.h"
#include "PathModel.h"
#include "QueryModel.h"
#include "RobotModel.h"
#include "Utilities/PickBox.h"

////////////////////////////////////////////////////////////////////////////////
//Define Vizmo singleton
////////////////////////////////////////////////////////////////////////////////
Vizmo vizmo;
Vizmo& GetVizmo(){return vizmo;}

////////////////////////////////////////////////////////////////////////////////
// Vizmo
////////////////////////////////////////////////////////////////////////////////
Vizmo::Vizmo() :
  m_envModel(NULL), m_robotModel(NULL),
  m_mapModel(NULL), m_showMap(false),
  m_queryModel(NULL), m_showQuery(false),
  m_pathModel(NULL), m_showPath(false),
  m_debugModel(NULL) {
    //temporary initialization of "unused" objects
    m_cfg = NULL;
    is_collison = false;
    m_isNode = false;
    mR = mG = mB = 0.0;
    m_doubleClick = false;
  }

Vizmo::~Vizmo() {
  Clean();
}

bool
Vizmo::InitModels() {
  Clean();

  try {
    //Create environment first
    if(m_envFilename.empty())
      throw ParseException(WHERE, "Vizmo must load an environment file.");

    m_envModel = new EnvModel(m_envFilename);
    m_loadedModels.push_back(m_envModel);

    //create robot
    m_robotModel = new RobotModel(m_envModel);
    m_loadedModels.push_back(m_robotModel);
    cout << "Load Environment File : "<< m_envFilename << endl;

    //Create map
    if(!m_mapFilename.empty()) {
      m_mapModel = new MapModel<CfgModel, EdgeModel>(m_mapFilename, m_robotModel);
      m_loadedModels.push_back(m_mapModel);
      cout << "Load Map File : " << m_mapFilename << endl;
    }

    //Create qry
    if(!m_queryFilename.empty()) {
      m_queryModel = new QueryModel(m_queryFilename, m_robotModel);
      m_loadedModels.push_back(m_queryModel);
      cout << "Load Query File : " << m_queryFilename << endl;
    }

    //Create path
    if(!m_pathFilename.empty()) {
      m_pathModel = new PathModel(m_pathFilename, m_robotModel);
      m_loadedModels.push_back(m_pathModel);
      cout << "Load Path File : " << m_pathFilename << endl;
    }

    //Create debug
    if(!m_debugFilename.empty()){
      m_debugModel = new DebugModel(m_debugFilename, m_robotModel);
      m_loadedModels.push_back(m_debugModel);
      cout << "Load Debug File : " << m_debugFilename << endl;
    }
  }
  catch(VizmoException& _e) {
    cerr << _e.what() << endl;
    cerr << "Cleaning vizmo objects." << endl;
    Clean();
    return false;
  }
  catch(exception& _e) {
    cerr << "Error::" << _e.what() << "\nExiting." << endl;
    exit(1);
  }

  //Put robot in start cfg, if availiable
  PlaceRobot();

  //Init. variables used to change color of env. objects
  mR = mG = mB = 0;

  return true;
}

void
Vizmo::Clean() {
  delete m_envModel;
  delete m_robotModel;
  delete m_mapModel;
  delete m_queryModel;
  delete m_pathModel;
  delete m_debugModel;
  m_envModel = NULL;
  m_robotModel = NULL;
  m_mapModel = NULL;
  m_queryModel = NULL;
  m_pathModel = NULL;
  m_debugModel = NULL;
  m_showMap = m_showQuery = m_showPath = false;
  m_loadedModels.clear();
  m_selectedModels.clear();
}

//Display OpenGL Scene
void
Vizmo::Display() {
  typedef vector<Model*>::iterator MIT;
  for(MIT mit = m_loadedModels.begin(); mit!=m_loadedModels.end(); ++mit)
    (*mit)->Draw(GL_RENDER);

  glColor3f(1,1,0); //Selections are yellow, so set the color once now
  for(MIT mit = m_selectedModels.begin(); mit != m_selectedModels.end(); ++mit)
    (*mit)->DrawSelect();
}

//Select Objects in OpenGL Scene
void
Vizmo::Select(const Box& _box) {
  GLuint hitBuffer[1024];
  GLint viewport[4];
  GLuint hits;

  // prepare for selection mode
  glSelectBuffer(1024, hitBuffer);
  glRenderMode(GL_SELECT);

  // get view port
  glGetIntegerv(GL_VIEWPORT, viewport);

  // initialize stack
  glInitNames();

  // change view volume
  glMatrixMode(GL_PROJECTION);

  double pm[16]; //current projection matrix
  glGetDoublev(GL_PROJECTION_MATRIX, pm);

  glPushMatrix();
  glLoadIdentity();

  double x = (_box.m_left + _box.m_right)/2;
  double y = (_box.m_top + _box.m_bottom)/2;
  double w = fabs(_box.m_right - _box.m_left); if(w<5) w=5;
  double h = fabs(_box.m_top - _box.m_bottom); if(h<5) h=5;

  gluPickMatrix(x, y, w, h, viewport);
  glMultMatrixd(pm); //apply current proj matrix

  //draw
  glMatrixMode(GL_MODELVIEW);
  typedef vector<Model*>::iterator MIT;
  for(MIT mit = m_loadedModels.begin(); mit != m_loadedModels.end(); ++mit) {
    glPushName(mit-m_loadedModels.begin());
    (*mit)->Draw(GL_SELECT);
    glPopName();
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  hits = glRenderMode(GL_RENDER);

  // unselect everything first
  m_selectedModels.clear();

  if(hits > 0)
    SearchSelectedItems(hits, hitBuffer, (w*h) > 100);
}

void
Vizmo::RefreshEnv(){
  if(m_envModel)
    m_envModel->SetRenderMode(SOLID_MODE);
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

  m_isNode = true;
  m_nodeCfg = cfg->GetDataCfg();
}

void Vizmo::TurnOn_CD(){

  string m_objName;

  for(MIT mit = m_selectedModels.begin(); mit != m_selectedModels.end(); ++mit)
    m_objName = (*mit)->Name();
  EnvModel* env = m_envModel;
  if(env != NULL){ //previously checked if loader was null

    RobotModel* robot = m_robotModel;

    list<Model*> robotList,modelList;
    //obtain robot model
    robot->GetChildren(modelList);
    MultiBodyModel* robotModel = (MultiBodyModel*)modelList.front();

    //If we'll test a node, copy CfgModel to CD class
    if(m_isNode){
      int dof = CfgModel::GetDOF();
      CD.CopyNodeCfg(m_nodeCfg, dof);
    }

    if(m_objName != "Node"){
      m_isNode = false;
    }

    bool b = false;
    b = CD.IsInCollision(0, env, robotModel, robot);
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
///////////////////////////////////////////////////

bool
Vizmo::SaveEnv(const char* _filename){
  m_envModel->SaveFile(_filename);
  return true;
}

void
Vizmo::SaveQuery(const string& _filename) {
  if(m_queryModel->GetQuerySize()) {
    ofstream ofs(_filename.c_str());
    for(size_t i = 0; i<m_queryModel->GetQuerySize(); ++i) {
      //output robot index. For now always a 0.
      ofs << "0 ";
      //output dofs
      typedef vector<double>::const_iterator DIT;
      const vector<double>& query = m_queryModel->GetStartGoal(i);
      for(DIT dit = query.begin(); dit != query.end(); ++dit)
        ofs << *dit << " ";
      ofs << endl;
    }
  }
}

void
Vizmo::ShowRoadMap(bool _show){
  m_showMap = _show;

  if(m_mapModel)
    m_mapModel->SetRenderMode(_show ? SOLID_MODE : INVISIBLE_MODE);
}

void
Vizmo::ShowPathFrame(bool _show){
  m_showPath = _show;

  if(m_pathModel)
    m_pathModel->SetRenderMode(_show ? SOLID_MODE : INVISIBLE_MODE);
}

void
Vizmo::ShowQueryFrame(bool _show){
  m_showQuery = _show;

  if(m_queryModel)
    m_queryModel->SetRenderMode(_show ? SOLID_MODE : INVISIBLE_MODE);
}

// Code To change the appearance of the env..
// BSS
void Vizmo::ChangeAppearance(int status)
{
  // status 0 = solid
  // status 1 = wire
  // status 2 = delete
  // status 3 = change color

  RobotModel* robot = m_robotModel;
  robot->BackUp();

  for(MIT mit = m_selectedModels.begin(); mit != m_selectedModels.end(); ++mit) {
    Model* model = *mit;
    if(status==0)
      model->SetRenderMode(SOLID_MODE);
    else if(status==1)
      model->SetRenderMode(WIRE_MODE);
    else if(status==2){
      model->SetRenderMode(INVISIBLE_MODE);
      DeleteObject((MultiBodyModel*)model);
    }
    else if(status == 3){
      if(model->Name() == "Robot"){
        robot->SetColor(Color4(mR, mG, mB, 1));
        robot->BackUp();
      }
      else
        model->SetColor(Color4(mR, mG, mB, 1));
    }
  }

  robot->Restore();
}

void Vizmo::DeleteObject(MultiBodyModel *mbl){
  /*
  EnvModel* envModel = m_envModel;
  int MBnum = envModel->GetNumMultiBodies();

  const MultiBodyInfo * mbi;
  MultiBodyInfo *mbiTmp;
  mbiTmp = new MultiBodyInfo [MBnum];
  mbi = envModel->GetMultiBodyInfo();

  int j=0;
  for(int i=0; i<MBnum; i++){
    if( (mbi[i].m_mBodyInfo[0].m_modelDataFileName !=
          mbl->GetMBinfo().m_mBodyInfo[0].m_modelDataFileName) ||
        (mbi[i].m_mBodyInfo[0].m_x != mbl->GetMBinfo().m_mBodyInfo[0].m_x )||
        (mbi[i].m_mBodyInfo[0].m_y != mbl->GetMBinfo().m_mBodyInfo[0].m_y)||
        (mbi[i].m_mBodyInfo[0].m_z != mbl->GetMBinfo().m_mBodyInfo[0].m_z) ){

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
  */
}

void Vizmo::Animate(int frame){
  if( m_robotModel==NULL || m_pathModel==NULL)
    return;

  PathModel* pathModel = m_pathModel;
  RobotModel* rmodel = m_robotModel;

  //Get Cfg
  vector<double> dCfg = pathModel->GetConfiguration(frame);

  rmodel->Configure(dCfg);
}

void Vizmo::AnimateDebug(int frame){
  if( m_robotModel==NULL || m_debugModel==NULL)
    return;
  DebugModel* debugModel = m_debugModel;

  debugModel->ConfigureFrame(frame);
}

int Vizmo::GetPathSize(){
  if(m_pathModel==NULL)
    return 0;

  PathModel* pathModel = m_pathModel;
  return pathModel->GetPathSize();
}

int Vizmo::GetDebugSize(){
  if(m_debugModel==NULL)
    return 0;
  DebugModel* debugModel = m_debugModel;
  return debugModel->GetDebugSize();
}

/*
void Vizmo::ChangeNodeColor(double _r, double _g, double _b, string _s){

  if(m_robotModel == NULL)
    return;

  if(m_mapModel == NULL)
    return;

  typedef MapModel<CfgModel,EdgeModel> MM;
  typedef CCModel<CfgModel,EdgeModel> CC;
  typedef vector<CC*>::iterator CCIT;

  //change color of one CC at a time
  int m_i;
  string m_sO;
  for(MIT mit = m_selectedModels.begin(); mit != m_selectedModels.end(); ++mit){
    Model* gl = *mit;
    m_sO = gl->GetName();

    string m_s="NULL";
    size_t position = m_sO.find("Node",0);
    if(position != string::npos){
      m_s = m_sO.substr(position+4, m_sO.length());
    }

    MapModel<CfgModel,EdgeModel>* mmodel = m_mapModel;
    vector<CC*>& cc=mmodel->GetCCModels();
    if(m_s != "NULL"){
      for(CCIT ic=cc.begin();ic!=cc.end();ic++){
        typedef map<CC::VID, CfgModel>::iterator NIT;
        for(NIT i = (*ic)->GetNodesInfo().begin(); i != (*ic)->GetNodesInfo().end(); i++)
          if(StringToInt(m_s, m_i)){
            if(m_i == i->second.GetIndex()){
              (*ic)->RebuildAll();
              i->second.SetColor(Color4(_r, _g, _b, 1));
              (*ic)->DrawRobotNodes((*ic)->m_renderMode);
              (*ic)->DrawSelect();
            }
          }
      }
    }
  }
}
*/
void
Vizmo::RandomizeCCColors(){

  if(!m_robotModel || !m_mapModel)
    return;

  typedef CCModel<CfgModel,EdgeModel> CC;
  typedef vector<CC*>::iterator CCIT;

  //change color
  MapModel<CfgModel,EdgeModel>* mmodel = m_mapModel;
  vector<CC*>& cc = mmodel->GetCCModels();
  for(CCIT ic = cc.begin(); ic != cc.end(); ++ic){
    (*ic)->SetColor(Color4(drand48(), drand48(), drand48(), 1));
  }
}

bool Vizmo::StringToInt(const string &s, int &i){
  istringstream myStream(s);
  return myStream >> i;
}

void Vizmo::RandomizeEnvColors(){

  m_envModel->ChangeColor();
}

double Vizmo::GetEnvRadius(){
  return m_envModel ? m_envModel->GetRadius() : 200;
}

void
Vizmo::PlaceRobot(){
  RobotModel* r = m_robotModel;
  if(r){
    vector<double> cfg;
    if(m_queryModel)
      cfg = m_queryModel->GetStartGoal(0);
    else if(m_pathModel)
      cfg = m_pathModel->GetConfiguration(0);
    else
      cfg = vector<double>(CfgModel::GetDOF());

    if(m_debugModel || (m_mapModel && !(m_pathModel || m_queryModel)))
      r->SetRenderMode(INVISIBLE_MODE);

    if(!cfg.empty()) {
      //copy initial cfg. to RobotModel
      r->Configure(cfg);
      r->SetInitialCfg(cfg);
    }
  }
}

//Parse the Hit Buffer. Store selected obj into m_selectedModels.
//hit is the number of hit by this selection
//buffer is the hit buffer
//if all, all obj select will be put into m_selectedItems,
//otherwise only the closest will be selected.
void
Vizmo::SearchSelectedItems(int _hit, void* _buffer, bool _all) {
  //init local data
  GLuint* ptr = (GLuint*)_buffer;
  unsigned int* selName = NULL;

  //input error
  if(!ptr)
    return;

  double z1; //near z for hit object
  double closeDistance = 1e3;

  for(int i=0; i<_hit; i++) {
    unsigned int* curName=NULL;
    GLuint nameSize = *ptr; ptr++;
    z1 = ((double)*ptr)/0x7fffffff; ptr++; //near z
    ptr++; //far z, we don't use this info

    curName = new unsigned int[nameSize];
    if(!curName)
      return;

    for(unsigned int iN=0; iN<nameSize; ++iN){
      curName[iN] = *ptr;
      ptr++;
    }

    if(!_all) {//not all
      if( z1<closeDistance ) {
        closeDistance = z1;     // set current nearset to z1
        delete [] selName;      //free preallocated mem
        if((selName=new unsigned int[nameSize])==NULL) return;
        memcpy(selName,curName,sizeof(unsigned int)*nameSize);
      }
    }
    else{ //select all
      if(curName[0] <= m_loadedModels.size()) {
        Model* selectModel = m_loadedModels[curName[0]];
        selectModel->Select(&curName[1], m_selectedModels);
      }
    }

    delete [] curName;  //free preallocated mem
  }

  //only the closest
  if(!_all) {
    // analyze selected item
    if(selName && selName[0] <= m_loadedModels.size()) {
      Model* selectModel = m_loadedModels[selName[0]];
      selectModel->Select(&selName[1], m_selectedModels);
    }
  }
  delete [] selName;
}

