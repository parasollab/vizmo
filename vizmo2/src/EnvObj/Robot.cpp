#include "Robot.h"


OBPRMView_Robot::OBPRMView_Robot(CEnvLoader * pEnv)
{
    m_pEnvLoader=pEnv;
    m_RobotModel=NULL;
}

OBPRMView_Robot::OBPRMView_Robot(const OBPRMView_Robot &other_robot)
:CGLModel(other_robot)
{
    m_pEnvLoader=other_robot.getEnvLoader();
    m_RobotModel = other_robot.getRobotModel();
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

//     int iM=0;
//     for( ; iM<numMBody; iM++ ){
//       if( !pMInfo[iM].m_pBodyInfo[0].m_bIsFixed ){
// 	break;
//       }
//     }
    
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
  if( m_RobotModel==NULL) return;
  glPushMatrix();
  glTransform();
  m_RobotModel->Draw(mode);
  glPopMatrix();

}

void OBPRMView_Robot::Select( unsigned int * index, vector<gliObj>& sel ){
  //unselect old one       
  if( index==NULL ) return;
  m_RobotModel->Select(index+1,sel);
}


void OBPRMView_Robot::DrawSelect()
{
  if( m_RobotModel==NULL ) return;
  glPushMatrix();
  glTransform();
  m_RobotModel->DrawSelect();
  glPopMatrix();

}

void OBPRMView_Robot::InitialCfg(double * cfg){
  //save initial Cfg.
  dof = m_pEnvLoader->getDOF();
  StCfg = new double[dof];
  for(int i=0; i<dof; i++)
    StCfg[i] = cfg[i];

  //save original size and color
  Poly = m_RobotModel->GetPolyhedron();
  originalR=Poly[0].GetColor()[0];
  originalG=Poly[0].GetColor()[1];
  originalB=Poly[0].GetColor()[2];
  originalSize[0]=sx(); originalSize[1]=sy();originalSize[2]=sz();
   
}

void OBPRMView_Robot::RestoreInitCfg(){

//reset MultiBody values to 0's
  m_RobotModel->tx() = 0;
  m_RobotModel->ty() = 0;
  m_RobotModel->tz() = 0;
  Vector3d v;
  v.set(0,0,0);	
  Quaternion tmpQ(1, v);
  m_RobotModel->q(tmpQ);

  //set to start Cfg.
  Configure(StCfg);

  //set robot to its original color and size
  this->Scale(originalSize[0],originalSize[1],originalSize[2]);
  this->SetColor(originalR,originalG,originalB,this->GetColor()[3]);   
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

   //save current cfg. of robot
   //and set m_RobotModel to zero

   //mbRobotBackUp = new CMultiBodyModel[numMBody];
   mbRobotBackUp = new double[6];
   mbRobotBackUp[0]= m_RobotModel->tx();
   mbRobotBackUp[1]= m_RobotModel->ty();
   mbRobotBackUp[2]= m_RobotModel->tz();

   m_RobotModel->tx() =  m_RobotModel->ty() =  m_RobotModel->tz() = 0;
   
   MBq = m_RobotModel->q();
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

 //restore multibody

 m_RobotModel->tx() = mbRobotBackUp[0];
 m_RobotModel->ty() = mbRobotBackUp[1];
 m_RobotModel->tz() = mbRobotBackUp[2];

 m_RobotModel->q(MBq);

 //The nex lines are to keep the translation Tool ON the object
 m_RobotModel->m_PosPoly[0] = Poly[0].tx();
 m_RobotModel->m_PosPoly[1] = Poly[0].ty();
 m_RobotModel->m_PosPoly[2] = Poly[0].tz();

  //delete [] Poly;
  //delete [] pPolyBackUp;

}


void OBPRMView_Robot::Configure( double * cfg) { 

  const CMultiBodyInfo * MBInfo = m_pEnvLoader->GetMultiBodyInfo();
  int numMBody=m_pEnvLoader->GetNumberOfMultiBody();
  int robIndx = 0;
  
  CPolyhedronModel * pPoly = m_RobotModel->GetPolyhedron();

  Vector3d position;
  Orientation orientation;

  static double TwoPI=3.1415926535*2.0;


  /////////////////////////////////////////////////////////////////////////////
  for( int iM=0; iM<numMBody; iM++ ){
    if( MBInfo[iM].m_active )
      robIndx = iM;
  }
  bool fixedBase=MBInfo[robIndx].m_pBodyInfo[0].m_bIsFixed;
  int myDOF=m_pEnvLoader->getDOF();

  /////////////////////////////////////////////////////////////////////////////
  // convert cfg
  for( int id=0;id<myDOF;id++ )
	if( fixedBase || id>=3 ) cfg[id]*=TwoPI;
  /////////////////////////////////////////////////////////////////////////////

  int numBody=MBInfo[robIndx].m_cNumberOfBody;

  //to keep Cfgs. for bodies other than FirstLink
  RotFstBody = new double[ myDOF - 3];

  {for( int iM=0; iM<numBody; iM++ ){
    MBInfo[robIndx].m_pBodyInfo[iM].m_currentTransform.ResetTransformation();
    MBInfo[robIndx].m_pBodyInfo[iM].m_prevTransform.ResetTransformation();
    MBInfo[robIndx].m_pBodyInfo[iM].m_transformDone = false;
  }}

  if(fixedBase){

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

    m_RobotModel->m_PosPoly[0] = pPoly[0].tx();
    m_RobotModel->m_PosPoly[1] = pPoly[0].ty();
    m_RobotModel->m_PosPoly[2] = pPoly[0].tz();

    m_RobotModel->m_RotPoly[0] = pPoly[0].rx();
    m_RobotModel->m_RotPoly[1] = pPoly[0].ry();
    m_RobotModel->m_RotPoly[2] = pPoly[0].rz();
  
    RotFstBody[0] = MBInfo[robIndx].m_pBodyInfo[0].m_X;
    RotFstBody[1] = MBInfo[robIndx].m_pBodyInfo[0].m_Y;
    RotFstBody[2] = MBInfo[robIndx].m_pBodyInfo[0].m_Z;
  }

  else{

    pPoly[0].tx()=cfg[0]; 
    pPoly[0].ty()=cfg[1]; 
    pPoly[0].tz()=cfg[2];
    
    position.set(cfg[0], cfg[1], cfg[2]);
    Orientation orientation_tmp(Orientation::FixedXYZ, cfg[3], cfg[4], cfg[5]);
    orientation = orientation_tmp;

    m_RobotModel->m_PosPoly[0] = pPoly[0].tx();
    m_RobotModel->m_PosPoly[1] = pPoly[0].ty();
    m_RobotModel->m_PosPoly[2] = pPoly[0].tz();

    //this variable is used when THIS is an art. object
    //they are accessed in returnCurrCfg( int dof) func.
    //however, they may not be necessary, we can use 
    // pPoly[0].rx(), etc. for this case...

    RotFstBody[0] = cfg[3];
    RotFstBody[1] = cfg[4];
    RotFstBody[2] = cfg[5];

    pPoly[0].rx() = cfg[3];
    pPoly[0].ry() = cfg[4];
    pPoly[0].rz() = cfg[5];

    m_RobotModel->m_RotPoly[0] = pPoly[0].rx();
    m_RobotModel->m_RotPoly[1] = pPoly[0].ry();
    m_RobotModel->m_RotPoly[2] = pPoly[0].rz();
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
      
      if(MBInfo[robIndx].m_pBodyInfo[0].m_bIsFixed){
	theta = cfg[iM];
	//save the rest of the Cfg. to pass it on to the querySaving func.
	//RotFstBody[iM+6] = cfg[iM];
      }
      else{
	theta = cfg[6+iM]; //base already computed from cfg0,...,cfg5

	//save the rest of the Cfg. to pass it on to the querySaving func.
	RotFstBody[iM+3] = cfg[6+iM];
      }
      
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

double *OBPRMView_Robot::returnCurrCfg( int dof){
  double * currentCfg=new double[dof]; 

  double TwoPI=3.1415926535*2.0;
  for(int i=0;i<dof;i++){
    if(i==0)
      currentCfg[i] = m_RobotModel->m_PosPoly[0];
    else if(i==1)
      currentCfg[i] = m_RobotModel->m_PosPoly[1];
    else if(i==2)
      currentCfg[i] = m_RobotModel->m_PosPoly[2];
    else
      currentCfg[i] = RotFstBody[i-3]/TwoPI;
  }
  return currentCfg;
}


void OBPRMView_Robot::SaveQry(vector<double *> cfg, char ch){

  int dof = m_pEnvLoader->getDOF();
  //to store actual cfg
  double *currCfg = new double[dof];
  //to store cfg. to be sent to Robot::setStart
  vector<double *> vCfg;

  CGLModel * rl;

  double TwoPI=3.1415926535*2.0;    
    
  list<CGLModel*> robotList;
  m_RobotModel->GetChildren(robotList);

  if(dof == 6){ //rigid body
      
    //if user didn't move robot by hand, this means
    //the animation tool was used
    if(m_RobotModel->tx() == 0 && m_RobotModel->ty() == 0 && m_RobotModel->tz() == 0){
      typedef list<CGLModel *>::iterator RI;
      
      for(RI i=robotList.begin(); i!=robotList.end(); i++){
	rl = (CGLModel*)(*i);
	
	currCfg[0] = rl->tx();
	currCfg[1] = rl->ty();
	currCfg[2] = rl->tz();


	//Need to compute rotation from Quaternion
	
	CGLModel* rm = robotList.front();
	//get current quaternion from polyhedron0
	Quaternion qt0;
	qt0 = rm->q();
	Matrix3x3 pm = qt0.getMatrix();
	Vector3d pv = qt0.MatrixToEuler(pm);
	
	//get new rotation from GL
	Quaternion qrm;
	qrm = m_RobotModel->q();

	//multiply polyhedron0 and multiBody quaternions
	//to get new rotation
	Quaternion finalQ;
	finalQ = qrm * qt0;

	//set new rotation angles to multiBody rx(), ry(), and rz()

	Matrix3x3 mFinal; Vector3d vFinal;
	mFinal = finalQ.getMatrix();
	vFinal = finalQ.MatrixToEuler(mFinal);

	m_RobotModel->rx() = vFinal[0];
	m_RobotModel->ry() = vFinal[1];
	m_RobotModel->rz() = vFinal[2];
	
	//set new angles for first polyhedron
	//NOTE:: This works for **FREE** robots

	currCfg[3] =  m_RobotModel->rx()/TwoPI;
	currCfg[4] =  m_RobotModel->ry()/TwoPI;
	currCfg[5] =  m_RobotModel->rz()/TwoPI;


	//currCfg[3] = rl->rx();
	//currCfg[4] = rl->ry();
	//currCfg[5] = rl->rz();
      }
    }
    else{ //user moved robot by hand 
      
      Matrix3x3 m = m_RobotModel->getMatrix();
      Vector3d vRot;
      vRot = m_RobotModel->MatrixToEuler(m);
      
      if(m_RobotModel->m_PosPoly[0] != -1){
	//robot has been moved
	currCfg[0] = m_RobotModel->tx() + m_RobotModel->m_PosPoly[0];
	currCfg[1] = m_RobotModel->ty() + m_RobotModel->m_PosPoly[1];
	currCfg[2] = m_RobotModel->tz() + m_RobotModel->m_PosPoly[2];
	
	//Need to compute rotation from Quaternion
	
	CGLModel* rm = robotList.front();
	//get current quaternion from polyhedron0
	Quaternion qt0;
	qt0 = rm->q();
	Matrix3x3 pm = qt0.getMatrix();
	Vector3d pv = qt0.MatrixToEuler(pm);
	
	//get new rotation from GL
	Quaternion qrm;
	qrm = m_RobotModel->q();

	//multiply polyhedron0 and multiBody quaternions
	//to get new rotation
	Quaternion finalQ;
	finalQ = qrm * qt0;

	//set new rotation angles to multiBody rx(), ry(), and rz()

	Matrix3x3 mFinal; Vector3d vFinal;
	mFinal = finalQ.getMatrix();
	vFinal = finalQ.MatrixToEuler(mFinal);

	m_RobotModel->rx() = vFinal[0];
	m_RobotModel->ry() = vFinal[1];
	m_RobotModel->rz() = vFinal[2];
	
	//set new angles for first polyhedron
	//NOTE:: This works for **FREE** robots

	currCfg[3] =  m_RobotModel->rx()/TwoPI;
	currCfg[4] =  m_RobotModel->ry()/TwoPI;
	currCfg[5] =  m_RobotModel->rz()/TwoPI;
 
      }
      else{
	//robot has not been moved before
	currCfg[0] = m_RobotModel->tx() + cfg[0][0];
	currCfg[1] = m_RobotModel->ty() + cfg[0][1];
	currCfg[2] = m_RobotModel->tz() + cfg[0][2];
	currCfg[3] = vRot[0]/TwoPI;
	currCfg[4] = vRot[1]/TwoPI;
	currCfg[5] = vRot[2]/TwoPI;
      }

    }
      
    vCfg.push_back(currCfg);
    //this->storeCfg(vCfg, 's', dof);
    this->storeCfg(vCfg, ch, dof);

  }
  
  else{//articulated
      
    currCfg = this->returnCurrCfg(dof);
    //add translation of MBody
    currCfg[0] = currCfg[0] + m_RobotModel->tx();
    currCfg[1] = currCfg[1] + m_RobotModel->ty();
    currCfg[2] = currCfg[2] + m_RobotModel->tz();
    
    //Need to compute rotation from Quaternion
    
    CGLModel* rm = robotList.front();
    //get current quaternion from polyhedron0
    Quaternion qt0;
    qt0 = rm->q();
    Matrix3x3 pm = qt0.getMatrix();
    Vector3d pv = qt0.MatrixToEuler(pm);
    
    //get new rotation from multiBody 
    Quaternion qrm;
    qrm = m_RobotModel->q();
    
    //multiply polyhedron0 and multiBody quaternions
    //to get new rotation
    Quaternion finalQ;
    finalQ = qrm * qt0;
    
    //set new rotation angles to multiBody rx(), ry(), and rz()
    
    Matrix3x3 mFinal; Vector3d vFinal;
    mFinal = finalQ.getMatrix();
    vFinal = finalQ.MatrixToEuler(mFinal);
    
    m_RobotModel->rx() = vFinal[0];
    m_RobotModel->ry() = vFinal[1];
    m_RobotModel->rz() = vFinal[2];
    
    //set new angles for first polyhedron
    //NOTE:: This works for **FREE** robots
    
    currCfg[3] =  m_RobotModel->rx()/TwoPI;
    currCfg[4] =  m_RobotModel->ry()/TwoPI;
    currCfg[5] =  m_RobotModel->rz()/TwoPI;	

    vCfg.push_back(currCfg);
    this->storeCfg(vCfg, ch, dof);

  }//else articulated

}
