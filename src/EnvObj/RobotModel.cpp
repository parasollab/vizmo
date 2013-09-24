#include "RobotModel.h"

#include "Models/Vizmo.h"
#include "Plum/EnvObj/ConnectionModel.h"
#include "Plum/EnvObj/BodyModel.h"
#include "Models/PolyhedronModel.h"
#include "Utilities/Exceptions.h"

RobotModel::RobotModel(EnvModel* _env){
  m_envModel = _env;
  m_robotModel = NULL;
  m_renderModeBackUp = INVISIBLE_MODE;
  BuildModels();
}

MultiBodyModel* RobotModel::getRobotModel() const {
  return m_robotModel;
}

RobotModel::~RobotModel() {
}

void
RobotModel::SetRenderMode(RenderMode _mode) {
  GLModel::SetRenderMode(_mode);
  m_robotModel->SetRenderMode(_mode);
}

void
RobotModel::SetColor(const Color4& _c) {
  GLModel::SetColor(_c);
  m_robotModel->SetColor(_c);
}

void
RobotModel::BuildModels() {
  //find robot
  const vector<MultiBodyModel*>& multibodies = m_envModel->GetMultiBodies();
  typedef vector<MultiBodyModel*>::const_iterator MIT;
  for(MIT mit = multibodies.begin(); mit!=multibodies.end(); ++mit) {
    if((*mit)->IsActive()){
      m_robotModel = *mit;
      SetColor((*mit)->GetColor());
      break;
    }
  }

  if(!m_robotModel)
    throw BuildException(WHERE, "RobotModel is null, no active body loaded.");
}

void RobotModel::Draw(GLenum _mode){
  glPushMatrix();
  GLTransform::Transform();
  m_robotModel->Draw(_mode);
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
  GLTransform::Transform();
  m_robotModel->DrawSelect();
  glPopMatrix();
  //getFinalCfg();
}

void RobotModel::InitialCfg(vector<double>& cfg){
  //save initial Cfg.
  StCfg = cfg;
}

void RobotModel::RestoreInitCfg(){
  //reset MultiBody values to 0's
  m_robotModel->Translation()(0, 0, 0);
  double z = 0.0;
  double cx_2=cos(z); double sx_2=sin(z);
  double cy_2=cos(z); double sy_2=sin(z);
  double cz_2=cos(z); double sz_2=sin(z);

  Quaternion qx(cx_2,sx_2*Vector3d(1,0,0));
  Quaternion qy(cy_2,sy_2*Vector3d(0,1,0));
  Quaternion qz(cz_2,sz_2*Vector3d(0,0,1));
  Quaternion nq=qz*qy*qx; //new q

  m_robotModel->RotationQ() = nq.normalized();

  Configure(StCfg);

  //set robot to its current color and original size
  BodyModel* body = *m_robotModel->Begin();
  this->SetColor(body->GetColor());
}

void RobotModel::BackUp() {
  m_translationBackUp = m_robotModel->Translation();
  m_quaternionBackUp = m_robotModel->RotationQ();

  m_colorBackUp = m_robotModel->GetColor();
  m_renderModeBackUp = m_renderMode;
}

void RobotModel::Restore(){
  m_robotModel->Translation() = m_translationBackUp;
  m_robotModel->RotationQ() = m_quaternionBackUp;

  this->SetColor(m_colorBackUp);
  SetRenderMode(m_renderModeBackUp);

  RestoreInitCfg();
}

void
RobotModel::Configure(vector<double>& _cfg) {

  vector<Robot>& robots = m_envModel->GetRobots();

  Vector3d position;
  Orientation orientation;

  int myDOF = m_envModel->GetDOF();

  //to keep Cfgs. for bodies other than FirstLink
  m_currCfg = _cfg;

  for(MultiBodyModel::BodyIter bit = m_robotModel->Begin();
      bit != m_robotModel->End(); ++bit)
    (*bit)->ResetTransform();

  int index=0;
  typedef vector<Robot>::iterator RIT;
  for(RIT rit = robots.begin(); rit!=robots.end(); rit++){
    int posIndex=index;
    double x=0, y=0, z=0, alpha=0, beta=0, gamma=0;
    if(rit->m_base != Robot::FIXED){
      x = _cfg[posIndex];
      y = _cfg[posIndex+1];
      index+=2;
      if(rit->m_base==Robot::VOLUMETRIC){
        index++;
        z = _cfg[posIndex+2];
      }
      if(rit->m_baseMovement == Robot::ROTATIONAL){
        if(rit->m_base==Robot::PLANAR){
          index++;
          gamma = _cfg[posIndex+2] * PI;
        }
        else{
          index+=3;
          alpha = _cfg[posIndex+3] * PI;
          beta = _cfg[posIndex+4] * PI;
          gamma = _cfg[posIndex+5] * PI;
        }
      }
    }

    BodyModel* body = *(m_robotModel->Begin() + rit->m_bodyIndex);
    //step from PMPL
    body->Translation()(x, y, z);

    body->Rotation()(alpha, beta, gamma);

    Transformation t(Vector3d(x, y, z),
        Orientation(EulerAngle(gamma, beta, alpha)));
    body->SetTransform(t);
    body->SetTransformDone(true);

    //compute rotation
    Quaternion q;
    convertFromMatrix(q, t.rotation().matrix());
    body->RotationQ() = q.normalized(); //set new q

    //now for the joints of the robot
    //this code from PMPL
    //configuration of links after the base
    //Compute position and orientation for all of the links left
    typedef Robot::JointMap::const_iterator MIT;
    for(MIT mit = rit->GetJointMap().begin(); mit!=rit->GetJointMap().end(); mit++){
      double theta = _cfg[index] * PI;
      index++;
      double alpha = 0;
      if((*mit)->GetJointType() == ConnectionModel::SPHERICAL){
        alpha = _cfg[index] * PI;
        index++;
      }

      size_t currentBodyIdx = (*mit)->GetPreviousIndex(); //index of current Body
      BodyModel* currentBody = *(m_robotModel->Begin() + currentBodyIdx);
      size_t nextBodyIdx = (*mit)->GetNextIndex(); //index of next Body
      BodyModel* nextBody = *(m_robotModel->Begin() + nextBodyIdx);

      (*mit)->SetAlpha(alpha);
      (*mit)->SetTheta(theta);

      if(!nextBody->IsTransformDone()) {

        nextBody->SetPrevTransform(currentBody->GetTransform());
        nextBody->ComputeTransform(currentBody, nextBodyIdx);
        nextBody->SetTransformDone(true);

        nextBody->Translation() = nextBody->GetTransform().translation();

        //compute rotation
        Quaternion q;
        convertFromMatrix(q,
            nextBody->GetTransform().rotation().matrix());
        nextBody->RotationQ() = q.normalized(); //set new q
      }
    }
  }
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
    if(m_robotModel->Translation() == Point3d()){
      typedef list<GLModel *>::iterator RI;

      for(RI i=robotList.begin(); i!=robotList.end(); i++){
        rl = (GLModel*)(*i);
        currCfg[0] = rl->Translation()[0];
        currCfg[1] = rl->Translation()[1];
        currCfg[2] = rl->Translation()[2];
        //Need to compute rotation from Quaternion

        GLModel* rm = robotList.front();

        //get new rotation from GL
        Quaternion qrm;
        qrm = m_robotModel->RotationQ();

        //multiply polyhedron0 and multiBody quaternions
        //to get new rotation
        Quaternion finalQ;
        finalQ = qrm * rm->RotationQ();

        //set new rotation angles to multiBody rx(), ry(), and rz()
        EulerAngle eFinal;
        convertFromQuaternion(eFinal, finalQ);

        m_robotModel->Rotation()[0] = eFinal.alpha();
        m_robotModel->Rotation()[1] = eFinal.beta();
        m_robotModel->Rotation()[2] = eFinal.gamma();

        //set new angles for first polyhedron
        //NOTE:: This works for **FREE** robots

        currCfg[3] =  m_robotModel->Rotation()[0]/PI;
        currCfg[4] =  m_robotModel->Rotation()[1]/PI;
        currCfg[5] =  m_robotModel->Rotation()[2]/PI;

        //currCfg[3] = rl->rx();
        //currCfg[4] = rl->ry();
        //currCfg[5] = rl->rz();
      }
    }
    else{ //user moved robot by hand
      EulerAngle e;
      convertFromQuaternion(e, m_robotModel->RotationQ());

      //robot has not been moved before
      currCfg[0] = m_robotModel->Translation()[0] + cfg[0][0];
      currCfg[1] = m_robotModel->Translation()[1] + cfg[0][1];
      currCfg[2] = m_robotModel->Translation()[2] + cfg[0][2];

      currCfg[3] = e.alpha()/PI;
      currCfg[4] = e.beta()/PI;
      currCfg[5] = e.gamma()/PI;
    }

    vCfg.push_back(currCfg);
    this->storeCfg(vCfg, ch, dof);

  }

  else{//articulated

    currCfg = m_currCfg;
    //add translation of MBody
    currCfg[0] = currCfg[0] + m_robotModel->Translation()[0];
    currCfg[1] = currCfg[1] + m_robotModel->Translation()[1];
    currCfg[2] = currCfg[2] + m_robotModel->Translation()[2];

    //Need to compute rotation from Quaternion

    GLModel* rm = robotList.front();

    //get new rotation from multiBody
    Quaternion qrm;
    qrm = m_robotModel->RotationQ();

    //multiply polyhedron0 and multiBody quaternions
    //to get new rotation
    Quaternion finalQ;
    finalQ = qrm * rm->RotationQ();

    //set new rotation angles to multiBody rx(), ry(), and rz()
    EulerAngle eFinal;
    convertFromQuaternion(eFinal, finalQ);

    m_robotModel->Rotation()[0] = eFinal.alpha();
    m_robotModel->Rotation()[1] = eFinal.beta();
    m_robotModel->Rotation()[2] = eFinal.gamma();

    //set new angles for first polyhedron
    //NOTE:: This works for **FREE** robots

    currCfg[3] =  m_robotModel->Rotation()[0]/PI;
    currCfg[4] =  m_robotModel->Rotation()[1]/PI;
    currCfg[5] =  m_robotModel->Rotation()[2]/PI;

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
    if(m_robotModel->Translation() == Point3d()){
      typedef list<GLModel *>::iterator RI;

      for(RI i=robotList.begin(); i!=robotList.end(); i++){
        rl = (GLModel*)(*i);

        currCfg[0] = rl->Translation()[0];
        currCfg[1] = rl->Translation()[1];
        currCfg[2] = rl->Translation()[2];


        //Need to compute rotation from Quaternion

        GLModel* rm = robotList.front();

        //get new rotation from GL
        Quaternion qrm;
        qrm = m_robotModel->RotationQ();

        //multiply polyhedron0 and multiBody quaternions
        //to get new rotation
        Quaternion finalQ = qrm * rm->RotationQ();

        //set new rotation angles to multiBody rx(), ry(), and rz()
        EulerAngle eFinal;
        convertFromQuaternion(eFinal, finalQ);

        m_robotModel->Rotation()[0] = eFinal.alpha();
        m_robotModel->Rotation()[1] = eFinal.beta();
        m_robotModel->Rotation()[2] = eFinal.gamma();

        //set new angles for first polyhedron
        //NOTE:: This works for **FREE** robots

        currCfg[3] =  m_robotModel->Rotation()[0]/PI;
        currCfg[4] =  m_robotModel->Rotation()[1]/PI;
        currCfg[5] =  m_robotModel->Rotation()[2]/PI;
      }
    }
    else{ //user moved robot by hand
      EulerAngle e;
      convertFromQuaternion(e, m_robotModel->RotationQ());

      currCfg[0] = m_robotModel->Translation()[0];
      currCfg[1] = m_robotModel->Translation()[1];
      currCfg[2] = m_robotModel->Translation()[2];

      currCfg[3] = e.alpha()/PI;
      currCfg[4] = e.beta()/PI;
      currCfg[5] = e.gamma()/PI;
    }

    return currCfg;
  }
  else{//articulated

    currCfg = m_currCfg;
    //add translation of MBody
    cout << "currCfg::" << currCfg[0] << endl;
    currCfg[0] = currCfg[0] + m_robotModel->Translation()[0];
    currCfg[1] = currCfg[1] + m_robotModel->Translation()[1];
    currCfg[2] = currCfg[2] + m_robotModel->Translation()[2];

    //Need to compute rotation from Quaternion

    GLModel* rm = robotList.front();

    //get new rotation from multiBody
    Quaternion qrm;
    qrm = m_robotModel->RotationQ();

    //multiply polyhedron0 and multiBody quaternions
    //to get new rotation
    Quaternion finalQ = qrm * rm->RotationQ();

    //set new rotation angles to multiBody rx(), ry(), and rz()
    EulerAngle eFinal;
    convertFromQuaternion(eFinal, finalQ);

    m_robotModel->Rotation()[0] = eFinal.alpha();
    m_robotModel->Rotation()[1] = eFinal.beta();
    m_robotModel->Rotation()[2] = eFinal.gamma();

    //set new angles for first polyhedron
    //NOTE:: This works for **FREE** robots

    currCfg[3] =  m_robotModel->Rotation()[0]/PI;
    currCfg[4] =  m_robotModel->Rotation()[1]/PI;
    currCfg[5] =  m_robotModel->Rotation()[2]/PI;

    return currCfg;
  }//else articulated
}

void
RobotModel::storeCfg(vector<vector<double> >& cfg, char c, int dof){
  typedef vector<vector<double> >::iterator IC;
  if(c == 's'){
    StartCfg.clear();
    for(IC ic=cfg.begin(); ic!=cfg.end(); ic++){
      StartCfg.push_back(*ic);
    }
  }
  else{
    GoalCfg.clear();
    for(IC ic=cfg.begin(); ic!=cfg.end(); ic++){
      GoalCfg.push_back(*ic);
    }
  }
}

vector<vector<double> >
RobotModel::getNewStartAndGoal() {
  vector<vector<double> > v;
  if(!StartCfg.empty())
    v.push_back(StartCfg[0]);
  if(!GoalCfg.empty())
    v.push_back(GoalCfg[0]);
  return v;
}

