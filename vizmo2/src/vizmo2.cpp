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
    if( FileExits(envname) ){ m_obj.m_EnvFile=envname; }
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

bool vizmo::SaveEnv(const char *filename)
{
  CEnvLoader* envLoader=(CEnvLoader*)m_obj.m_Env->getLoader();
  const CMultiBodyInfo* MBI = envLoader->GetMultiBodyInfo();

//   vector<CPolyhedronModel *> pPoly; 

//   pPoly = env->getPoly();

//   int numPoly=pPoly.size();
//   cout<<"Number of Polys: "<<numPoly<<endl;
//   for(int iP=0; iP<numPoly; iP++ ){
//     printf("Position:: %1f, %1f, %1f\n ",pPoly[iP]->tx(), 
// 	   pPoly[iP]->ty(), pPoly[iP]->tz());
    //printf("Orientation:: %1f, %1f, %1f\n ",pPoly[iP]->rx(), 
    // pPoly[iP]->ry(),pPoly[iP]->rz());
    //pPoly[iP].tx()
    //pPoly[iP].ty();
    //pPoly[iP].tz();
    //pPoly[iP].rx();
    //pPoly[iP].ry();
    //pPoly[iP].rz();

    // }

  //get polyhedron for

  //CMultiBodyModel * m_Model = rmodel->getRobotModel();;
  //CPolyhedronModel * pPoly = m_Model ->GetPolyhedron();

  FILE *envFile;
  if((envFile = fopen(filename, "a")) == NULL){
    cout<<"Couldn't open the file"<<endl;
    return 0;
  }
  int MBnum = envLoader->GetNumberOfMultiBody();//number of objects in env.
  //write num. of Bodies
  fprintf(envFile,"%d\n\n", MBnum );

  CEnvModel* env=(CEnvModel*)m_obj.m_Env->getModel();

  //getMBody() and then current position and orientation
  vector<CMultiBodyModel *> MBmodel = env->getMBody();
  //  int numMB = MBmodel.size();
  //cout<<"Number of MBodies: "<<MBmodel.size()<<endl;
  //cout<<"----------------------------------------------------"<<endl;
//   for(int iMB=0; iMB<numMB; iMB++ ){

//     printf("\nX: %1f\t", MBmodel[iMB]->tx());
//     printf("Y: %1f\t", MBmodel[iMB]->ty());
//     printf("Z: %1f\n", MBmodel[iMB]->tz());
//     printf("A: %1f\t", MBmodel[iMB]->rx());
//     printf("B: %1f\t", MBmodel[iMB]->ry());
//     printf("G: %1f\n", MBmodel[iMB]->rz());
//     printf("=========================================");

//   }

  for(int i = 0; i<MBnum; i++){ //for each body in *.env

    if(MBI[i].m_active)
      fprintf(envFile,"Multibody   Active\n");
    else
      fprintf(envFile,"Multibody   Passive\n");

    if(MBI[i].m_cNumberOfBody != 0){
      int nB = MBI[i].m_cNumberOfBody;
      //write Num. of Bodies in the current MultiBody
      fprintf(envFile,"%d\n", nB);
      for(int j = 0; j<nB; j++){
	
	if(MBI[i].m_pBodyInfo[j].m_bIsFixed)
	  fprintf(envFile,"FixedBody    "); 
	else
	  fprintf(envFile,"FreeBody    ");

	fprintf(envFile,"%d  ",MBI[i].m_pBodyInfo[j].m_Index);
	string s_tmp = MBI[i].m_pBodyInfo[j].m_strModelDataFileName;
	const char* st;
	st = s_tmp.c_str();
	char *pos = strrchr(st, '/');
	int position = pos-st+1;
	string sub_string = s_tmp.substr(position);

	const char* f;
	f = sub_string.c_str();

	if(!MBI[i].m_active){
	  string sub = "./BYUdata/" + sub_string;
	  f = sub.c_str();
	  fprintf(envFile,"%s  ",f);
	}
	else
	  fprintf(envFile,"%s  ",f);
// 	fprintf(envFile,"%.1f %.1f %.1f %.1f %.1f %.1f\n",
// 		MBI[i].m_pBodyInfo[j].m_X, MBI[i].m_pBodyInfo[j].m_Y,
// 		MBI[i].m_pBodyInfo[j].m_Z, MBI[i].m_pBodyInfo[j].m_Alpha,
// 		MBI[i].m_pBodyInfo[j].m_Beta, MBI[i].m_pBodyInfo[j].m_Gamma);

	fprintf(envFile,"%.1f %.1f %.1f %.1f %.1f %.1f\n",
		MBmodel[i]->tx(), MBmodel[i]->ty(), MBmodel[i]->tz(),
		MBmodel[i]->rx()*57.29578, 
		MBmodel[i]->ry()*57.29578, 
		MBmodel[i]->rz()*57.29578);	
      }
      //write Connection tag
      if(MBI[i].m_NumberOfConnections !=0)
	fprintf(envFile,"\nConnection\n"); 
      else
	fprintf(envFile,"Connection\n"); 

      fprintf(envFile,"%d\n", MBI[i].m_NumberOfConnections); 	  

      //write Connection info.
      if(MBI[i].m_NumberOfConnections !=0){
	const char* str;
	int numConn = MBI[i].listConnections.size();

	for(int l=0; l<numConn; l++){
	  int indexList = MBI[i].listConnections[l].first;
	  if(MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo->m_actuated)
	    str= "Actuated";
	  else
	    str = "NonActuated";

	  fprintf(envFile,"%d %d  %s\n",MBI[i].listConnections[l].first,
		  MBI[i].listConnections[l].second, str);

	  //get info. from current Body and current connection
	  int index;
	  for(int b=0; 
	      b<MBI[i].m_pBodyInfo[indexList].m_cNumberOfConnection; b++){
	    
	    int n = MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[b].m_nextIndex;

	    if( MBI[i].listConnections[l].second == n){
	      index = b;
	      break;
	    }
	  }

	  string s_tmp = MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_articulation;
	  const char* f = s_tmp.c_str();
	  
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_posX);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_posY);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_posZ);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orientX*57.29578);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orientY* 57.29578);
	  fprintf(envFile, "%.1f\t",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orientZ* 57.29578);

	  fprintf(envFile, "%.1f ", 
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].alpha);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].a);
	  fprintf(envFile, "%.1f ",
		    MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].d);
	  fprintf(envFile, "%.1f        ",
		    MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_theta);
	  fprintf(envFile, "%s        ", f);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_pos2X);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_pos2Y);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_pos2Z);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orient2X* 57.29578);
	  fprintf(envFile, "%.1f ",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orient2Y* 57.29578);
	  fprintf(envFile, "%.1f\n\n",
		  MBI[i].m_pBodyInfo[indexList].m_pConnectionInfo[index].m_orient2Z* 57.29578);
	}
	
      }

    }
    fprintf(envFile,"\n");
  }
  
  fclose(envFile);

}

void vizmo::SaveQryStart(){

  int dof = CCfg::dof;
  //to store a single cfg
  vector<double *> cfg;
  //to store actual cfg
  double *currCfg = new double[dof];
  //to store cfg. to be sent to Robot
  vector<double *> vCfg;

  typedef vector<gliObj>::iterator GIT;
  string name;
  CGLModel * gl;
  CGLModel * rl;

  double TwoPI=3.1415926535*2.0;

  OBPRMView_Robot* robot=(OBPRMView_Robot*)m_obj.m_Robot->getModel();

  list<CGLModel*> robotList,modelList;
  //obtain robot model	  
  robot->GetChildren(modelList);
  CMultiBodyModel * robotModel = (CMultiBodyModel*)modelList.front();
  //get robot polyhedron
  robotModel->GetChildren(robotList);

  for(GIT ig= GetSelectedItem().begin();ig!=GetSelectedItem().end();ig++)
  {
    gl=(CGLModel *)(*ig);
    list<string> info=gl->GetInfo();
    cout<<"TYPE:: "<<info.front()<<", "<<gl->GetName()<<endl;
    name = info.front();
  }

  if(name == "Robot"){
    
    //get original Cfgs from QueryLoader
    CQueryLoader * q=(CQueryLoader*)m_obj.m_Qry->getLoader();
    unsigned int iQSize = q->GetQuerySize();
    
    for( unsigned int iQ=0; iQ<iQSize; iQ++ ){
      double * Cfg = new double[dof];
      Cfg = q->GetStartGoal(iQ); 
      cfg.push_back(Cfg);
    }
    
    int dof_left = dof - 6;
    if(dof_left == 0){ //rigid body
      
      //if user didn't move robot by hand, this means
      //the animation tool was used
      if(gl->tx() == 0 && gl->ty() == 0 && gl->tz() == 0){
	cout<<"+++++++ user used ToolBar to move robot +++++++++"<<endl;
	typedef list<CGLModel *>::iterator RI;
	
	for(RI i=robotList.begin(); i!=robotList.end(); i++){
	  rl = (CGLModel*)(*i);
	  currCfg[0] = rl->tx();
	  currCfg[1] = rl->ty();
	  currCfg[2] = rl->tz();
	  currCfg[3] = rl->rx();
	  currCfg[4] = rl->ry();
	  currCfg[5] = rl->rz();
	}
      }
      else{ //user moved robot by hand
	cout<<"+++++++ user moved robot by hand +++++++++"<<endl;
	Matrix3x3 m = gl->getMatrix();
	Vector3d vRot;
	vRot = gl->MatrixToEuler(m);

	if(gl->m_PosPoly[0] != -1){
	  //robot has been moved
	currCfg[0] = gl->tx() + gl->m_PosPoly[0];
	currCfg[1] = gl->ty() + gl->m_PosPoly[1];
	currCfg[2] = gl->tz() + gl->m_PosPoly[2];

	//Need to compute rotation from Quaternion
	
	CGLModel* rm = robotList.front();
	//get current quaternion from polyhedron0
	Quaternion qt0;
	qt0 = rm->q();
	Matrix3x3 pm = qt0.getMatrix();
	Vector3d pv = qt0.MatrixToEuler(pm);

	//get new rotation from GL
	Quaternion qrm;
	qrm = gl->q();

	//multiply polyhedron0 and multiBody quaternions
	//to get new rotation
	Quaternion finalQ;
	finalQ = qrm * qt0;

	//set new rotation angles to multiBody rx(), ry(), and rz()

	Matrix3x3 mFinal; Vector3d vFinal;
	mFinal = finalQ.getMatrix();
	vFinal = finalQ.MatrixToEuler(mFinal);

	robotModel->rx() = vFinal[0]/TwoPI;
	robotModel->ry() = vFinal[1]/TwoPI;
	robotModel->rz() = vFinal[2]/TwoPI;
	
	//set new angles for first polyhedron
	//NOTE:: This works for **FREE** robots

	currCfg[3] =  robotModel->rx();
	currCfg[4] =  robotModel->ry();
	currCfg[5] =  robotModel->rz();
 
	}
	else{
	  //robot has not been moved before
	  currCfg[0] = gl->tx() + cfg[0][0];
	  currCfg[1] = gl->ty() + cfg[0][1];
	  currCfg[2] = gl->tz() + cfg[0][2];
	  currCfg[3] = vRot[0]/TwoPI;
	  currCfg[4] = vRot[1]/TwoPI;
	  currCfg[5] = vRot[2]/TwoPI;
	}

      }
      
      vCfg.push_back(currCfg);
      robot->storeCfg(vCfg, 's', dof);
    }
    
    else{//articulated

      //if robot is moved using the animation bar
      //then get Cfg. from robot->Configure
      //and add current displacement tx(), ty() and tz()
      if(gl->tx() == 0 && gl->ty() == 0 && gl->tz() == 0){
	currCfg = robot->returnCurrCfg(dof);

      }

      //if robot is moved by hand, gl->tx(), gl->ty(), and gl->tz()
      //have translation of polyhedron0
 
     else{

	currCfg = robot->returnCurrCfg(dof);
	//add translation of MBody
	currCfg[0] = currCfg[0] + gl->tx();
	currCfg[1] = currCfg[1] + gl->ty();
	currCfg[2] = currCfg[2] + gl->tz();

	//Need to compute rotation from Quaternion
	
	CGLModel* rm = robotList.front();
	//get current quaternion from polyhedron0
	Quaternion qt0;
	qt0 = rm->q();
	Matrix3x3 pm = qt0.getMatrix();
	Vector3d pv = qt0.MatrixToEuler(pm);

	//get new rotation from multiBody 
	Quaternion qrm;
	qrm = robotModel->q();

	//multiply polyhedron0 and multiBody quaternions
	//to get new rotation
	Quaternion finalQ;
	finalQ = qrm * qt0;

	//set new rotation angles to multiBody rx(), ry(), and rz()

	Matrix3x3 mFinal; Vector3d vFinal;
	mFinal = finalQ.getMatrix();
	vFinal = finalQ.MatrixToEuler(mFinal);

	robotModel->rx() = vFinal[0]/TwoPI;
	robotModel->ry() = vFinal[1]/TwoPI;
	robotModel->rz() = vFinal[2]/TwoPI;
	
	//set new angles for first polyhedron
	//NOTE:: This works for **FREE** robots

	currCfg[3] =  robotModel->rx();
	currCfg[4] =  robotModel->ry();
	currCfg[5] =  robotModel->rz();
 
	
      }

      vCfg.push_back(currCfg);
      robot->storeCfg(vCfg, 's', dof);

    }//else articulated
    
  }//if Robot
  
}

void vizmo::SaveQryGoal(){
  
  int dof = CCfg::dof;
  //to store a single cfg
  vector<double *> cfg;
  //to store actual cfg
  double *currCfg = new double[dof];
  //to store cfg. to be sent to Robot
  vector<double *> vCfg;

  typedef vector<gliObj>::iterator GIT;
  string name;
  CGLModel * gl;
  CGLModel * rl;

  double TwoPI=3.1415926535*2.0;

  OBPRMView_Robot* robot=(OBPRMView_Robot*)m_obj.m_Robot->getModel();

  list<CGLModel*> robotList, modelList;
  robot->GetChildren(modelList);
  CMultiBodyModel * robotModel = (CMultiBodyModel*)modelList.front();
  //get robot polyhedron
  robotModel->GetChildren(robotList);

  for(GIT ig= GetSelectedItem().begin();ig!=GetSelectedItem().end();ig++)
  {
    gl=(CGLModel *)(*ig);
    list<string> info=gl->GetInfo();
    cout<<"TYPE:: "<<info.front()<<", "<<gl->GetName()<<endl;
    name = info.front();
  }
  if(name == "Robot"){
    
    //get original Cfgs from QueryLoader
    CQueryLoader * q=(CQueryLoader*)m_obj.m_Qry->getLoader();
    unsigned int iQSize = q->GetQuerySize();
    
    for( unsigned int iQ=0; iQ<iQSize; iQ++ ){
      double * Cfg = new double[dof];
      Cfg = q->GetStartGoal(iQ); 
      cfg.push_back(Cfg);
    }
    
    int dof_left = dof - 6;
    if(dof_left == 0){ //rigid body
      
      //if user didn't move robot by hand
      if(gl->tx() == 0 && gl->ty() == 0 && gl->tz() == 0){
	typedef list<CGLModel *>::iterator RI;
	
	for(RI i=robotList.begin(); i!=robotList.end(); i++){
	  rl = (CGLModel*)(*i);
	  currCfg[0] = rl->tx();
	  currCfg[1] = rl->ty();
	  currCfg[2] = rl->tz();
	  currCfg[3] = rl->rx();
	  currCfg[4] = rl->ry();
	  currCfg[5] = rl->rz();
	}
      }
      else{ //user moved robot by hand
	Matrix3x3 m = gl->getMatrix();
	Vector3d vRot;
	vRot = gl->MatrixToEuler(m);

	if(gl->m_PosPoly[0] != -1){
	  //robot has been moved
	currCfg[0] = gl->tx() + gl->m_PosPoly[0];
	currCfg[1] = gl->ty() + gl->m_PosPoly[1];
	currCfg[2] = gl->tz() + gl->m_PosPoly[2];

	//Need to compute rotation from Quaternion
	
	CGLModel* rm = robotList.front();
	//get current quaternion from polyhedron0
	Quaternion qt0;
	qt0 = rm->q();
	Matrix3x3 pm = qt0.getMatrix();
	Vector3d pv = qt0.MatrixToEuler(pm);

	//get new rotation from GL
	Quaternion qrm;
	qrm = gl->q();

	//multiply polyhedron0 and multiBody quaternions
	//to get new rotation
	Quaternion finalQ;
	finalQ = qrm * qt0;

	//set new rotation angles to multiBody rx(), ry(), and rz()

	Matrix3x3 mFinal; Vector3d vFinal;
	mFinal = finalQ.getMatrix();
	vFinal = finalQ.MatrixToEuler(mFinal);

	robotModel->rx() = vFinal[0]/TwoPI;
	robotModel->ry() = vFinal[1]/TwoPI;
	robotModel->rz() = vFinal[2]/TwoPI;
	
	//set new angles for first polyhedron
	//NOTE:: This works for **FREE** robots

	currCfg[3] =  robotModel->rx();
	currCfg[4] =  robotModel->ry();
	currCfg[5] =  robotModel->rz();
 
	}
	else{//robot has not been moved before
	currCfg[0] = gl->tx() + cfg[0][0];
	currCfg[1] = gl->ty() + cfg[0][1];
	currCfg[2] = gl->tz() + cfg[0][2];

	currCfg[3] = vRot[0]/TwoPI;
	currCfg[4] = vRot[1]/TwoPI;
	currCfg[5] = vRot[2]/TwoPI;
	}
      }
     
      vCfg.push_back(currCfg);
      robot->storeCfg(vCfg, 'g', dof);
    }
    
    else{//articulated

      //if robot is moved using the animation bar
      //then get Cfg. from robot->Configure
      //and add current displacement tx(), ty() and tz()
      if(gl->tx() == 0 && gl->ty() == 0 && gl->tz() == 0){
	currCfg = robot->returnCurrCfg(dof);
      }

      //if robot is moved by hand, gl->tx(), gl->ty(), and gl->tz()
      //have translation of polyhedron0
 
     else{

	currCfg = robot->returnCurrCfg(dof);
	//add translation of MBody
	currCfg[0] = currCfg[0] + gl->tx();
	currCfg[1] = currCfg[1] + gl->ty();
	currCfg[2] = currCfg[2] + gl->tz();

	//Need to compute rotation from Quaternion
	CGLModel* rm = robotList.front();

	//get current quaternion from polyhedron0
	Quaternion qt0;
	qt0 = rm->q();
	Matrix3x3 pm = qt0.getMatrix();
	Vector3d pv = qt0.MatrixToEuler(pm);

	//get new rotation from multiBody 
	Quaternion qrm;
	qrm = robotModel->q();

	//multiply polyhedron0 and multiBody quaternions
	//to get new rotation
	Quaternion finalQ;
	finalQ = qrm * qt0;

	//set new rotation angles to multiBody rx(), ry(), and rz()

	Matrix3x3 mFinal; Vector3d vFinal;
	mFinal = finalQ.getMatrix();
	vFinal = finalQ.MatrixToEuler(mFinal);

	robotModel->rx() = vFinal[0]/TwoPI;
	robotModel->ry() = vFinal[1]/TwoPI;
	robotModel->rz() = vFinal[2]/TwoPI;
	
	//set new angles for first polyhedron
	//NOTE:: This works for **FREE** robots

	currCfg[3] =  robotModel->rx();
	currCfg[4] =  robotModel->ry();
	currCfg[5] =  robotModel->rz();

     }

      vCfg.push_back(currCfg);
      robot->storeCfg(vCfg, 'g', dof);

    }//else articulated

  }//if Robot
    
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
	printf("%2f ", c[i]);
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
    // status 2 = invisible
    // status 3 = change color

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
	else if(status == 3)
	  model->SetColor( mR, mG, mB, 1 );
    }
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

//void vizmo::ChangeNodesSize(float s){
void vizmo::ChangeNodesSize(float s, string str){
    
    //cout<<"Size parameter: "<<s<<endl;
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

  //cout<<"S RECEIVED IN VIZMO2::"<<s<<endl;
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

    cout<<"ENV created"<<endl;

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
