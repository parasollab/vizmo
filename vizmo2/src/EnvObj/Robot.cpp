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
    int iM=0;
    for( ; iM<numMBody; iM++ ){
        if( !pMInfo[iM].m_pBodyInfo[0].m_bIsFixed ){
            break;
        }
    }

    //no robot info found
    if( iM==numMBody ) 
      return true; //no need to build
    
    //create MB for robot.
    m_RobotModel=new CMultiBodyModel(iM,pMInfo[iM]);
    if( m_RobotModel==NULL ) return false;
    m_RobotModel->setAsFree(); //set as free body
    SetColor(1,0,0,1);
    return m_RobotModel->BuildModels();
}

void OBPRMView_Robot::Draw(GLenum mode){
  if( m_RobotModel==NULL || GL_RENDER!=mode ) return;
  //static double rate = 360/(3.1415926*2);
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
  int numMBody=MBInfo[0].m_cNumberOfBody;
  CPolyhedronModel * pPoly = m_RobotModel->GetPolyhedron();

  for( int iM=0; iM<numMBody; iM++ ){
    MBInfo[0].m_pBodyInfo[iM].m_currentTransform.ResetTransformation();
    MBInfo[0].m_pBodyInfo[iM].m_prevTransform.ResetTransformation();
  }

  pPoly[0].tx()=cfg[0]; 
  pPoly[0].ty()=cfg[1]; 
  pPoly[0].tz()=cfg[2];

  Vector3d position;
  position.set(cfg[0], cfg[1], cfg[2]);
  Orientation orientation(Orientation::FixedXYZ, cfg[3], cfg[4], cfg[5]);
  Transformation b(orientation, position);
  MBInfo[0].m_pBodyInfo[0].m_currentTransform = b;

  //compute rotation
  Quaternion q1, q2;
  q2 = q1.getQuaternionFromMatrix(
  MBInfo[0].m_pBodyInfo[0].m_currentTransform.m_orientation.matrix);
  pPoly[0].q(q2.normalize()); //set new q
  
  //configuration of links after the base

  for( int iM=1; iM<numMBody; iM++ ){
    if( MBInfo[0].m_NumberOfConnections!=0 ){
      
      //Compute position and orientation for all of the links left

      int currentBody = 0; //index of current Body
      int nextBody = 0; //indext of next Body
      double theta = cfg[5+iM];

      currentBody = MBInfo[0].listConnections[iM-1].first;
      nextBody = MBInfo[0].listConnections[iM-1].second;
      Transformation t = MBInfo[0].m_pBodyInfo[currentBody].getTransform();    
      MBInfo[0].m_pBodyInfo[nextBody].setPrevTransform(t);
      MBInfo[0].m_pBodyInfo[nextBody].computeTransform(
                 MBInfo[0].m_pBodyInfo[currentBody], nextBody, theta);
      pPoly[iM].tx()=MBInfo[0].m_pBodyInfo[nextBody].m_currentTransform.m_position[0]; 
      pPoly[iM].ty()=MBInfo[0].m_pBodyInfo[nextBody].m_currentTransform.m_position[1]; 
      pPoly[iM].tz()=MBInfo[0].m_pBodyInfo[nextBody].m_currentTransform.m_position[2];  
      
      //compute rotation
      Quaternion q1, q2;
      q2 = q1.getQuaternionFromMatrix(
	   MBInfo[0].m_pBodyInfo[nextBody].m_currentTransform.m_orientation.matrix);
      pPoly[iM].q(q2.normalize()); //set new q
	
    }
  }

  m_Poly = m_RobotModel->GetPolyhedron();

}

void OBPRMView_Robot::DrawSelect()
{
  if( m_RobotModel==NULL ) return;
  glPushMatrix();
  glTransform();
  m_RobotModel->DrawSelect();
  glPopMatrix();
}

