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
using namespace plum;

//////////////////////////////////////////////////////////////////////
// Include OBPRMViewer headers
#include "EnvObj/PathLoader.h"
#include "EnvObj/PathModel.h"
#include "EnvObj/BoundingBoxModel.h"
#include "EnvObj/BoundingBoxParser.h"
#include "EnvObj/Robot.h"
#include "EnvObj/QueryLoader.h"
#include "EnvObj/QueryModel.h"

//////////////////////////////////////////////////////////////////////
// Qt header to change CC's color
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qapplication.h>
#include <qaction.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qtoolbutton.h>
#include <qwidget.h>
#include <qinputdialog.h>
#include <qcolordialog.h> 

//////////////////////////////////////////////////////////////////////
//Define singleton
vizmo g_vizmo2;
vizmo& GetVizmo(){ return g_vizmo2; }

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
    
    if( m_Env!=NULL ){
        delete m_Env->getModel(); delete m_Env->getLoader(); delete m_Env;
    }
    
    if( m_Map!=NULL ){
        delete m_Map->getModel(); delete m_Map->getLoader(); delete m_Map;
    }
    m_Robot=m_BBox=m_Qry=m_Path=m_Env=m_Map=NULL;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

vizmo::vizmo()
{
	m_obj.m_show_BBox=true;
	m_obj.m_show_Qry=false; 
	m_obj.m_show_Path=false;       
	m_obj.m_show_Map=false; 
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

    if( envname.empty() ) envname=name+".env";
    if( FileExits(envname) ){ m_obj.m_EnvFile=envname;}
	else m_obj.m_EnvFile="";
    //guess path and query file name
    if( FileExits(name+".path") ){m_obj.m_PathFile=name+".path";}
	else m_obj.m_PathFile="";
    if( FileExits(name+".query") ){m_obj.m_QryFile=name+".query";}
	else m_obj.m_QryFile="";
}

bool vizmo::InitVizmoObject()
{
    //delete old stuff
    m_Plum.Clean();
    m_obj.Clean();
    
    //create env first
    string name;
    name=m_obj.m_EnvFile; //FindName("env",filenames);
    if( !name.empty() ){
        if( !CreateEnvObj(m_obj,name) ) return false;
        cout<<"Load Environment File : "<<name<<endl;
    }
    
    //create robot
    if( !CreateRobotObj(m_obj) ) return false;
    
    //create map
    name=m_obj.m_MapFile;//FindName("map",filenames);
    if( !name.empty() ){
        if( !CreateMapObj(m_obj,name) ) return false;
        cout<<"Load Map File : "<<name<<endl;
    }

    //create path
    name=m_obj.m_PathFile;//FindName("path",filenames);
    if( !name.empty() ){
        if( !CreatePathObj(m_obj,name) ) return false;
        cout<<"Load Path File : "<<name<<endl;
    }
    
    //create qry
    name=m_obj.m_QryFile;//FindName("query",filenames);
    if( !name.empty() ){
        if( !CreateQueryObj(m_obj,name) ) return false;
        cout<<"Load Query File : "<<name<<endl;
    }
    
    //create bbx
    if( !CreateBBoxObj(m_obj) ) return false;
    
    //add all of them into plum
    
    m_Plum.AddPlumObject(m_obj.m_BBox);
    m_Plum.AddPlumObject(m_obj.m_Robot);
    m_Plum.AddPlumObject(m_obj.m_Env);
    m_Plum.AddPlumObject(m_obj.m_Path);
    m_Plum.AddPlumObject(m_obj.m_Qry);
    //m_Plum.AddPlumObject(m_obj.m_Env);
    m_Plum.AddPlumObject(m_obj.m_Map);


    //let plum do what he needs to do
    if( m_Plum.ParseFile()==CPlumState::PARSE_ERROR ){return false;}
    if( m_Plum.BuildModels()!=CPlumState::BUILD_MODEL_OK ){return false;}

    //put robot in start cfg, if availiable
    PlaceRobot();

    // Init. variables used to change color of env. objects
    mR = mG = mB = 0;

	//setup visibility
	ShowRoadMap(m_obj.m_show_Map);
	ShowPathFrame(m_obj.m_show_Path);
	ShowQueryFrame(m_obj.m_show_Qry);
	ShowBBox(m_obj.m_show_BBox);

    return true;
}

void vizmo::RefreshEnv()
{
    if(m_obj.m_Env==NULL) return;
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

  int dof = CCfg::dof;
  m_IsNode = true;
  vector<double> dataCfg;
  dataCfg = cfg->GetDataCfg();
  m_nodeCfg = new double[dof];
  
  for(int i=0;i<dof;i++){
    m_nodeCfg[i] = dataCfg[i];;
  }
}

void vizmo::TurnOn_CD(){

  string objName;

  vector<gliObj>& sel=GetVizmo().GetSelectedItem();
  typedef vector<gliObj>::iterator OIT;
  for(OIT i=sel.begin();i!=sel.end();i++){
    objName = ((CGLModel*)(*i))->GetName();
  }
  //cout<<"\n\nOBJECT SELECTED:: " << objName<< endl;
  CEnvModel* env=(CEnvModel*)m_obj.m_Env->getModel();
  CEnvLoader* envLoader=(CEnvLoader*)m_obj.m_Env->getLoader();
  int MBnum = envLoader->GetNumberOfMultiBody();

  OBPRMView_Robot* robot=(OBPRMView_Robot*)m_obj.m_Robot->getModel();

  list<CGLModel*> robotList,modelList;
  //obtain robot model	  
  robot->GetChildren(modelList);
  //cout<<"ROBOT MODEL LIST SIZE:: "<<modelList.size()<<endl;
  CMultiBodyModel * robotModel = (CMultiBodyModel*)modelList.front();

  //If we'll test a node, copy Cfg to CD class
  //cout<<"vizmo::TurnOn_CD():: m_IsNode = "<<m_IsNode<<endl;
  if(m_IsNode){
    int dof = CCfg::dof;
    CD.CopyNodeCfg(m_nodeCfg, dof);
  }

  if(objName != "Node"){
    m_IsNode = false;
    //CD.TestNode = false;
  }

  bool b = false;
  b = CD.IsInCollision(MBnum, env, envLoader, robotModel, robot);

  if (b){
    
    if(m_cfg != NULL){
      m_cfg->coll = true;
      //m_IsNode = false;
    }
  }
  else{
    
    if(m_cfg != NULL){
      m_cfg->coll = false;
      //m_IsNode = false;
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

}

void vizmo::SaveQryCfg(char ch){
  
  typedef vector<gliObj>::iterator GIT;
  string name;
  CGLModel * gl;
  //to store a single cfg
  vector<double *> cfg;

  OBPRMView_Robot* robot=(OBPRMView_Robot*)m_obj.m_Robot->getModel();

  for(GIT ig= GetSelectedItem().begin();ig!=GetSelectedItem().end();ig++)
  {
    gl=(CGLModel *)(*ig);
    list<string> info=gl->GetInfo();
    name = info.front();
  }

  if(name == "Robot"){
    int dof = CCfg::dof;
    //get original Cfgs from QueryLoader
    CQueryLoader * q=(CQueryLoader*)m_obj.m_Qry->getLoader();
    unsigned int iQSize = q->GetQuerySize();
    
    for( unsigned int iQ=0; iQ<iQSize; iQ++ ){
      double * Cfg = new double[dof];
      Cfg = q->GetStartGoal(iQ); 
      cfg.push_back(Cfg);
    }

    robot->SaveQry(cfg, ch);
  }

}

bool vizmo::SaveQry(const char *filename){
  int dof = CCfg::dof;
  vector<double *> cfg;
  FILE *qryFile;
  
  OBPRMView_Robot* robot=(OBPRMView_Robot*)m_obj.m_Robot->getModel();
  vector<double *> vSG = robot->getNewStartAndGoal();
  
  if(!vSG.empty()){
    //open file
    if((qryFile = fopen(filename, "a")) == NULL){
      cout<<"Couldn't open the file"<<endl;
      return 0;
    }
    //get values
    typedef vector<double *>::iterator IC;
    for(IC ic=vSG.begin(); ic!=vSG.end(); ic++){
      double * c = (double *)(*ic);
      for(int i=0; i<dof; i++){
	//printf("%2f ", c[i]);
	fprintf(qryFile, "%2f ", c[i]);
      }
      fprintf(qryFile, "\n");
    }
    fclose(qryFile);
  }
  
}

void vizmo::ShowRoadMap( bool bShow ){
    m_obj.m_show_Map=bShow;
    if( m_obj.m_Map==NULL ) return;
    CGLModel * m=m_obj.m_Map->getModel();
    
    if( bShow )
        m->SetRenderMode(CPlumState::MV_SOLID_MODE);
    else
        m->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);   

}

void vizmo::ShowPathFrame( bool bShow ){
	m_obj.m_show_Path=bShow;
    if( m_obj.m_Path==NULL ) return;
    CGLModel * m=m_obj.m_Path->getModel();
    
    if( bShow )
        m->SetRenderMode(CPlumState::MV_SOLID_MODE);
    else
        m->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);
}

void vizmo::ShowQueryFrame(bool bShow){
	m_obj.m_show_Qry=bShow;
    if( m_obj.m_Qry==NULL ) return;
    CGLModel * m=m_obj.m_Qry->getModel();
    if ( bShow )
        m->SetRenderMode(CPlumState::MV_SOLID_MODE);
    else
        m->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);
} 

// Code to show or hide bouding box!!!!
// BSS

void vizmo::ShowBBox(bool bShow){
    m_obj.m_show_BBox=bShow;
    if(m_obj.m_BBox==NULL) return;
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
    
    for(GIT ig= GetSelectedItem().begin();ig!=GetSelectedItem().end();ig++)
    {
        CGLModel *model=(CGLModel *)(*ig);
        if(status==0)
            model->SetRenderMode(CPlumState::MV_SOLID_MODE);
        else if(status==1)
            model->SetRenderMode(CPlumState::MV_WIRE_MODE);
        else if(status==2){
	    model->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);
	    CMultiBodyModel * mbl;
	    mbl=(CMultiBodyModel*)(*ig);
	    DeleteObject(mbl);
	}
	else if(status == 3)
	  model->SetColor( mR, mG, mB, 1 );
    }
}

void vizmo::DeleteObject(CMultiBodyModel *mbl){

  CEnvLoader* envLoader=(CEnvLoader*)m_obj.m_Env->getLoader();
  int MBnum = envLoader->GetNumberOfMultiBody();

  const CMultiBodyInfo * mbi;
  CMultiBodyInfo *mbiTmp;
  mbiTmp = new CMultiBodyInfo [MBnum];
  mbi = envLoader->GetMultiBodyInfo();

  int j=0;
  for(int i=0; i<MBnum; i++){
    if( (mbi[i].m_pBodyInfo[0].m_strModelDataFileName !=
	 mbl->m_MBInfo.m_pBodyInfo[0].m_strModelDataFileName) ||
	(mbi[i].m_pBodyInfo[0].m_X != mbl->m_MBInfo.m_pBodyInfo[0].m_X )||
	(mbi[i].m_pBodyInfo[0].m_Y != mbl->m_MBInfo.m_pBodyInfo[0].m_Y)||
	(mbi[i].m_pBodyInfo[0].m_Z != mbl->m_MBInfo.m_pBodyInfo[0].m_Z) ){
      
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
    if( m_obj.m_Robot==NULL || m_obj.m_Path==NULL )
        return;
    CPathLoader* ploader=(CPathLoader*)m_obj.m_Path->getLoader();
    OBPRMView_Robot* rmodel=(OBPRMView_Robot*)m_obj.m_Robot->getModel();
   
    double * dCfg;
    //Get Cfg

    dCfg=ploader->GetConfiguration(frame);

    //reset robot's original position
    ResetRobot();

    rmodel->Configure(dCfg);
    delete dCfg;
}

int vizmo::GetPathSize(){ 
    if(m_obj.m_Path==NULL) return 0; 
    CPathLoader* ploader=(CPathLoader*)m_obj.m_Path->getLoader();
    return ploader->GetPathSize();
}

void vizmo::ChangeNodesSize(float s, string str){
    
    if( m_obj.m_Robot==NULL ) return;
    
    if( m_obj.m_Map==NULL ) return;
    
    typedef CMapModel<CCfg,CSimpleEdge> MM;
    typedef CCModel<CCfg,CSimpleEdge> CC;
    typedef vector<CC*>::iterator CCIT;

    CMapModel<CCfg,CSimpleEdge>* mmodel =(MM*)m_obj.m_Map->getModel();
    vector<CC*>& cc=mmodel->GetCCModels();
    for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
        CC::Shape shape=CC::Point;
        if( str=="Robot" ) shape=CC::Robot;
        else if( str=="Box" ) shape=CC::Box;
          (*ic)->scaleNode(s, shape);
    }
}

void vizmo::ChangeNodesShape(string s){

    if( m_obj.m_Robot==NULL ) return;
    
    if( m_obj.m_Map==NULL ) return;
    

    typedef CMapModel<CCfg,CSimpleEdge> MM;
    typedef CCModel<CCfg,CSimpleEdge> CC;
    typedef vector<CC*>::iterator CCIT;

    CMapModel<CCfg,CSimpleEdge>* mmodel =(MM*)m_obj.m_Map->getModel();
    vector<CC*>& cc=mmodel->GetCCModels();

    for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
        CC::Shape shape=CC::Point;
        if( s=="Robot" ) shape=CC::Robot;
        else if( s=="Box" ) shape=CC::Box;
          (*ic)->changeShape(shape);
    }
}


void vizmo::WriteNodesInfo(ostream& _myostream, CMapLoader<CCfg,CSimpleEdge>::WG * g){
    if( m_obj.m_Robot==NULL ) return;
    
    if( m_obj.m_Map==NULL ) return;
    
    typedef CMapLoader<CCfg,CSimpleEdge>::WG WG;
    WG * graph;
    graph = g;

    double data;
    int numNodes;
    int indx=0, n_indx = 0;

    _myostream << endl << "#####GRAPHSTART#####";
    _myostream << endl << graph->size() << " " << graph->GetEdgeCount() << " " << graph->size();

    numNodes = graph->size();
      for(int j=0; j<numNodes; j++){
       
	_myostream << endl;
	_myostream <<  j << " ";
	_myostream <<graph->GetData(j)<< " ";
	graph->WriteNewGraph(_myostream, j);
      }	

      _myostream << endl << "#####GRAPHSTOP#####";

}



void vizmo::ChangeNodesColor(double r, double g, double b, string s){

    if( m_obj.m_Robot==NULL ) return;
    
    if( m_obj.m_Map==NULL ) return;

    typedef CMapModel<CCfg,CSimpleEdge> MM;
    typedef CCModel<CCfg,CSimpleEdge> CC;
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
    int position = m_sO.find("CC",0);
    if(position != string::npos){
      m_s = m_sO.substr(position+2, m_sO.length());
    }   

    CMapModel<CCfg,CSimpleEdge>* mmodel =(MM*)m_obj.m_Map->getModel();
    vector<CC*>& cc=mmodel->GetCCModels();
    if(m_s != "NULL"){
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
	CC::Shape shape=CC::Point;
	if( s=="Robot" ) shape=CC::Robot;
       	else if( s=="Box" ) shape=CC::Box;
	if(StringToInt(m_s, m_i)){
	  if(m_i == (*ic)->id){
	    (*ic)->DrawSelect();
	    (*ic)->newColor = true;
	    (*ic)->changeColor(r, g, b, shape);
	  }
	}
      }
    }
    else if(m_s == "NULL" && oneColor){
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
	CC::Shape shape=CC::Point;
	if( s=="Robot" ) shape=CC::Robot;
	else if( s=="Box" ) shape=CC::Box;
	(*ic)->newColor = true;
	(*ic)->changeColor(r, g, b, shape);
      }
      oneColor = false;
    }
    else{
      for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
	CC::Shape shape=CC::Point;
	if( s=="Robot" ) shape=CC::Robot;
	else if( s=="Box" ) shape=CC::Box;
	(*ic)->newColor = true;
	r = ((float)rand())/RAND_MAX; 
	g = ((float)rand())/RAND_MAX; 
        b = ((float)rand())/RAND_MAX; 
	(*ic)->changeColor(r, g, b, shape);
      }
    }
}

void vizmo::ChangeNodesRandomColor(){
	
  if( m_obj.m_Robot==NULL ) return;
  if( m_obj.m_Map==NULL ) return;

  typedef CMapModel<CCfg,CSimpleEdge> MM;
  typedef CCModel<CCfg,CSimpleEdge> CC;
  typedef vector<CC*>::iterator CCIT; 
  
  //change color
  CMapModel<CCfg,CSimpleEdge>* mmodel =(MM*)m_obj.m_Map->getModel();
  vector<CC*>& cc=mmodel->GetCCModels();

  for( CCIT ic=cc.begin();ic!=cc.end();ic++ ){
    float r = ((float)rand())/RAND_MAX; 
    float g = ((float)rand())/RAND_MAX; 
    float b = ((float)rand())/RAND_MAX; 
    (*ic)->SetColor(r,g,b,1);
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
  //CMapLoader<CSimpleCfg,CSimpleEdge> * mloader=new CMapLoader<CSimpleCfg,CSimpleEdge>();
  //CMapModel<CSimpleCfg,CSimpleEdge> * mmodel = new CMapModel<CSimpleCfg,CSimpleEdge>();
    CMapLoader<CCfg,CSimpleEdge> * mloader=new CMapLoader<CCfg,CSimpleEdge>();
    CMapModel<CCfg,CSimpleEdge> * mmodel = new CMapModel<CCfg,CSimpleEdge>();
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
    CPathLoader * ploader=new CPathLoader();
    CPathModel * pmodel=new CPathModel();
    if( ploader==NULL || pmodel==NULL ) return false;
    ploader->SetDataFileName(fname);
    pmodel->SetPathLoader(ploader);
    if( obj.m_Robot!=NULL )
        pmodel->SetModel((OBPRMView_Robot *)obj.m_Robot->getModel());
    obj.m_Path=new PlumObject(pmodel,ploader);
    return (obj.m_Path!=NULL);
}

bool vizmo::CreateQueryObj( vizmo_obj& obj, const string& fname )
{
    CQueryLoader * qloader=new CQueryLoader();
    CQueryModel * qmodel=new CQueryModel();
    if( qloader==NULL || qmodel==NULL ) return false;
    qloader->SetDataFileName(fname);
    qmodel->SetQueryLoader(qloader);
    if( obj.m_Robot!=NULL )
        qmodel->SetModel((OBPRMView_Robot *)obj.m_Robot->getModel());
    obj.m_Qry=new PlumObject(qmodel,qloader);
    return (obj.m_Qry!=NULL);
}

bool vizmo::CreateBBoxObj( vizmo_obj& obj )
{
    if( obj.m_Map==NULL ) return true; //can't build
    CMapHeaderLoader * maploader=(CMapHeaderLoader*)obj.m_Map->getLoader();
    if( maploader->ParseHeader()==false ) return false;
    const string command=maploader->GetPreamble();
    CBoundingBoxParser * bloader=new CBoundingBoxParser();
    CBoundingBoxModel * bmodel=new CBoundingBoxModel();
    if( bloader==NULL || bmodel==NULL ) return false;
    //well, this time we view filename as command
    bloader->SetDataFileName(command);
    bmodel->SetBBXParser(bloader);
    obj.m_BBox=new PlumObject(bmodel,bloader);
    return (obj.m_BBox!=NULL);
}

bool vizmo::CreateRobotObj( vizmo_obj& obj )
{
    if( m_obj.m_Env==NULL ) return true; //can't build
    
    CEnvLoader* envLoader=(CEnvLoader*)m_obj.m_Env->getLoader();
    OBPRMView_Robot * r=new OBPRMView_Robot(envLoader);
    if( r==NULL ) return false;
    obj.m_Robot=new PlumObject(r,NULL);
   return (obj.m_Robot!=NULL);
}

void vizmo::PlaceRobot()
{
    OBPRMView_Robot * r=(OBPRMView_Robot*)m_obj.m_Robot->getModel();
    if( r!=NULL ){
        double * cfg=NULL;
        if( m_obj.m_Qry!=NULL ){//check query loader
            CQueryLoader * q=(CQueryLoader*)m_obj.m_Qry->getLoader();
            cfg=q->GetStartGoal(0);
        }
        else if( m_obj.m_Path!=NULL ){//check path loader
            CPathLoader * p=(CPathLoader*)m_obj.m_Path->getLoader();
            cfg=p->GetConfiguration(0);
        }
        if( cfg!=NULL){
            r->Configure(cfg);
	    //copy initial cfg. to OBPRMView_Robot
	    r->InitialCfg(cfg);
            delete [] cfg;
        }
    }
}

void vizmo::ResetRobot(){

  OBPRMView_Robot * r=(OBPRMView_Robot*)m_obj.m_Robot->getModel();
  if( r!=NULL )
    r->RestoreInitCfg();
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
 if(pos==string::npos) continue; //not . found
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
