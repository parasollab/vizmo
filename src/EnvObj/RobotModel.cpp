#include "RobotModel.h"
#include "Models/Vizmo.h"


RobotModel::RobotModel(EnvModel* _env){

  m_envModel = _env;
  dof = 1000;
  m_RobotModel = NULL;
  mode = 0;
  delta = .1;
  phantomdelta = .1;
  tempCfg = new double[dof];
  currentCfg = new double[dof];
  rotation_axis = new double[3];
  mbRobotBackUp = NULL;
  RotFstBody = NULL;
  StCfg = NULL;
  pthread_mutex_init(&mutex, NULL);
}

RobotModel::RobotModel(const RobotModel& _otherRobot)
  :GLModel(_otherRobot){

   m_envModel = _otherRobot.GetEnvModel();
   m_RobotModel = _otherRobot.getRobotModel();
}

MultiBodyModel* RobotModel::getRobotModel() const {
   return m_RobotModel;
}

RobotModel::~RobotModel()
{
   delete m_RobotModel;
}

bool RobotModel::BuildModels(){

   //find robot name
   const CMultiBodyInfo* pMInfo = m_envModel->GetMultiBodyInfo();
   int numMBody=m_envModel->GetNumMultiBodies();
   m_RobotInfo = m_envModel->GetMultiBodyInfo();
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
            m_RobotModel = new MultiBodyModel(pMInfo[i]);
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

void RobotModel::Draw(GLenum _mode){
   if( m_RobotModel==NULL){cout<<"m_RobotModel==NULL"<<endl; return;}
   glPushMatrix();
   glTransform();
   m_RobotModel->Draw(mode);
   glPopMatrix();
   //getFinalCfg();
}

void RobotModel::Select( unsigned int * index, vector<gliObj>& sel ){
   //unselect old one
   if( index==NULL ) return;
   m_RobotModel->Select(index+1,sel);
   //getFinalCfg();
}


void RobotModel::DrawSelect()
{
   if( m_RobotModel==NULL ) return;
   glPushMatrix();
   glTransform();
   m_RobotModel->DrawSelect();
   glPopMatrix();
   //getFinalCfg();
}

void RobotModel::InitialCfg(vector<double>& cfg){
   //save initial Cfg.
   dof = m_envModel->GetDOF();
   StCfg = new double[dof];
   for(int i=0; i<dof; i++){
      StCfg[i] = cfg[i];
   }

   //save original size and color
   PolyhedronModel& poly = m_RobotModel->GetPolyhedron()[0];
   originalR = poly.GetColor()[0];
   originalG = poly.GetColor()[1];
   originalB = poly.GetColor()[2];
   originalSize[0]=sx();
   originalSize[1]=sy();
   originalSize[2]=sz();
}

void RobotModel::RestoreInitCfg(){
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
   PolyhedronModel& poly = m_RobotModel->GetPolyhedron()[0];
   this->SetColor(poly.GetColor()[0], poly.GetColor()[1],
       poly.GetColor()[2], poly.GetColor()[3]);
}

void RobotModel::BackUp(){
  m_RenderModeBackUp = m_renderMode;
   vector<PolyhedronModel>& polys = m_RobotModel->GetPolyhedron();

   m_polyXBack = polys[0].tx();
   m_polyYBack = polys[0].ty();
   m_polyZBack = polys[0].tz();

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

void RobotModel::Restore(){
   this->Scale(o_s[0],o_s[1],o_s[2]);

   this->SetColor(R,G,B,this->GetColor()[3]);

   //restore multibody

   m_RobotModel->tx() = mbRobotBackUp[0];
   m_RobotModel->ty() = mbRobotBackUp[1];
   m_RobotModel->tz() = mbRobotBackUp[2];

   m_RobotModel->q(MBq);

   //The nex lines are to keep the translation Tool ON the object
   m_RobotModel->m_PosPoly[0] = m_polyXBack;
   m_RobotModel->m_PosPoly[1] = m_polyYBack;
   m_RobotModel->m_PosPoly[2] = m_polyZBack;

  SetRenderMode(m_RenderModeBackUp);
  RestoreInitCfg();
}

void RobotModel::keepColor(float r, float g, float b){
   m_rR = r; m_rG = g; m_rB = b;
   m_RobotInfo[0].m_pBodyInfo[0].rgb[0] = r;
   m_RobotInfo[0].m_pBodyInfo[0].rgb[1] = g;
   m_RobotInfo[0].m_pBodyInfo[0].rgb[2] = b;
}

void RobotModel::Configure( double * cfg) {
  pthread_mutex_lock(&mutex);

  const CMultiBodyInfo* MBInfo = m_envModel->GetMultiBodyInfo();
  int numMBody = m_envModel->GetNumMultiBodies();
  int robIndx = 0;
  vector<Robot>& robots = m_envModel->GetRobots();

  vector<PolyhedronModel>& pPoly = m_RobotModel->GetPolyhedron();

  Vector3d position;
  Orientation orientation;

  /////////////////////////////////////////////////////////////////////////////
  for( int iM=0; iM<numMBody; iM++ ){
    if( MBInfo[iM].m_active )
      robIndx = iM;
  }
  int myDOF = m_envModel->GetDOF();

  int numBody=MBInfo[robIndx].m_cNumberOfBody;

  //to keep Cfgs. for bodies other than FirstLink
  if(RotFstBody == NULL)
    RotFstBody = new double[myDOF];
  for(int i = 0; i<myDOF; i++){
    RotFstBody[i] = cfg[i];
  }

  for( int i=0; i<numBody; i++ ){
    MBInfo[robIndx].m_pBodyInfo[i].m_currentTransform = Transformation();
    MBInfo[robIndx].m_pBodyInfo[i].m_prevTransform = Transformation();
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
          gamma = cfg[posIndex+2] * PI;
        }
        else{
          index+=3;
          alpha = cfg[posIndex+3] * PI;
          beta = cfg[posIndex+4] * PI;
          gamma = cfg[posIndex+5] * PI;
        }
      }
    }

    size_t baseIndex = rit->m_bodyIndex;
    //step from PMPL
    pPoly[baseIndex].tx()=x;
    pPoly[baseIndex].ty()=y;
    pPoly[baseIndex].tz()=z;

    pPoly[baseIndex].rx() = alpha;
    pPoly[baseIndex].ry() = beta;
    pPoly[baseIndex].rz() = gamma;

    MBInfo[robIndx].m_pBodyInfo[baseIndex].m_currentTransform =
      Transformation(Vector3d(x, y, z), Orientation(EulerAngle(gamma, beta, alpha)));

    //compute rotation
    Quaternion q;
    convertFromMatrix(q,
        MBInfo[robIndx].m_pBodyInfo[baseIndex].m_currentTransform.rotation().matrix());
    pPoly[baseIndex].q(q.normalized()); //set new q

    MBInfo[robIndx].m_pBodyInfo[baseIndex].m_transformDone = true;

    //now for the joints of the robot
    //this code from PMPL
    //configuration of links after the base
    //Compute position and orientation for all of the links left
    typedef Robot::JointMap::const_iterator MIT;
    for(MIT mit = rit->GetJointMap().begin(); mit!=rit->GetJointMap().end(); mit++){
      double theta = cfg[index] * PI;
      index++;
      double alpha = 0;
      if((*mit)->GetJointType() == CConnectionInfo::SPHERICAL){
        alpha = cfg[index] * PI;
        index++;
      }

      int currentBodyIdx = (*mit)->GetPreviousBody(); //index of current Body
      CBodyInfo& currentBody = MBInfo[robIndx].m_pBodyInfo[currentBodyIdx];
      int nextBodyIdx = (*mit)->GetNextBody(); //index of next Body
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

        pPoly[nextBodyIdx].tx()=nextBody.m_currentTransform.translation()[0];
        pPoly[nextBodyIdx].ty()=nextBody.m_currentTransform.translation()[1];
        pPoly[nextBodyIdx].tz()=nextBody.m_currentTransform.translation()[2];

        //compute rotation
        Quaternion q;
        convertFromMatrix(q,
            nextBody.m_currentTransform.rotation().matrix());
        pPoly[nextBodyIdx].q(q.normalized()); //set new q

        nextBody.m_transformDone = true;
      }
    }
  }

  pthread_mutex_unlock(&mutex);

}

void RobotModel::Configure(vector<double>& _cfg){
  double* cfg = new double[_cfg.size()];
  for(size_t i = 0; i<_cfg.size(); i++)
    cfg[i] = _cfg[i];
  Configure(cfg);
  delete [] cfg;
}

vector<double> RobotModel::returnCurrCfg( int dof){
   vector<double> currentCfg;
   for(int i=0;i<dof;i++){
     currentCfg.push_back(RotFstBody[i]);
   }
   return currentCfg;
}


void RobotModel::SaveQry(vector<vector<double> >& cfg, char ch){

   int dof = m_envModel->GetDOF();
   //to store actual cfg
   vector<double> currCfg(dof);
   //to store cfg. to be sent to Robot::setStart
   vector<vector<double> > vCfg;



   GLModel * rl;

   list<GLModel*> robotList;
   m_RobotModel->GetChildren(robotList);

   if(dof == 6){ //rigid body

      //if user didn't move robot by hand, this means
      //the animation tool was used
      if(m_RobotModel->tx() == 0 && m_RobotModel->ty() == 0 && m_RobotModel->tz() == 0){
         typedef list<GLModel *>::iterator RI;

         for(RI i=robotList.begin(); i!=robotList.end(); i++){
            rl = (GLModel*)(*i);
            currCfg[0] = rl->tx();
            currCfg[1] = rl->ty();
            currCfg[2] = rl->tz();
            //Need to compute rotation from Quaternion

            GLModel* rm = robotList.front();

            //get new rotation from GL
            Quaternion qrm;
            qrm = m_RobotModel->q();

            //multiply polyhedron0 and multiBody quaternions
            //to get new rotation
            Quaternion finalQ;
            finalQ = qrm * rm->q();

            //set new rotation angles to multiBody rx(), ry(), and rz()
            EulerAngle eFinal;
            convertFromQuaternion(eFinal, finalQ);

            m_RobotModel->rx() = eFinal.alpha();
            m_RobotModel->ry() = eFinal.beta();
            m_RobotModel->rz() = eFinal.gamma();

            //set new angles for first polyhedron
            //NOTE:: This works for **FREE** robots

            currCfg[3] =  m_RobotModel->rx()/PI;
            currCfg[4] =  m_RobotModel->ry()/PI;
            currCfg[5] =  m_RobotModel->rz()/PI;

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

            GLModel* rm = robotList.front();

            //get new rotation from GL
            Quaternion qrm;
            qrm = m_RobotModel->q();

            //multiply polyhedron0 and multiBody quaternions
            //to get new rotation
            Quaternion finalQ;
            finalQ = qrm * rm->q();

            //set new rotation angles to multiBody rx(), ry(), and rz()
            EulerAngle eFinal;
            convertFromQuaternion(eFinal, finalQ);

            m_RobotModel->rx() = eFinal.alpha();
            m_RobotModel->ry() = eFinal.beta();
            m_RobotModel->rz() = eFinal.gamma();

            //set new angles for first polyhedron
            //NOTE:: This works for **FREE** robots

            currCfg[3] =  m_RobotModel->rx()/PI;
            currCfg[4] =  m_RobotModel->ry()/PI;
            currCfg[5] =  m_RobotModel->rz()/PI;

         }
         else{
            //robot has not been moved before
            currCfg[0] = m_RobotModel->tx() + cfg[0][0];
            currCfg[1] = m_RobotModel->ty() + cfg[0][1];
            currCfg[2] = m_RobotModel->tz() + cfg[0][2];
            currCfg[3] = vRot[0]/PI;
            currCfg[4] = vRot[1]/PI;
            currCfg[5] = vRot[2]/PI;
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

      GLModel* rm = robotList.front();

      //get new rotation from multiBody
      Quaternion qrm;
      qrm = m_RobotModel->q();

      //multiply polyhedron0 and multiBody quaternions
      //to get new rotation
      Quaternion finalQ;
      finalQ = qrm * rm->q();

      //set new rotation angles to multiBody rx(), ry(), and rz()
      EulerAngle eFinal;
      convertFromQuaternion(eFinal, finalQ);

      m_RobotModel->rx() = eFinal.alpha();
      m_RobotModel->ry() = eFinal.beta();
      m_RobotModel->rz() = eFinal.gamma();

      //set new angles for first polyhedron
      //NOTE:: This works for **FREE** robots

      currCfg[3] =  m_RobotModel->rx()/PI;
      currCfg[4] =  m_RobotModel->ry()/PI;
      currCfg[5] =  m_RobotModel->rz()/PI;

      vCfg.push_back(currCfg);
      this->storeCfg(vCfg, ch, dof);

   }//else articulated

}


vector<double> RobotModel::getFinalCfg(){

  int dof = m_envModel->GetDOF();
  //to store actual cfg
  vector<double> currCfg(dof);
  //to store cfg. to be sent to Robot::setStart
  vector<double *> vCfg;

  GLModel * rl;

  list<GLModel*> robotList;
  m_RobotModel->GetChildren(robotList);

  //bool tool = false;

  if(dof == 6){ //rigid body

    //if user didn't move robot by hand, this means
    //the animation tool was used
    if(m_RobotModel->tx() == 0 && m_RobotModel->ty() == 0 && m_RobotModel->tz() == 0){
      typedef list<GLModel *>::iterator RI;

      for(RI i=robotList.begin(); i!=robotList.end(); i++){
        rl = (GLModel*)(*i);

        currCfg[0] = rl->tx();
        currCfg[1] = rl->ty();
        currCfg[2] = rl->tz();


        //Need to compute rotation from Quaternion

        GLModel* rm = robotList.front();

        //get new rotation from GL
        Quaternion qrm;
        qrm = m_RobotModel->q();

        //multiply polyhedron0 and multiBody quaternions
        //to get new rotation
        Quaternion finalQ = qrm * rm->q();

        //set new rotation angles to multiBody rx(), ry(), and rz()
        EulerAngle eFinal;
        convertFromQuaternion(eFinal, finalQ);

        m_RobotModel->rx() = eFinal.alpha();
        m_RobotModel->ry() = eFinal.beta();
        m_RobotModel->rz() = eFinal.gamma();

        //set new angles for first polyhedron
        //NOTE:: This works for **FREE** robots

        currCfg[3] =  m_RobotModel->rx()/PI;
        currCfg[4] =  m_RobotModel->ry()/PI;
        currCfg[5] =  m_RobotModel->rz()/PI;
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

        GLModel* rm = robotList.front();

        //get new rotation from GL
        Quaternion qrm;
        qrm = m_RobotModel->q();

        //multiply polyhedron0 and multiBody quaternions
        //to get new rotation
        Quaternion finalQ;
        finalQ = qrm * rm->q();

        //set new rotation angles to multiBody rx(), ry(), and rz()
        EulerAngle eFinal;
        convertFromQuaternion(eFinal, finalQ);

        m_RobotModel->rx() = eFinal.alpha();
        m_RobotModel->ry() = eFinal.beta();
        m_RobotModel->rz() = eFinal.gamma();

        //set new angles for first polyhedron
        //NOTE:: This works for **FREE** robots

        currCfg[3] =  m_RobotModel->rx()/PI;
        currCfg[4] =  m_RobotModel->ry()/PI;
        currCfg[5] =  m_RobotModel->rz()/PI;

      }
      else{
        currCfg[0] = m_RobotModel->tx();
        currCfg[1] = m_RobotModel->ty();
        currCfg[2] = m_RobotModel->tz();

        currCfg[3] = vRot[0]/PI;
        currCfg[4] = vRot[1]/PI;
        currCfg[5] = vRot[2]/PI;
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

    GLModel* rm = robotList.front();

    //get new rotation from multiBody
    Quaternion qrm;
    qrm = m_RobotModel->q();

    //multiply polyhedron0 and multiBody quaternions
    //to get new rotation
    Quaternion finalQ = qrm * rm->q();

    //set new rotation angles to multiBody rx(), ry(), and rz()
    EulerAngle eFinal;
    convertFromQuaternion(eFinal, finalQ);

    m_RobotModel->rx() = eFinal.alpha();
    m_RobotModel->ry() = eFinal.beta();
    m_RobotModel->rz() = eFinal.gamma();

    //set new angles for first polyhedron
    //NOTE:: This works for **FREE** robots

    currCfg[3] =  m_RobotModel->rx()/PI;
    currCfg[4] =  m_RobotModel->ry()/PI;
    currCfg[5] =  m_RobotModel->rz()/PI;


    m_RobotModel->SetCfg(currCfg);

    return currCfg;


  }//else articulated

}


int
RobotModel::getNumJoints(){

   m_RobotInfo = m_envModel->GetMultiBodyInfo();
   return m_RobotInfo[0].m_NumberOfConnections;
}




bool RobotModel::KP( QKeyEvent * e )
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

void RobotModel::Transform(int dir){
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







