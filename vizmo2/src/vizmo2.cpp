// OBPRMView.cpp: implementation of the vizmo class.
//
//////////////////////////////////////////////////////////////////////

#include "vizmo2.h"
#include <limits.h>

//////////////////////////////////////////////////////////////////////
// Include Plum headers
#include <MapObj/SimpleCfg.h>
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
//Define singleton
vizmo g_vizmo2;
vizmo& GetVizmo(){ return g_vizmo2; }

//////////////////////////////////////////////////////////////////////
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
}

vizmo::~vizmo()
{
}

//////////////////////////////////////////////////////////////////////
// Core 
//////////////////////////////////////////////////////////////////////
vector<string> vizmo::GetAccessFiles(const string& filename)
{
    vector<string> names; //the result, empty now
    int pos=filename.rfind('.');
    if( pos==string::npos ) return names;
    string name=filename.substr(0,pos);
    //test if files exist
    string envname; //env file name
    string mapname=name+".map";
    if( FileExits(mapname) ){ //check map and get env file name
        names.push_back(mapname);
        //parse header of map to get env filename
        CMapHeaderLoader maploader;
        maploader.SetDataFileName(mapname);
        maploader.ParseFile();
        envname=maploader.GetEnvFileName();
    }
    if( envname.empty() ) envname=name+".env";
    if( FileExits(envname) ) names.push_back(envname);
    //guess path and query file name
    if( FileExits(name+".path") ) names.push_back(name+".path");
    if( FileExits(name+".query") ) names.push_back(name+".query");

    return names;
}

bool vizmo::InitVizmoObject( const vector<string>& filenames )
{
    //delete old stuff
    m_Plum.Clean();
    m_obj.Clean();

    //create env first
    string name;
    name=FindName("env",filenames);
    if( !name.empty() ){
        if( !CreateEnvObj(m_obj,name) ) return false;
        cout<<"Load Environment File : "<<name<<endl;
    }

    //create map
    name=FindName("map",filenames);
    if( !name.empty() ){
        if( !CreateMapObj(m_obj,name) ) return false;
        cout<<"Load Map File : "<<name<<endl;
    }

    //create robot
    if( !CreateRobotObj(m_obj) ) return false;

    //create path
    name=FindName("path",filenames);
    if( !name.empty() ){
        if( !CreatePathObj(m_obj,name) ) return false;
        cout<<"Load Path File : "<<name<<endl;
    }

    //create qry
    name=FindName("query",filenames);
    if( !name.empty() ){
        if( !CreateQueryObj(m_obj,name) ) return false;
        cout<<"Load Query File : "<<name<<endl;
    }

    //create bbx
    if( !CreateBBoxObj(m_obj) ) return false;

    //add all of them into plum
    m_Plum.AddPlumObject(m_obj.m_BBox);
    m_Plum.AddPlumObject(m_obj.m_Env);
    m_Plum.AddPlumObject(m_obj.m_Robot);
    m_Plum.AddPlumObject(m_obj.m_Path);
    m_Plum.AddPlumObject(m_obj.m_Qry);
    m_Plum.AddPlumObject(m_obj.m_Map);

    //let plum do what he needs to do
    if( m_Plum.ParseFile()==CPlumState::PARSE_ERROR ) return false;
    if( m_Plum.BuildModels()!=CPlumState::BUILD_MODEL_OK ) return false;
    return true;
}

/*
void vizmo::DumpSelected(){
m_Plum.DumpSelected();
}
*/




void vizmo::refreshEnv()
{
  if(m_obj.m_Env==NULL) return;
  CGLModel *m=m_obj.m_Env->getModel();

  m->SetRenderMode(CPlumState::MV_SOLID_MODE);
}

void vizmo::ShowRoadMap( bool bShow ){

    if( m_obj.m_Map==NULL ) return;
    CGLModel * m=m_obj.m_Map->getModel();

    if( bShow )
        m->SetRenderMode(CPlumState::MV_SOLID_MODE);
    else
        m->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);       
}

void vizmo::ShowPathFrame( bool bShow ){
    if( m_obj.m_Path==NULL ) return;
    CGLModel * m=m_obj.m_Path->getModel();

    if( bShow )
        m->SetRenderMode(CPlumState::MV_SOLID_MODE);
    else
        m->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);
}

void vizmo::ShowQueryFrame( bool bShow){
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
    // status 2 = invisible

    typedef vector<gliObj>::iterator GIT;

    for(GIT ig= GetSelectedItem().begin();ig!=GetSelectedItem().end();ig++)
      {
	CGLModel *model=(CGLModel *)(*ig);
	if(status==0)
	  model->SetRenderMode(CPlumState::MV_SOLID_MODE);
	else if(status==1)
	  model->SetRenderMode(CPlumState::MV_WIRE_MODE);
	else if(status==2)
	  model->SetRenderMode(CPlumState::MV_INVISIBLE_MODE);
      }

}


void vizmo::Animate(bool bForward){
    if( m_obj.m_Robot==NULL || m_obj.m_Path==NULL )
        return;
    CPathLoader* ploader=(CPathLoader*)m_obj.m_Path->getLoader();
    OBPRMView_Robot* rmodel=(OBPRMView_Robot*)m_obj.m_Robot->getModel();

    double * dCfg;
    //Get Cfg
    if( bForward )
        dCfg=ploader->GetNextConfigure(true);
    else
        dCfg=ploader->GetPreviousConfigure(true);
    rmodel->Configure(dCfg);
    delete dCfg;
   
}


void vizmo::GetConfiguration(int index)
{

  if(m_obj.m_Robot==NULL || m_obj.m_Path==NULL)
  {
    cout<<"Exiting here";
    return;
  }

  CPathLoader* ploader=(CPathLoader*)m_obj.m_Path->getLoader();
  OBPRMView_Robot* rmodel=(OBPRMView_Robot*)m_obj.m_Robot->getModel();

  double * dCfg; 

  dCfg=ploader->GetParticularConfiguration(index);
  rmodel->Configure(dCfg);
  delete dCfg;

}



// BSS returns the number of frames
int vizmo::getTimer()
{
  int size;
  if(m_obj.m_Path==NULL) 
    {
    return 0;
    cout<<flush<<endl<<size<<endl;
    }
   
   CPathLoader* ploader=(CPathLoader*)m_obj.m_Path->getLoader();
   size=(signed int)ploader->GetPathSize();
   
  
  return size-1;
   
}

int vizmo::GetCurrentCfg()
{
  int cfg;
  if(m_obj.m_Path==NULL) 
    {
      return 0;
    
    }
   CPathLoader* ploader=(CPathLoader*)m_obj.m_Path->getLoader();
   cfg=(signed int)ploader->GetCurrentCfg();
   
   return cfg;
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
    obj.m_Map=createMapObj<CSimpleCfg,CSimpleEdge>(fname);
    return (obj.m_Map!=NULL);
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

///////////////////////////////////////////////////////////////////////////////
// Private Functions
///////////////////////////////////////////////////////////////////////////////
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

bool vizmo::FileExits(const string& filename) const
{
    ifstream fin(filename.c_str());
    bool result=fin.good();
    fin.close();
    return result;
}










