#include "RobotModel.h"

#include "Models/Vizmo.h"
#include "Plum/EnvObj/ConnectionModel.h"
#include "Plum/EnvObj/BodyModel.h"
#include "Models/PolyhedronModel.h"
#include "Utilities/Exceptions.h"

RobotModel::RobotModel(EnvModel* _env){
  m_envModel = _env;
  dof = 1000;
  m_robotModel = NULL;
  mode = 0;
  delta = .1;
  phantomdelta = .1;
  tempCfg = new double[dof];
  currentCfg = new double[dof];
  rotation_axis = new double[3];
  mbRobotBackUp = NULL;
  RotFstBody = NULL;
  queryCfg = NULL;
  StCfg = NULL;
  m_renderModeBackUp = INVISIBLE_MODE;
  pthread_mutex_init(&mutex, NULL);
  BuildModels();
}

/*RobotModel::RobotModel(const RobotModel& _otherRobot)
  : GLModel(_otherRobot) {
    m_envModel = _otherRobot.GetEnvModel();
    m_robotModel = _otherRobot.getRobotModel();
  }
*/

MultiBodyModel* RobotModel::getRobotModel() const {
  return m_robotModel;
}

RobotModel::~RobotModel() {
  delete [] tempCfg;
  delete [] currentCfg;
  delete [] rotation_axis;
}

void
RobotModel::BuildModels() {
  //find robot
  const vector<MultiBodyModel*>& multibodies = m_envModel->GetMultiBodies();
  typedef vector<MultiBodyModel*>::const_iterator MIT;
  for(MIT mit = multibodies.begin(); mit!=multibodies.end(); ++mit) {
    if((*mit)->IsActive()){
      m_robotModel = *mit;
      m_originalColor = (*mit)->GetColor();
      SetColor(m_originalColor);
      break;
    }
  }

  if(!m_robotModel)
    throw BuildException(WHERE, "RobotModel is null, no active body loaded.");
}

void RobotModel::Draw(GLenum _mode){
  if( m_robotModel==NULL){cout<<"m_robotModel==NULL"<<endl; return;}
  glPushMatrix();
  glTransform();
  m_robotModel->Draw(mode);
  glPopMatrix();
  //getFinalCfg();
}

void RobotModel::Select( unsigned int * index, vector<GLModel*>& sel ){
  //unselect old one
  if( index==NULL ) return;
  m_robotModel->Select(index+1,sel);
  //getFinalCfg();
}


void RobotModel::DrawSelect()
{
  if( m_robotModel==NULL ) return;
  glPushMatrix();
  glTransform();
  m_robotModel->DrawSelect();
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
  BodyModel* body = *m_robotModel->Begin();
  m_originalColor = body->GetColor();
  originalSize[0]=sx();
  originalSize[1]=sy();
  originalSize[2]=sz();
}

void RobotModel::RestoreInitCfg(){
  //reset MultiBody values to 0's
  m_robotModel->tx() = m_robotModel->ty() = m_robotModel->tz() = 0;
  double z = 0.0;
  double cx_2=cos(z); double sx_2=sin(z);
  double cy_2=cos(z); double sy_2=sin(z);
  double cz_2=cos(z); double sz_2=sin(z);

  Quaternion qx(cx_2,sx_2*Vector3d(1,0,0));
  Quaternion qy(cy_2,sy_2*Vector3d(0,1,0));
  Quaternion qz(cz_2,sz_2*Vector3d(0,0,1));
  Quaternion nq=qz*qy*qx; //new q

  m_robotModel->q(nq.normalize());

  Configure(StCfg);

  //set robot to its current color and original size
  this->Scale(originalSize[0],originalSize[1],originalSize[2]);
  //this->SetColor(originalR,originalG,originalB,this->GetColor()[3]);
  BodyModel* body = *m_robotModel->Begin();
  this->SetColor(body->GetColor());
}

void RobotModel::BackUp() {
  m_renderModeBackUp = m_renderMode;

  BodyModel* body = *m_robotModel->Begin();

  m_polyBack[0] = body->tx();
  m_polyBack[1] = body->ty();
  m_polyBack[2] = body->tz();

  color = body->GetColor();

  o_s[0]=sx();
  o_s[1]=sy();
  o_s[2]=sz();

  //save current cfg. of robot
  //and set m_robotModel to zero

  if(mbRobotBackUp == NULL)
    mbRobotBackUp = new double[6];
  mbRobotBackUp[0]= m_robotModel->tx();
  mbRobotBackUp[1]= m_robotModel->ty();
  mbRobotBackUp[2]= m_robotModel->tz();

  m_robotModel->tx() =  m_robotModel->ty() =  m_robotModel->tz() = 0;

  MBq = m_robotModel->q();
}

void RobotModel::Restore(){
  this->Scale(o_s[0],o_s[1],o_s[2]);

  this->SetColor(color);

  //restore multibody

  m_robotModel->tx() = mbRobotBackUp[0];
  m_robotModel->ty() = mbRobotBackUp[1];
  m_robotModel->tz() = mbRobotBackUp[2];

  m_robotModel->q(MBq);

  SetRenderMode(m_renderModeBackUp);
  RestoreInitCfg();
}

void RobotModel::Configure(double* cfg) {
  pthread_mutex_lock(&mutex);

  vector<Robot>& robots = m_envModel->GetRobots();

  Vector3d position;
  Orientation orientation;

  int myDOF = m_envModel->GetDOF();

  //to keep Cfgs. for bodies other than FirstLink
  if(RotFstBody == NULL)
    RotFstBody = new double[myDOF];
  for(int i = 0; i<myDOF; i++){
    RotFstBody[i] = cfg[i];
  }

  for(MultiBodyModel::BodyIter bit = m_robotModel->Begin();
      bit != m_robotModel->End(); ++bit)
    (*bit)->ResetTransform();

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

    BodyModel* body = *(m_robotModel->Begin() + rit->m_bodyIndex);
    //step from PMPL
    body->tx()=x;
    body->ty()=y;
    body->tz()=z;

    body->rx() = alpha;
    body->ry() = beta;
    body->rz() = gamma;

    Transformation t(Vector3d(x, y, z),
        Orientation(EulerAngle(gamma, beta, alpha)));
    body->SetTransform(t);
    body->SetTransformDone(true);

    //compute rotation
    Quaternion q;
    convertFromMatrix(q, t.rotation().matrix());
    body->q(q.normalized()); //set new q

    //now for the joints of the robot
    //this code from PMPL
    //configuration of links after the base
    //Compute position and orientation for all of the links left
    typedef Robot::JointMap::const_iterator MIT;
    for(MIT mit = rit->GetJointMap().begin(); mit!=rit->GetJointMap().end(); mit++){
      double theta = cfg[index] * PI;
      index++;
      double alpha = 0;
      if((*mit)->GetJointType() == ConnectionModel::SPHERICAL){
        alpha = cfg[index] * PI;
        index++;
      }

      size_t currentBodyIdx = (*mit)->GetPreviousIndex(); //index of current Body
      BodyModel* currentBody = *(m_robotModel->Begin() + currentBodyIdx);
      size_t nextBodyIdx = (*mit)->GetNextIndex(); //index of next Body
      BodyModel* nextBody = *(m_robotModel->Begin() + nextBodyIdx);

      for(BodyModel::ConnectionIter cit = currentBody->Begin(); cit!=currentBody->End(); ++cit){
        if((*cit)->GetPreviousIndex() == currentBodyIdx && (*cit)->GetNextIndex() == nextBodyIdx){
          (*cit)->SetAlpha(alpha);
          (*cit)->SetTheta(theta);
          break;
        }
      }

      if(!nextBody->IsTransformDone()) {

        nextBody->SetPrevTransform(currentBody->GetTransform());
        nextBody->ComputeTransform(currentBody, nextBodyIdx);
        nextBody->SetTransformDone(true);

        nextBody->tx() = nextBody->GetTransform().translation()[0];
        nextBody->ty() = nextBody->GetTransform().translation()[1];
        nextBody->tz() = nextBody->GetTransform().translation()[2];

        //compute rotation
        Quaternion q;
        convertFromMatrix(q,
            nextBody->GetTransform().rotation().matrix());
        nextBody->q(q.normalized()); //set new q
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
  m_robotModel->GetChildren(robotList);

  if(dof == 6){ //rigid body

    //if user didn't move robot by hand, this means
    //the animation tool was used
    if(m_robotModel->tx() == 0 && m_robotModel->ty() == 0 && m_robotModel->tz() == 0){
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
        qrm = m_robotModel->q();

        //multiply polyhedron0 and multiBody quaternions
        //to get new rotation
        Quaternion finalQ;
        finalQ = qrm * rm->q();

        //set new rotation angles to multiBody rx(), ry(), and rz()
        EulerAngle eFinal;
        convertFromQuaternion(eFinal, finalQ);

        m_robotModel->rx() = eFinal.alpha();
        m_robotModel->ry() = eFinal.beta();
        m_robotModel->rz() = eFinal.gamma();

        //set new angles for first polyhedron
        //NOTE:: This works for **FREE** robots

        currCfg[3] =  m_robotModel->rx()/PI;
        currCfg[4] =  m_robotModel->ry()/PI;
        currCfg[5] =  m_robotModel->rz()/PI;

        //currCfg[3] = rl->rx();
        //currCfg[4] = rl->ry();
        //currCfg[5] = rl->rz();
      }
    }
    else{ //user moved robot by hand

      Matrix3x3 m = m_robotModel->getMatrix();
      Vector3d vRot;
      vRot = m_robotModel->MatrixToEuler(m);
      //robot has not been moved before
      currCfg[0] = m_robotModel->tx() + cfg[0][0];
      currCfg[1] = m_robotModel->ty() + cfg[0][1];
      currCfg[2] = m_robotModel->tz() + cfg[0][2];
      currCfg[3] = vRot[0]/PI;
      currCfg[4] = vRot[1]/PI;
      currCfg[5] = vRot[2]/PI;

    }

    vCfg.push_back(currCfg);
    this->storeCfg(vCfg, ch, dof);

  }

  else{//articulated

    currCfg = this->returnCurrCfg(dof);
    //add translation of MBody
    currCfg[0] = currCfg[0] + m_robotModel->tx();
    currCfg[1] = currCfg[1] + m_robotModel->ty();
    currCfg[2] = currCfg[2] + m_robotModel->tz();

    //Need to compute rotation from Quaternion

    GLModel* rm = robotList.front();

    //get new rotation from multiBody
    Quaternion qrm;
    qrm = m_robotModel->q();

    //multiply polyhedron0 and multiBody quaternions
    //to get new rotation
    Quaternion finalQ;
    finalQ = qrm * rm->q();

    //set new rotation angles to multiBody rx(), ry(), and rz()
    EulerAngle eFinal;
    convertFromQuaternion(eFinal, finalQ);

    m_robotModel->rx() = eFinal.alpha();
    m_robotModel->ry() = eFinal.beta();
    m_robotModel->rz() = eFinal.gamma();

    //set new angles for first polyhedron
    //NOTE:: This works for **FREE** robots

    currCfg[3] =  m_robotModel->rx()/PI;
    currCfg[4] =  m_robotModel->ry()/PI;
    currCfg[5] =  m_robotModel->rz()/PI;

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
  m_robotModel->GetChildren(robotList);

  //bool tool = false;

  if(dof == 6){ //rigid body

    //if user didn't move robot by hand, this means
    //the animation tool was used
    if(m_robotModel->tx() == 0 && m_robotModel->ty() == 0 && m_robotModel->tz() == 0){
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
        qrm = m_robotModel->q();

        //multiply polyhedron0 and multiBody quaternions
        //to get new rotation
        Quaternion finalQ = qrm * rm->q();

        //set new rotation angles to multiBody rx(), ry(), and rz()
        EulerAngle eFinal;
        convertFromQuaternion(eFinal, finalQ);

        m_robotModel->rx() = eFinal.alpha();
        m_robotModel->ry() = eFinal.beta();
        m_robotModel->rz() = eFinal.gamma();

        //set new angles for first polyhedron
        //NOTE:: This works for **FREE** robots

        currCfg[3] =  m_robotModel->rx()/PI;
        currCfg[4] =  m_robotModel->ry()/PI;
        currCfg[5] =  m_robotModel->rz()/PI;
      }
    }
    else{ //user moved robot by hand
      Matrix3x3 m = m_robotModel->getMatrix();
      Vector3d vRot;
      vRot = m_robotModel->MatrixToEuler(m);

      currCfg[0] = m_robotModel->tx();
      currCfg[1] = m_robotModel->ty();
      currCfg[2] = m_robotModel->tz();

      currCfg[3] = vRot[0]/PI;
      currCfg[4] = vRot[1]/PI;
      currCfg[5] = vRot[2]/PI;
    }

    return currCfg;
  }
  else{//articulated

    currCfg = this->returnCurrCfg(dof);
    //add translation of MBody
    cout << "currCfg::" << currCfg[0] << endl;
    currCfg[0] = currCfg[0] + m_robotModel->tx();
    currCfg[1] = currCfg[1] + m_robotModel->ty();
    currCfg[2] = currCfg[2] + m_robotModel->tz();

    //Need to compute rotation from Quaternion

    GLModel* rm = robotList.front();

    //get new rotation from multiBody
    Quaternion qrm;
    qrm = m_robotModel->q();

    //multiply polyhedron0 and multiBody quaternions
    //to get new rotation
    Quaternion finalQ = qrm * rm->q();

    //set new rotation angles to multiBody rx(), ry(), and rz()
    EulerAngle eFinal;
    convertFromQuaternion(eFinal, finalQ);

    m_robotModel->rx() = eFinal.alpha();
    m_robotModel->ry() = eFinal.beta();
    m_robotModel->rz() = eFinal.gamma();

    //set new angles for first polyhedron
    //NOTE:: This works for **FREE** robots

    currCfg[3] =  m_robotModel->rx()/PI;
    currCfg[4] =  m_robotModel->ry()/PI;
    currCfg[5] =  m_robotModel->rz()/PI;

    return currCfg;
  }//else articulated
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
      prx = m_robotModel->rx();
      pry = m_robotModel->ry();
      prz = m_robotModel->rz();

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
      qrm = m_robotModel->q();

      //multiply polyhedron0 and multiBody quaternions //to get new rotation
      finalQ = qrm * qt0;

      //set new rotation angles to multiBody rx(), ry(), and rz()
      mFinal = finalQ.getMatrix();
      vFinal = finalQ.MatrixToEuler(mFinal);

      m_robotModel->rx() = vFinal[0];
      m_robotModel->ry() = vFinal[1];
      m_robotModel->rz() = vFinal[2];


      //set new angles for first polyhedron //NOTE:: This works for
      //**FREE** robots

      tempCfg[3] =  vFinal[0]/TwoPI;
      tempCfg[4] =  vFinal[1]/TwoPI;
      tempCfg[5] =  vFinal[2]/TwoPI;


      m_robotModel->setCurrCfg(tempCfg, dof);

      m_robotModel->Euler2Quaternion();
      GetVizmo().TurnOn_CD();
      if(GetPhantomManager().Collision > .5 && !GetPhantomManager().UseFeedback){
        m_robotModel->setCurrCfg(currentCfg, dof);
        return true;
      }
      m_robotModel->Euler2Quaternion();
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
        m_robotModel->tx() += dx;
        m_robotModel->ty() += dy;
        m_robotModel->tz() += dz;
        tempCfg[0] += dx;
        tempCfg[1] += dy;
        tempCfg[2] += dz;
      }
      GetVizmo().TurnOn_CD();
      if(GetPhantomManager().Collision > .5 && !GetPhantomManager().UseFeedback){
        //m_robotModel->tx() -= dx;
        //m_robotModel->ty() -= dy;
        //m_robotModel->tz() -= dz;
        tempCfg[0] -= dx;
        tempCfg[1] -= dy;
        tempCfg[2] -= dz;
        return true;
      }
      //m_robotModel->glTransform();
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
      m_robotModel->tx()+=dir*delta;
      break;
    case 1:
      m_robotModel->ty()+=dir*delta;
      break;
    case 2:
      m_robotModel->tz()+=dir*delta;
      break;
    case 3:
      m_robotModel->rx()+=dir*delta;
      break;
    case 4:
      m_robotModel->ry()+=dir*delta;
      break;
    case 5:
      m_robotModel->rz()+=dir*delta;
      break;
  }
  m_robotModel->Euler2Quaternion();
  m_robotModel->glTransform();
}







