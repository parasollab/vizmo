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
  // do not let select the robot:
  // if( m_RobotModel==NULL || GL_RENDER!=mode ) return;

  if( m_RobotModel==NULL) return;
  glPushMatrix();
  glTransform();
  // m_RobotModel->Draw(GL_RENDER);
  m_RobotModel->Draw(mode);
  glPopMatrix();

}

void OBPRMView_Robot::Select( unsigned int * index, vector<gliObj>& sel ){
  //unselect old one       
  if( index==NULL ) return;
  
  m_RobotModel->Select(index+1,sel);

  //call InitialCfg
  //BackUp();
  
  //sel.push_back(this);
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

    //update rotation vars.
    //set rx(), ry(), and rz() of RobotPolyhedron
    //with cfgs. of robot's base

    pPoly[0].rx() = cfg[3]/TwoPI;
    pPoly[0].ry() = cfg[4]/TwoPI;
    pPoly[0].rz() = cfg[5]/TwoPI;

    m_RobotModel->m_RotPoly[0] = pPoly[0].rx();
    m_RobotModel->m_RotPoly[1] = pPoly[0].ry();
    m_RobotModel->m_RotPoly[2] = pPoly[0].rz();

//     printf("Position of Robot:: %1f, %1f, %1f \n", 
// 	   m_RobotModel->m_PosPoly[0],m_RobotModel->m_PosPoly[1],m_RobotModel->m_PosPoly[2]);
//     printf("Orientation of Robot:: %1f, %1f, %1f \n", 
// 	   RotFstBody[0],RotFstBody[1],RotFstBody[2]);

  }

  Transformation b(orientation, position);
  MBInfo[robIndx].m_pBodyInfo[0].m_currentTransform = b;
  
  //compute rotation
  Quaternion q1, q2;
  q2 = q1.getQuaternionFromMatrix(
	  MBInfo[robIndx].m_pBodyInfo[0].m_currentTransform.m_orientation.matrix);
  pPoly[0].q(q2.normalize()); //set new q


 //  //testing fucntion to convert Quat.to Euler
   Matrix3x3 m;
   m =  pPoly[0].getMatrix();
   Vector3d v;
   v = pPoly[0].MatrixToEuler(m);
 
  //  cout<<"######################################################"<<endl;
  
  //   printf("Original X, Y, Z:: %2f, %2f, %2f \n",cfg[3]/TwoPI, cfg[4]/TwoPI, cfg[5]/TwoPI);
  //   printf("To radians X, Y, Z:: %2f, %2f, %2f \n",cfg[3], cfg[4], cfg[5]);
  //   printf("MatToEuler X, Y, Z:: %2f, %2f, %2f \n", v[0], v[1], v[2]);
  
  //   cout<<"######################################################"<<endl;
  
  //set rx(), ry(), and rz() of RobotMBModel
  //for robot's base
//   m_RobotModel->rx() = v[0]/TwoPI;
//   m_RobotModel->ry() = v[1]/TwoPI;
//   m_RobotModel->rz() = v[2]/TwoPI;

    m_RobotModel->m_RotPoly[0] = v[0]/TwoPI;
    m_RobotModel->m_RotPoly[1] = v[1]/TwoPI;
    m_RobotModel->m_RotPoly[2] = v[2]/TwoPI;


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

	//update rotation vars.
	//pPoly[nextBody].Quaternion2Euler();
	
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
