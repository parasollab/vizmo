#include "Robot.h"
#include "vizmo2.h"


OBPRMView_Robot::OBPRMView_Robot(CEnvLoader * pEnv)
{
   m_pEnvLoader=pEnv;
   dof=1000;
   m_RobotModel=NULL;
   mode = 0;
   delta = .1;
   phantomdelta = .1;
   tempCfg = new double[dof];
   currentCfg = new double[dof]; 
   rotation_axis = new double[3];
   pPolyBackUp = NULL;
   mbRobotBackUp = NULL;
   RotFstBody = NULL;

   pthread_mutex_init(&mutex,NULL);

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

MultiBodyModel * OBPRMView_Robot::getRobotModel() const {
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
   m_RobotInfo = m_pEnvLoader->GetMultiBodyInfo();
   //     int iM=0;
   //     for( ; iM<numMBody; iM++ ){
   //       if( !pMInfo[iM].m_pBodyInfo[0].m_bIsFixed ){
   // 	break;
   //       }
   //     }

   //create MB for robot.
   for(int i = 0; i<numMBody; i++ ){
      if( pMInfo[i].m_active ){
         if(pMInfo[i].m_pBodyInfo[0].m_bIsFixed)
            m_RobotModel = new MultiBodyModel(pMInfo[0]);
         else
            m_RobotModel = new MultiBodyModel(pMInfo[i]);

         SetColor(pMInfo[i].m_pBodyInfo[0].rgb[0], 
               pMInfo[i].m_pBodyInfo[0].rgb[1], 
               pMInfo[i].m_pBodyInfo[0].rgb[2], 1);
         originalR = pMInfo[i].m_pBodyInfo[0].rgb[0];
         originalG = pMInfo[i].m_pBodyInfo[0].rgb[1];
         originalB = pMInfo[i].m_pBodyInfo[0].rgb[2];
         m_rR = originalR;
         m_rG = originalG; 
         m_rB = originalB; 
      }
   }

   if( m_RobotModel==NULL ){return false;}
   m_RobotModel->SetAsFree(); //set as free body
   //SetColor(1,0,0,1);
   return m_RobotModel->BuildModels();
}

void OBPRMView_Robot::Draw(GLenum mode){
   if( m_RobotModel==NULL){cout<<"m_RobotModel==NULL"<<endl; return;}
   glPushMatrix();
   glTransform();
   m_RobotModel->Draw(mode);
   glPopMatrix();
   //getFinalCfg();
}

void OBPRMView_Robot::Select( unsigned int * index, vector<gliObj>& sel ){
   //unselect old one       
   if( index==NULL ) return;
   m_RobotModel->Select(index+1,sel);
   //getFinalCfg();
}


void OBPRMView_Robot::DrawSelect()
{
   if( m_RobotModel==NULL ) return;
   glPushMatrix();
   glTransform();
   m_RobotModel->DrawSelect();
   glPopMatrix();
   //getFinalCfg();
}

void OBPRMView_Robot::InitialCfg(vector<double>& cfg){
   //save initial Cfg.
   dof = m_pEnvLoader->getDOF();
   //cout << "InitialCfg" << endl;
   StCfg = new double[dof];
   for(int i=0; i<dof; i++){
      StCfg[i] = cfg[i];
   }

   //save original size and color
   CPolyhedronModel& poly = m_RobotModel->GetPolyhedron()[0];
   originalR = poly.GetColor()[0];
   originalG = poly.GetColor()[1];
   originalB = poly.GetColor()[2];
   originalSize[0]=sx(); 
   originalSize[1]=sy();
   originalSize[2]=sz();
}

void OBPRMView_Robot::RestoreInitCfg(){
   //reset MultiBody values to 0's
   m_RobotModel->tx() = m_RobotModel->ty() = m_RobotModel->tz() = 0;
   double z = 0.0;
   double cx_2=cos(z); double sx_2=sin(z);
   double cy_2=cos(z); double sy_2=sin(z);
   double cz_2=cos(z); double sz_2=sin(z);

   Quaternion qx(cx_2,sx_2*Vector3d(1,0,0));
   Quaternion qy(cy_2,sy_2*Vector3d(0,1,0));
   Quaternion qz(cz_2,sz_2*Vector3d(0,0,1));
   Quaternion nq=qz*qy*qx; //new q

   m_RobotModel->q(nq.normalize());

   Configure(StCfg);

   //set robot to its current color and original size
   this->Scale(originalSize[0],originalSize[1],originalSize[2]);
   //this->SetColor(originalR,originalG,originalB,this->GetColor()[3]);  
   CPolyhedronModel& poly = m_RobotModel->GetPolyhedron()[0];
   this->SetColor(poly.GetColor()[0], poly.GetColor()[1],
       poly.GetColor()[2], poly.GetColor()[3]);
}

void OBPRMView_Robot::BackUp(){
  m_RenderModeBackUp = m_RenderMode;
   vector<CPolyhedronModel>& polys = m_RobotModel->GetPolyhedron();
   const CMultiBodyInfo * MBInfo = m_pEnvLoader->GetMultiBodyInfo();
   int numMBody=MBInfo[0].m_cNumberOfBody;
   if(pPolyBackUp == NULL)
      pPolyBackUp = new CPolyhedronModel[numMBody];
   for( int iM=0; iM<numMBody; iM++ ){
      pPolyBackUp[iM].tx()=polys[iM].tx();
      pPolyBackUp[iM].ty()=polys[iM].ty();
      pPolyBackUp[iM].tz()=polys[iM].tz();
      pPolyBackUp[iM].q(polys[iM].q());

   }

   R=polys[0].GetColor()[0];
   G=polys[0].GetColor()[1];
   B=polys[0].GetColor()[2];

   o_s[0]=sx(); 
   o_s[1]=sy();
   o_s[2]=sz();

   //save current cfg. of robot
   //and set m_RobotModel to zero

   if(mbRobotBackUp == NULL)
      mbRobotBackUp = new double[6];
   mbRobotBackUp[0]= m_RobotModel->tx();
   mbRobotBackUp[1]= m_RobotModel->ty();
   mbRobotBackUp[2]= m_RobotModel->tz();

   m_RobotModel->tx() =  m_RobotModel->ty() =  m_RobotModel->tz() = 0;

   MBq = m_RobotModel->q();
}

void OBPRMView_Robot::Restore(){
   this->Scale(o_s[0],o_s[1],o_s[2]);

   this->SetColor(R,G,B,this->GetColor()[3]);   

   //restore multibody

   m_RobotModel->tx() = mbRobotBackUp[0];
   m_RobotModel->ty() = mbRobotBackUp[1];
   m_RobotModel->tz() = mbRobotBackUp[2];

   m_RobotModel->q(MBq);

   //The nex lines are to keep the translation Tool ON the object
   m_RobotModel->m_PosPoly[0] = pPolyBackUp[0].tx();
   m_RobotModel->m_PosPoly[1] = pPolyBackUp[0].ty();
   m_RobotModel->m_PosPoly[2] = pPolyBackUp[0].tz();

  SetRenderMode(m_RenderModeBackUp);
  RestoreInitCfg();
}

void OBPRMView_Robot::keepColor(float r, float g, float b){
   m_rR = r; m_rG = g; m_rB = b;
   m_RobotInfo[0].m_pBodyInfo[0].rgb[0] = r;
   m_RobotInfo[0].m_pBodyInfo[0].rgb[1] = g;
   m_RobotInfo[0].m_pBodyInfo[0].rgb[2] = b;
}

void OBPRMView_Robot::Configure( double * cfg) { 

  pthread_mutex_lock(&mutex);

  const CMultiBodyInfo * MBInfo = m_pEnvLoader->GetMultiBodyInfo();
  int numMBody=m_pEnvLoader->GetNumberOfMultiBody();
  int robIndx = 0;
  vector<Robot>& robots = m_pEnvLoader->GetRobots();

  vector<CPolyhedronModel>& pPoly = m_RobotModel->GetPolyhedron();

  Vector3d position;
  Orientation orientation;

  static double TwoPI=3.1415926535*2.0;


  /////////////////////////////////////////////////////////////////////////////
  for( int iM=0; iM<numMBody; iM++ ){
    if( MBInfo[iM].m_active )
      robIndx = iM;
  }
  int myDOF=m_pEnvLoader->getDOF();

  int numBody=MBInfo[robIndx].m_cNumberOfBody;

  //to keep Cfgs. for bodies other than FirstLink
  if(RotFstBody == NULL)
    RotFstBody = new double[myDOF];
  for(int i = 0; i<myDOF; i++){
    RotFstBody[i] = cfg[i];
  }

  for( int i=0; i<numBody; i++ ){
    MBInfo[robIndx].m_pBodyInfo[i].m_currentTransform.ResetTransformation();
    MBInfo[robIndx].m_pBodyInfo[i].m_prevTransform.ResetTransformation();
    MBInfo[robIndx].m_pBodyInfo[i].m_transformDone = false;
  }

  int index=0; 
  typedef vector<Robot>::iterator RIT;
  for(RIT rit = robots.begin(); rit!=robots.end(); rit++){
    int posIndex=index;
    double x=0, y=0, z=0, alpha=0, beta=0, gamma=0;
    if(rit->m_base != Robot::FIXED){
      x = cfg[posIndex];
      y = cfg[posIndex+1];
      index+=2;
      if(rit->m_base==Robot::VOLUMETRIC){
        index++;
        z = cfg[posIndex+2];
      }
      if(rit->m_baseMovement == Robot::ROTATIONAL){
        if(rit->m_base==Robot::PLANAR){
          index++;
          gamma = cfg[posIndex+2] * TwoPI;
        }
        else{
          index+=3;
          alpha = cfg[posIndex+3] * TwoPI;
          beta = cfg[posIndex+4] * TwoPI;
          gamma = cfg[posIndex+5] * TwoPI;
        }
      }
    }

    size_t baseIndex = rit->m_bodyIndex;
    //step from PMPL
    pPoly[baseIndex].tx()=x; 
    pPoly[baseIndex].ty()=y; 
    pPoly[baseIndex].tz()=z;

    position.set(x, y, z);
    Orientation orientation_tmp(Orientation::FixedXYZ, alpha, beta, gamma);
    orientation = orientation_tmp;

    pPoly[baseIndex].rx() = alpha;
    pPoly[baseIndex].ry() = beta;
    pPoly[baseIndex].rz() = gamma;
    
    Transformation b(orientation, position);
    MBInfo[robIndx].m_pBodyInfo[baseIndex].m_currentTransform = b;

    //compute rotation
    Quaternion q1, q2;
    q2 = q1.getQuaternionFromMatrix(
        MBInfo[robIndx].m_pBodyInfo[baseIndex].m_currentTransform.m_orientation.matrix);
    pPoly[baseIndex].q(q2.normalize()); //set new q

    MBInfo[robIndx].m_pBodyInfo[baseIndex].m_transformDone = true;

    //now for the joints of the robot
    //this code from PMPL
    //configuration of links after the base
    //Compute position and orientation for all of the links left
    typedef Robot::JointMap::iterator MIT;
    for(MIT mit = rit->m_joints.begin(); mit!=rit->m_joints.end(); mit++){
      double theta = cfg[index] * TwoPI;
      index++;
      double alpha = 0;
      if(mit->second == Robot::SPHERICAL){
        alpha = cfg[index] * TwoPI;
        index++;
      }

      int currentBodyIdx = mit->first.first; //index of current Body
      CBodyInfo& currentBody = MBInfo[robIndx].m_pBodyInfo[currentBodyIdx];
      int nextBodyIdx = mit->first.second; //index of next Body
      CBodyInfo& nextBody = MBInfo[robIndx].m_pBodyInfo[nextBodyIdx];

      for( int i=0; i<currentBody.m_cNumberOfConnection; i++ ){
        CConnectionInfo& c = currentBody.m_pConnectionInfo[i];
        if(c.m_preIndex == currentBodyIdx && c.m_nextIndex == nextBodyIdx){
          c.alpha = alpha;
          c.theta = theta;
          break;
        }
      }

      if(!nextBody.m_transformDone){

        Transformation t = currentBody.getTransform(); 
        nextBody.setPrevTransform(t); 
        nextBody.computeTransform(currentBody, nextBodyIdx);

        pPoly[nextBodyIdx].tx()=nextBody.m_currentTransform.m_position[0]; 
        pPoly[nextBodyIdx].ty()=nextBody.m_currentTransform.m_position[1]; 
        pPoly[nextBodyIdx].tz()=nextBody.m_currentTransform.m_position[2];  

        //compute rotation
        Quaternion q1, q2;
        q2 = q1.getQuaternionFromMatrix(
            nextBody.m_currentTransform.m_orientation.matrix);
        pPoly[nextBodyIdx].q(q2.normalize()); //set new q

        nextBody.m_transformDone = true;
      }    
    }  
  }

  pthread_mutex_unlock(&mutex);

}

void OBPRMView_Robot::Configure(vector<double>& _cfg){
  double* cfg = new double[_cfg.size()];
  for(size_t i = 0; i<_cfg.size(); i++)
    cfg[i] = _cfg[i];
  Configure(cfg);
  delete [] cfg;
}

vector<double> OBPRMView_Robot::returnCurrCfg( int dof){
   vector<double> currentCfg; 
   for(int i=0;i<dof;i++){
     currentCfg.push_back(RotFstBody[i]);
   }
   return currentCfg;
}


void OBPRMView_Robot::SaveQry(vector<vector<double> >& cfg, char ch){

   int dof = m_pEnvLoader->getDOF();
   //to store actual cfg
   vector<double> currCfg(dof);
   //to store cfg. to be sent to Robot::setStart
   vector<vector<double> > vCfg;



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


vector<double> OBPRMView_Robot::getFinalCfg(){

  int dof = m_pEnvLoader->getDOF();
  //to store actual cfg
  vector<double> currCfg(dof);
  //to store cfg. to be sent to Robot::setStart
  vector<double *> vCfg;

  CGLModel * rl;

  double TwoPI=3.1415926535*2.0;    

  list<CGLModel*> robotList;
  m_RobotModel->GetChildren(robotList);

  //bool tool = false;

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
        currCfg[0] = m_RobotModel->tx();
        currCfg[1] = m_RobotModel->ty();
        currCfg[2] = m_RobotModel->tz();

        currCfg[3] = vRot[0]/TwoPI;
        currCfg[4] = vRot[1]/TwoPI;
        currCfg[5] = vRot[2]/TwoPI;
      }

    }

    m_RobotModel->SetCfg(currCfg);

    return currCfg;

  }

  else{//articulated

    currCfg = this->returnCurrCfg(dof);
    //add translation of MBody
    cout << "currCfg::" << currCfg[0] << endl;
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


    m_RobotModel->SetCfg(currCfg);

    return currCfg;


  }//else articulated

}


int OBPRMView_Robot::getNumJoints(){

   //const CMultiBodyInfo * pMInfo = m_pEnvLoader->GetMultiBodyInfo();
   //int numMBody=m_pEnvLoader->GetNumberOfMultiBody();
   m_RobotInfo = m_pEnvLoader->GetMultiBodyInfo();

   return m_RobotInfo[0].m_NumberOfConnections;

}




bool OBPRMView_Robot::KP( QKeyEvent * e )
{

#ifdef USE_PHANTOM
   Matrix3x3 mFinal; Vector3d vFinal;
   Quaternion qt0,qrm,qrmconj,finalQ,tmpq;
   hduVector3Dd pposition;
   hduVector3Dd protation;
   hduVector3Dd position;
   hduVector3Dd rotation;
   Vector3d axis; 
   Vector3d tmpaxis;
   double TwoPI = 2*3.14159;
   double angle;
   double R;
   double tx,ty,tz;
   double theta,phi,xi;
#endif

   switch(e->key()){
#ifdef USE_PHANTOM
      case Qt::Key_PageUp :

         pthread_mutex_lock(&mutex);

         //cout << "page up" << endl;

         pposition=GetPhantomManager().getEndEffectorPosition();
         protation=GetPhantomManager().getRotation();
         rotation=GetPhantomManager().getRotation();
         double prx,pry,prz;
         double drx,dry,drz;
         prx = m_RobotModel->rx();
         pry = m_RobotModel->ry();
         prz = m_RobotModel->rz();

         axis[0] = 0;
         axis[1] = 0;
         axis[2] = 1;


         tx = axis[0];
         ty = axis[1];
         tz = axis[2];


         GetCamera()->Transform(tx,ty,tz);


         axis[0] = tx;
         axis[1] = ty;
         axis[2] = tz;


         tmpaxis[0] = axis[0];
         tmpaxis[1] = axis[1];
         tmpaxis[2] = axis[2];
         phi = rotation[1];

         axis[1] = tmpaxis[1]*cos(phi) + tmpaxis[2]*sin(phi);
         axis[2] =-tmpaxis[1]*sin(phi) + tmpaxis[2]*cos(phi);


         tmpaxis[0] = axis[0];
         tmpaxis[1] = axis[1];
         tmpaxis[2] = axis[2];
         theta = rotation[0];

         axis[0] = tmpaxis[0]*cos(theta) + tmpaxis[2]*sin(theta);
         axis[2] =-tmpaxis[0]*sin(theta) + tmpaxis[2]*cos(theta);


         tmpaxis[0] = axis[0];
         tmpaxis[1] = axis[1];
         tmpaxis[2] = axis[2];
         xi = prz;

         axis[0] = tmpaxis[0]*cos(xi) + tmpaxis[1]*sin(xi);
         axis[1] =-tmpaxis[0]*sin(xi) + tmpaxis[1]*cos(xi);


         tmpaxis[0] = axis[0];
         tmpaxis[1] = axis[1];
         tmpaxis[2] = axis[2];
         theta = -pry;

         axis[0] = tmpaxis[0]*cos(theta) + tmpaxis[2]*sin(theta);
         axis[2] =-tmpaxis[0]*sin(theta) + tmpaxis[2]*cos(theta);


         tmpaxis[0] = axis[0];
         tmpaxis[1] = axis[1];
         tmpaxis[2] = axis[2];
         phi = prx;

         axis[1] = tmpaxis[1]*cos(phi) + tmpaxis[2]*sin(phi);
         axis[2] =-tmpaxis[1]*sin(phi) + tmpaxis[2]*cos(phi);


         rotation_axis[0] = axis[0];
         rotation_axis[1] = axis[1];
         rotation_axis[2] = axis[2];


         angle = -100*(rotation[2] - protation[2]);

         for(unsigned int i=0;i<dof;i++) 
            tempCfg[i] = StCfg[i]; 

         //Need to compute rotation from Quaternion

         //get rotation quaternion 
         qt0 = Quaternion(cos(angle),sin(angle)*axis);

         //get new rotation from multiBody 
         qrm = m_RobotModel->q();

         //multiply polyhedron0 and multiBody quaternions //to get new rotation
         finalQ = qrm * qt0;

         //set new rotation angles to multiBody rx(), ry(), and rz()
         mFinal = finalQ.getMatrix(); 
         vFinal = finalQ.MatrixToEuler(mFinal);

         m_RobotModel->rx() = vFinal[0]; 
         m_RobotModel->ry() = vFinal[1];
         m_RobotModel->rz() = vFinal[2];


         //set new angles for first polyhedron //NOTE:: This works for
         //**FREE** robots

         tempCfg[3] =  vFinal[0]/TwoPI; 
         tempCfg[4] =  vFinal[1]/TwoPI; 
         tempCfg[5] =  vFinal[2]/TwoPI;


         m_RobotModel->setCurrCfg(tempCfg, dof);

         m_RobotModel->Euiler2Quaternion();
         GetVizmo().TurnOn_CD();
         if(GetPhantomManager().Collision > .5 && !GetPhantomManager().UseFeedback){
            m_RobotModel->setCurrCfg(currentCfg, dof);
            return true;
         }
         m_RobotModel->Euiler2Quaternion();
         for(int i=0; i<dof; i++){
            StCfg[i] = tempCfg[i];
            //cout << StCfg[i] << " ";
         }
         //cout << endl;

         pthread_mutex_unlock(&mutex);

         return true;
      case Qt::Key_PageDown :

         pthread_mutex_lock(&mutex);

         //cout << "page down" << endl;

         for(unsigned int i=0;i<dof;i++){
            tempCfg[i] = StCfg[i]; 
            //cout << StCfg[i] << " ";
         }
         //cout << endl;

         pposition=GetPhantomManager().getEndEffectorPosition();
         protation=GetPhantomManager().getRotation();
         position=GetPhantomManager().getEndEffectorPosition();
         double dx,dy,dz;
         dx=phantomdelta*(position[0]-pposition[0]);
         dy=phantomdelta*(position[1]-pposition[1]);
         dz=phantomdelta*(position[2]-pposition[2]);
         GetCamera()->Transform(dx,dy,dz);
         if(fabs(dx) < 10 && fabs(dy) < 10 && fabs(dz) < 10){
            m_RobotModel->tx() += dx;
            m_RobotModel->ty() += dy;
            m_RobotModel->tz() += dz;
            tempCfg[0] += dx;
            tempCfg[1] += dy;
            tempCfg[2] += dz;
         }
         GetVizmo().TurnOn_CD();
         if(GetPhantomManager().Collision > .5 && !GetPhantomManager().UseFeedback){
            //m_RobotModel->tx() -= dx;
            //m_RobotModel->ty() -= dy;
            //m_RobotModel->tz() -= dz;
            tempCfg[0] -= dx;
            tempCfg[1] -= dy;
            tempCfg[2] -= dz;
            return true;
         }
         //m_RobotModel->glTransform();
         for(int i=0; i<dof; i++){
            StCfg[i] = tempCfg[i];
            //cout << StCfg[i] << " ";
         }
         //cout << endl;
         pthread_mutex_unlock(&mutex);

         return true;
#endif
         //case 'Z': case 'z':
         //	  print();
         //        return true;
         //case '0':
         //	  mode = 0;
         //        return true;
         //case '1':
         //        mode = 1;
         //        return true;
         //case '2':
         //        mode = 2;
         //     	  return true;
         //case '3':
         // 	  mode = 3;
         //        return true;
         //case '4':
         //        mode = 4;
         // 	  return true;
#ifdef USE_PHANTOM
      case Qt::Key_Insert : 
         phantomdelta *= 1.1;
         //cout << phantomdelta << endl;
         return true;
      case Qt::Key_Delete :
         phantomdelta /= 1.1;
         //cout << phantomdelta << endl;
         return true;
      case Qt::Key_Home:
         if(GetPhantomManager().phantomforce < 5)
            GetPhantomManager().phantomforce *= 1.1;
         //cout << GetPhantomManager().phantomforce << endl;
         return true;
      case Qt::Key_End:
         if(GetPhantomManager().phantomforce > .05)
            GetPhantomManager().phantomforce /= 1.1;
         //cout << GetPhantomManager().phantomforce << endl;
         return true;
      case '.':
         GetPhantomManager().UseFeedback = !GetPhantomManager().UseFeedback;
         return true;
#endif
         //case '5':
         //        mode = 5;
         //	return true;
         //case 'A': case 'a':
         //        Transform(1);
         //	return true;
         //case 'S': case 's':
         //        Transform(-1);
         //	return true;



   }



   return false;
}

void OBPRMView_Robot::Transform(int dir){
   switch(mode){
      case 0:
         m_RobotModel->tx()+=dir*delta;
         break;
      case 1:
         m_RobotModel->ty()+=dir*delta;
         break;
      case 2:
         m_RobotModel->tz()+=dir*delta;
         break;
      case 3:
         m_RobotModel->rx()+=dir*delta;
         break;
      case 4:
         m_RobotModel->ry()+=dir*delta;
         break;
      case 5:
         m_RobotModel->rz()+=dir*delta;
         break;
   }
   m_RobotModel->Euiler2Quaternion();
   m_RobotModel->glTransform();
}







