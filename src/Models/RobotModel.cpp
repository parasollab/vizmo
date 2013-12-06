#include "RobotModel.h"

#include "BodyModel.h"
#include "ConnectionModel.h"
#include "EnvModel.h"
#include "Vizmo.h"
#include "Utilities/Exceptions.h"

RobotModel::RobotModel(EnvModel* _env) : Model("Robot"), m_envModel(_env) {
  BuildModels();
}

void
RobotModel::SetRenderMode(RenderMode _mode) {
  Model::SetRenderMode(_mode);
  m_robotModel->SetRenderMode(_mode);
}

void
RobotModel::SetColor(const Color4& _c) {
  Model::SetColor(_c);
  m_robotModel->SetColor(_c);
}

void
RobotModel::BackUp() {
  m_renderModeBackUp = m_renderMode;
  m_colorBackUp = m_robotModel->GetColor();
}

void
RobotModel::Configure(vector<double>& _cfg) {

  m_currCfg = _cfg;
  const MultiBodyModel::Robots& robots = m_robotModel->GetRobots();

  Vector3d position;
  Orientation orientation;

  for(MultiBodyModel::BodyIter bit = m_robotModel->Begin();
      bit != m_robotModel->End(); ++bit)
    (*bit)->ResetTransform();

  int index = 0;
  typedef MultiBodyModel::Robots::const_iterator RIT;
  for(RIT rit = robots.begin(); rit!=robots.end(); rit++){
    int posIndex = index;
    double x = 0, y = 0, z = 0, alpha = 0, beta = 0, gamma = 0;
    BodyModel* base = rit->first;
    if(!base->IsBaseFixed()) {
      x = _cfg[posIndex];
      y = _cfg[posIndex+1];
      index += 2;
      if(base->IsBaseVolumetric()) {
        index++;
        z = _cfg[posIndex+2];
      }
      if(base->IsBaseRotational()) {
        if(base->IsBasePlanar()) {
          index++;
          gamma = _cfg[posIndex+2] * PI;
        }
        else {
          index += 3;
          alpha = _cfg[posIndex+3] * PI;
          beta = _cfg[posIndex+4] * PI;
          gamma = _cfg[posIndex+5] * PI;
        }
      }
    }

    base->Translation()(x, y, z);
    base->Rotation()(alpha, beta, gamma);

    Transformation t(Vector3d(x, y, z),
        Orientation(EulerAngle(gamma, beta, alpha)));
    base->SetTransform(t);
    base->SetTransformDone(true);

    //compute rotation
    Quaternion q;
    convertFromMatrix(q, t.rotation().matrix());
    base->RotationQ() = q.normalized(); //set new q

    //now for the joints of the robot
    //configuration of links after the base
    //Compute position and orientation for all of the links left
    typedef MultiBodyModel::Joints::const_iterator MIT;
    for(MIT mit = rit->second.begin(); mit!=rit->second.end(); mit++){
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

void
RobotModel::Restore() {
  SetRenderMode(m_renderModeBackUp);
  SetColor(m_colorBackUp);
  Configure(m_initCfg);
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

void
RobotModel::Select(unsigned int* _index, vector<Model*>& _sel) {
  if(!_index) return;
  m_robotModel->Select(_index+1, _sel);
}

void
RobotModel::Draw(GLenum _mode) {
  glPushMatrix();
  m_robotModel->Draw(_mode);
  glPopMatrix();
}

void
RobotModel::DrawSelect() {
  glPushMatrix();
  m_robotModel->DrawSelect();
  glPopMatrix();
}

void
RobotModel::Print(ostream& _os) const {
  _os << Name() << endl;
}
