#include "Robot.h"


OBPRMView_Robot::OBPRMView_Robot(CEnvLoader * pEnv)
{
    m_pEnvLoader=pEnv;
    m_RobotModel=NULL;
    //size = 1.0;
}

OBPRMView_Robot::OBPRMView_Robot(const OBPRMView_Robot &other_robot)
:CGLModel(other_robot)
{
    m_pEnvLoader=other_robot.getEnvLoader();
    m_RobotModel = other_robot.getRobotModel();
    //size = other_robot.size;
}

CEnvLoader * OBPRMView_Robot::getEnvLoader() const {
    return m_pEnvLoader;
}

CMultiBodyModel * OBPRMView_Robot::getRobotModel() const {
    return m_RobotModel;
}

OBPRMView_Robot::~OBPRMView_Robot()
{
    delete m_RobotModel;
}

bool OBPRMView_Robot::BuildModels(){
    
    //find robot name
    const CMultiBodyInfo * pMInfo = m_pEnvLoader->GetMultiBodyInfo();
    int numMBody=m_pEnvLoader->GetNumberOfMultiBody();
    
    //create MB for robot.
    int iM=0;
    for( ; iM<numMBody; iM++ ){
      if( pMInfo[iM].m_active ){
	if(pMInfo[iM].m_pBodyInfo[0].m_bIsFixed)
	  m_RobotModel=new CMultiBodyModel(iM,pMInfo[0]);
	else
	  m_RobotModel=new CMultiBodyModel(iM,pMInfo[iM]);
      }
    }
    
    if( m_RobotModel==NULL ){return false;}
    m_RobotModel->setAsFree(); //set as free body
    SetColor(1,0,0,1);
    return m_RobotModel->BuildModels();
}

void OBPRMView_Robot::Draw(GLenum mode){
  if( m_RobotModel==NULL || GL_RENDER!=mode ) return;
  glPushMatrix();
  glTransform();
  m_RobotModel->Draw(GL_RENDER);
  glPopMatrix();
}

void OBPRMView_Robot::BackUp(){

  Poly = m_RobotModel->GetPolyhedron();
  const CMultiBodyInfo * MBInfo = m_pEnvLoader->GetMultiBodyInfo();
  int numMBody=MBInfo[0].m_cNumberOfBody;
  pPolyBackUp = new CPolyhedronModel[numMBody];

  for( int iM=0; iM<numMBody; iM++ ){
      pPolyBackUp[iM].tx()=Poly[iM].tx();
      pPolyBackUp[iM].ty()=Poly[iM].ty();
      pPolyBackUp[iM].tz()=Poly[iM].tz();
      pPolyBackUp[iM].q(Poly[iM].q());
    }

   R=Poly[0].GetColor()[0];
   G=Poly[0].GetColor()[1];
   B=Poly[0].GetColor()[2];
   o_s[0]=sx(); o_s[1]=sy();o_s[2]=sz();

}

void OBPRMView_Robot::Restore(){

  const CMultiBodyInfo * MBInfo = m_pEnvLoader->GetMultiBodyInfo();
  int numMBody=MBInfo[0].m_cNumberOfBody;
 
 for( int iM=0; iM<numMBody; iM++ ){
      Poly[iM].tx()=pPolyBackUp[iM].tx();
      Poly[iM].ty()=pPolyBackUp[iM].ty();
      Poly[iM].tz()=pPolyBackUp[iM].tz();
      Poly[iM].q(pPolyBackUp[iM].q());
    }

  this->Scale(o_s[0],o_s[1],o_s[2]);
  this->SetColor(R,G,B,this->GetColor()[3]);   

  //delete [] Poly;
  delete [] pPolyBackUp;

}


void OBPRMView_Robot::Configure( double * cfg) { 
  const CMultiBodyInfo * MBInfo = m_pEnvLoader->GetMultiBodyInfo();
  int numMBody=m_pEnvLoader->GetNumberOfMultiBody();
  int robIndx = 0;
  
  CPolyhedronModel * pPoly = m_RobotModel->GetPolyhedron();

  Vector3d position;
  Orientation orientation;

  for( int iM=0; iM<numMBody; iM++ ){
    if( MBInfo[iM].m_active )
      robIndx = iM;
  }

  int numBody=MBInfo[robIndx].m_cNumberOfBody;

  for( int iM=0; iM<numBody; iM++ ){
    MBInfo[robIndx].m_pBodyInfo[iM].m_currentTransform.ResetTransformation();
    MBInfo[robIndx].m_pBodyInfo[iM].m_prevTransform.ResetTransformation();
    MBInfo[robIndx].m_pBodyInfo[iM].m_transformDone = false;
  }

  if(MBInfo[robIndx].m_pBodyInfo[0].m_bIsFixed){

    pPoly[0].tx()=MBInfo[robIndx].m_pBodyInfo[0].m_X; 
    pPoly[0].ty()=MBInfo[robIndx].m_pBodyInfo[0].m_Y; 
    pPoly[0].tz()=MBInfo[robIndx].m_pBodyInfo[0].m_Z;

    position.set(MBInfo[robIndx].m_pBodyInfo[0].m_X,
		 MBInfo[robIndx].m_pBodyInfo[0].m_Y,
		 MBInfo[robIndx].m_pBodyInfo[0].m_Z );

    Orientation orientation_tmp(Orientation::FixedXYZ,
			    MBInfo[robIndx].m_pBodyInfo[0].m_Alpha,
			    MBInfo[robIndx].m_pBodyInfo[0].m_Beta,
			    MBInfo[robIndx].m_pBodyInfo[0].m_Gamma);
    orientation = orientation_tmp;
  }

  else{

    pPoly[0].tx()=cfg[0]; 
    pPoly[0].ty()=cfg[1]; 
    pPoly[0].tz()=cfg[2];
    
    position.set(cfg[0], cfg[1], cfg[2]);
    Orientation orientation_tmp(Orientation::FixedXYZ, cfg[3], cfg[4], cfg[5]);
    orientation = orientation_tmp;
  }

  Transformation b(orientation, position);
  MBInfo[robIndx].m_pBodyInfo[0].m_currentTransform = b;
  
  //compute rotation
  Quaternion q1, q2;
  q2 = q1.getQuaternionFromMatrix(
	  MBInfo[robIndx].m_pBodyInfo[0].m_currentTransform.m_orientation.matrix);
  pPoly[0].q(q2.normalize()); //set new q
  
  MBInfo[robIndx].m_pBodyInfo[0].m_transformDone = true;
  
  //configuration of links after the base
  if( MBInfo[robIndx].m_NumberOfConnections!=0 ){
    //Compute position and orientation for all of the links left
    for( int iM=0; iM<MBInfo[robIndx].m_NumberOfConnections; iM++ ){
      
      int currentBody = 0; //index of current Body
      int nextBody = 0; //indext of next Body
      double theta; //to keep angle of current Cfg.
      
      if(MBInfo[robIndx].m_pBodyInfo[0].m_bIsFixed)
	theta = cfg[iM];
      else
	theta = cfg[6+iM]; //base already computed from cfg0,...,cfg5
      
      currentBody = MBInfo[robIndx].listConnections[iM].first;
      nextBody = MBInfo[robIndx].listConnections[iM].second;
      
      if(!MBInfo[robIndx].m_pBodyInfo[nextBody].m_transformDone){
	
	Transformation t = MBInfo[robIndx].m_pBodyInfo[currentBody].getTransform(); 
	MBInfo[robIndx].m_pBodyInfo[nextBody].setPrevTransform(t); 
	MBInfo[robIndx].m_pBodyInfo[nextBody].computeTransform(
	      MBInfo[robIndx].m_pBodyInfo[currentBody], nextBody, theta);
	
	pPoly[nextBody].tx()=MBInfo[robIndx].m_pBodyInfo[nextBody].m_currentTransform.m_position[0]; 
	pPoly[nextBody].ty()=MBInfo[robIndx].m_pBodyInfo[nextBody].m_currentTransform.m_position[1]; 
	pPoly[nextBody].tz()=MBInfo[robIndx].m_pBodyInfo[nextBody].m_currentTransform.m_position[2];  
	  
	//compute rotation
	Quaternion q1, q2;
	q2 = q1.getQuaternionFromMatrix(
	       MBInfo[robIndx].m_pBodyInfo[nextBody].m_currentTransform.m_orientation.matrix);
	pPoly[nextBody].q(q2.normalize()); //set new q
	
	MBInfo[robIndx].m_pBodyInfo[nextBody].m_transformDone = true;
      }    
    }
  }
}

void OBPRMView_Robot::DrawSelect()
{
  if( m_RobotModel==NULL ) return;
  glPushMatrix();
  glTransform();
  m_RobotModel->DrawSelect();
  glPopMatrix();
}

