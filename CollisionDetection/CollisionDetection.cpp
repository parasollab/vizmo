/*******************************************************************/
////////////////////////////////////////////////////////////////////
//
// Implementation of Collision Detection class
//
///////////////////////////////////////////////////////////////////
/*******************************************************************/

#include "CollisionDetection.h"
#include <math.h>
#include <GL/gliFont.h>

//////////////////////////////
// Constructor / Destructor
//////////////////////////////

CollisionDetection::CollisionDetection(){

  m_rapid = new RAPID_model();
  TestNode = false;

}

CollisionDetection::~CollisionDetection(){

  delete m_rapid;

}

bool CollisionDetection::IsInCollision(plum::CEnvLoader* envLoader, 
				       plum::CMultiBodyModel* robot, 
				       plum::CMultiBodyModel* obstacle){

    return true;
}


bool CollisionDetection::IsInCollision(int numMB, 
				       plum::CEnvModel* env, 
				       plum::CEnvLoader* envLoader,
				       plum::CMultiBodyModel * robotModel,
				       OBPRMView_Robot * robotObj) {


  vector<CMultiBodyModel *> MBmodel = env->getMBody();
  const CMultiBodyInfo* MBI = envLoader->GetMultiBodyInfo();
  int dof = envLoader->getDOF();
  int robIndx = 0;
  bool collision = false;
  bool value = false;
  CMultiBodyModel *m_robot;

  ////// Making a copy of the robot
  /// use this new robot to work...

  OBPRMView_Robot* robotCpy(robotObj);

  list<CGLModel*> robotList,modelList;
  //obtain robot model	  
  robotCpy->GetChildren(modelList);
  CMultiBodyModel * m_robotModel = (CMultiBodyModel*)modelList.front();
 

  //get current color of robot
  //  CGLModel* m_gl = modelList.front();
  //const float * curr_color = m_gl->GetColor();

  rapid = new Rapid();
  if(TestNode){
    rapid->test_node = true;
    rapid->RCopyNodeCfg(nodeCfg, dof);
    robotCpy->BackUp();
    robotCpy->Configure(nodeCfg);
  }

////////////////////////////////////////////////
// look for index of robot
////////////////////////////////////////////////

  //look for the robot 
  for(int i = 0; i<numMB; i++){ 
    if(MBI[i].m_active){ 
      //if this MBody is ROBOT
      m_robot =  MBmodel[i];
      robIndx = i;
      break;
    } 
  }

  //rapid->DoF = envLoader->getDOF();
 
  for(int i=0; i<numMB; i++){
    if(i != robIndx){
      collision = rapid->IsInCollision(m_robotModel, robotCpy, dof, MBmodel[i]);
      if(collision){
	value = true;
      }
    }
  }

  if(TestNode){
    robotCpy->Restore(); 
  }
 

  //rapid->test_node = false;

  if(!value && !rapid->test_node){
    const CMultiBodyInfo rMBInfo = robotModel->m_MBInfo;
    int NumBodiesRob= rMBInfo.m_cNumberOfBody;
    CPolyhedronModel * rPoly = robotModel->GetPolyhedron();
    int c;
    for(c=0; c<NumBodiesRob; c++){
      //rPoly[c].SetColor(1, 0, 0, 1);
      rPoly[c].SetColor(robotCpy->Get_R(),robotCpy->Get_G(),robotCpy->Get_B(), 1);
    }
   }

  rapid->test_node = false;
  TestNode = false;

  return value;
  
}

//called from vizmo::TurnOn_CD()
void CollisionDetection::CopyNodeCfg(double * cfg, int dof){
  TestNode = true;
  nodeCfg = new double[dof];
  for(int i=0; i<dof; i++){
    nodeCfg[i] = cfg[i];
  }
}

/*******************************************************************/
/////////////////////////////////////////////////////////////////
//
// Implementation of Rapid class
//
/////////////////////////////////////////////////////////////////
/*******************************************************************/

void Rapid::RCopyNodeCfg(double * n_cfg, int dof){
  nodeCfg = new double[dof];
  for(int i=0;i<dof;i++){
    nodeCfg[i] = n_cfg[i];
  }
}

bool Rapid::IsInCollision(CMultiBodyModel * robot, 
			  OBPRMView_Robot * robotObj, int dof, 
			  CMultiBodyModel * obstacle){

  bool is_collision = false;

  double mR[3][3], mO[3][3], pR[3],  pO[3];
  double pRt[3];
  double TwoPI= M_PI * 2;
  RAPID_model *RMobst, *RMrobot;

 
  const CMultiBodyInfo rMBInfo = robot->m_MBInfo;
  const CMultiBodyInfo oMBInfo = obstacle->m_MBInfo;

  int NumBodiesRob= rMBInfo.m_cNumberOfBody;
 
  int NumBodiesObs= oMBInfo.m_cNumberOfBody;

  //to store rotation matrix for moving objs.
  Matrix3x3 m;
  Vector3d fv;


  /*****************************************************
   **  For each robot's body, create its rapid model 
   **  generate those models just for free bodies

   **  Then test each robot's body for collision against
   **  each obstacle's bodies
   ******************************************************/

  CPolyhedronModel * rPoly = robot->GetPolyhedron();
  CPolyhedronModel * oPoly = obstacle->GetPolyhedron();

  //////////////////////////////////////////////////////////////////////////
  //
  // Compute position and orientation of firts link, then compute
  // for the rest of the bodies
  //
  ////////////////////////////////////////////////////////////////////////
  if(test_node){
    fv[0] = nodeCfg[3]; fv[1] = nodeCfg[4]; fv[2] = nodeCfg[5];
  }
  
  for(int i=0; i<NumBodiesRob; i++){
    //get RAPID model for ROBOT
    RMrobot = rPoly[i].GetRapidModel();
    
    for(int j=0; j<NumBodiesObs; j++){
      //get RAPID model for OBSTACLE
      RMobst = oPoly[j].GetRapidModel();
      
      //Get Position and orientation Matrix for robot and obstacle   
      //Get those values from Tranformation IF the robot was 
      //moved with the animation tool   
      //use tx, ty, tz, rx, ry, rz variables otherwise
      if(!test_node){
	pRt[0] = rPoly[0].tx()+ robot->tx();
	pRt[1] = rPoly[0].ty()+ robot->ty();
	pRt[2] = rPoly[0].tz()+ robot->tz();
	//rotation link0
	//Need to compute rotation from Quaternion
		
	list<CGLModel*> modelList;
	CGLModel* gl;
	robot->GetChildren(modelList);
	gl = modelList.front();
	//get initial quaternion from polyhedron
	Quaternion qt0;
	qt0 = gl->q();
	
	//get current/new rotation from objs var.
	Quaternion qrm = robot->q();
	
	//multiply polyhedron0 and multiBody quaternions
	//to get new rotation
	Quaternion finalQ;
	finalQ = qrm * qt0;
	
	Matrix3x3 fm = finalQ.getMatrix();
	//Vector3d fv = qt0.MatrixToEuler(fm);
	fv = qt0.MatrixToEuler(fm);
	
	if(i == 0){
	  pR[0] = pRt[0]; pR[1] = pRt[1]; pR[2] = pRt[2];
	  Orientation r_obst(Orientation::FixedXYZ,
			       fv[0],fv[1],fv[2]);
	  m = r_obst.matrix;
	}
	else{
	  ////////////////////////////////////////////////////////////////
	  // Since the moving object has been moved by hand, we need to
	  // compute new position and orientation of the object.
	  // CALL: Robot->Configure()
	  // to compute those new values
	  // NOTE:: UPDATE  
	  //             Position (pR[]) & Orientation(m) in this section
	  ///////////////////////////////////////////////////////////////
	  int last_dof;
	  last_dof = dof - 6;
	  //IF THE MOVING OBJ. IS ARTICULATED
	  //THEN COMPUTE NEW POSITION AND ANGLES FOR EACH LINK
	  
	  if(last_dof != 0){
	    
	    double *CurrCfg = new double[dof];
	    CurrCfg[0] = pRt[0]; CurrCfg[1] = pRt[1];  CurrCfg[2] = pRt[2]; 
	    CurrCfg[3] = fv[0]/TwoPI; 
	    CurrCfg[4] = fv[1]/TwoPI; 
	    CurrCfg[5] = fv[2]/TwoPI;
	    
	    double * Cfg = new double[dof];
	    Cfg = robotObj->returnCurrCfg(dof);
	    
	    for(int c=6; c<dof; c++)
	      CurrCfg[c] = Cfg[c];

	    robotObj->BackUp();
	    robotObj->Configure(CurrCfg);
	 
	    pR[0] = rPoly[i].tx();
	    pR[1] = rPoly[i].ty();
	    pR[2] = rPoly[i].tz();
	    
	    Quaternion qtmp = rPoly[i].q();
	    Matrix3x3 mtmp = qtmp.getMatrix();
	    Vector3d vtmp = qtmp.MatrixToEuler(mtmp);
	    
	    Orientation r_obst(Orientation::FixedXYZ,
			       vtmp[0],vtmp[1],vtmp[2]);
	    
	    m = r_obst.matrix;
	    
	    robotObj->Restore();
	  }
	}
      }// ROBOT

      else{
	pRt[0] = nodeCfg[0]; pRt[1] = nodeCfg[1]; pRt[2] =  nodeCfg[2];

	//IF IS ARTICULATED
	if( i == 0 ){

	  pR[0] = pRt[0]; pR[1] = pRt[1]; pR[2] = pRt[2];

	  Orientation r_rob(Orientation::FixedXYZ,
			      fv[0], fv[1], fv[2]);
	  m = r_rob.matrix;

	}
	else{
	  pR[0] = rPoly[i].tx();
	  pR[1] = rPoly[i].ty();
	  pR[2] = rPoly[i].tz();
	  
	  Quaternion qtmp = rPoly[i].q();
	  Matrix3x3 mtmp = qtmp.getMatrix();
	  Vector3d vtmp = qtmp.MatrixToEuler(mtmp);
	  
	  Orientation r_rob(Orientation::FixedXYZ,
			    vtmp[0],vtmp[1],vtmp[2]);
	  m = r_rob.matrix;
	}     
      }// NODE
      
      //Copy Matrix to double[][]
      for(int a =0; a<3; a++){
	for(int b=0; b<3; b++){
	  mR[a][b] = m[a][b];
	}
      }
      
	
      //////////////////////////////////////////////////
      //                 OBSTACLE
      //////////////////////////////////////////////////

      pO[0] = obstacle->tx();
      pO[1] = obstacle->ty();
      pO[2] = obstacle->tz();
           
      //Set current orientation of Obstacle
      
      Orientation o_obst(Orientation::FixedXYZ,
			 obstacle->rx(), obstacle->ry(), obstacle->rz());

      //get matrix
      Matrix3x3 mObs = o_obst.matrix;

      for(int a =0; a<3; a++){
	for(int b=0; b<3; b++){
	  mO[a][b] = mObs[a][b];	
	}
      }
 
 
      if(RAPID_Collide(mR, pR, RMrobot, mO, pO, RMobst, 
		       RAPID_FIRST_CONTACT)){
	cout<<"ERROR calling RAPID_Collide() method"<<endl;
	exit(1);
      }
      if(RAPID_num_contacts){
//    	cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
//  	cout<<"LINK "<<i << "is in "<<endl;
//   	cout<<"Collision with obstacle "<<oPoly[0].GetName()<<endl;
//   	cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
	
	if(test_node == false){
	  for(int c=0; c<NumBodiesRob; c++){
	    robotObj->SetColor(0,1,0, 1);
	    rPoly[c].SetColor(0,1,0, 1);
	  }
	}	
	return true;
      }
    }//for j

  }//for i
    

  return false;

}
