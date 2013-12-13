#include "EditRobotDialog.h"

#include <iostream>
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QListWidget>
#include <QValidator>
#include <QMessageBox>
#include <QFileDialog>

#include "MainWindow.h"
#include "GLWidget.h"
#include "ModelSelectionWidget.h"
#include "Models/BodyModel.h"
#include "Models/ConnectionModel.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

#define DEFAULT_COLOR Color4(0.0,0.0,0.0,1)
#define DEFAULT_BASE_CONF "  FIXED  0 0 0 0 0 0 \n"
#define DEFAULT_JOINT_CONF "0 1  REVOLUTE -1:1\n0 0 0 0 0 0  0 0 0 0  0 0 0 0 0 0\n"

EditRobotDialog::EditRobotDialog(MainWindow* _mainWindow, QWidget* _parent) : QDialog(_parent) {
  resize(650, 480);
  setWindowTitle("EditRobot");
  m_okButton = new QPushButton("Leave", this);
  m_baseAreaLabel = new QLabel("<b>Bases:<b>", this);
  m_jointAreaLabel = new QLabel("<b>Joints:<b>", this);
  m_attributesLabel = new QLabel("<b>Attributes:<b>", this);
  m_directoryLine = new QLineEdit(this);
  m_directoryLabel = new QLabel("Directory:", this);
  m_addBaseButton = new QPushButton("Add", this);
  m_deleteBaseButton = new QPushButton("Delete", this);
  m_addJointButton = new QPushButton("Add", this);
  m_deleteJointButton = new QPushButton("Delete", this);
  m_baseList = new QListWidget(this);
  m_jointList = new QListWidget(this);
  m_baseFixedCheck = new QCheckBox("fixed", this);
  m_basePlanarCheck = new QCheckBox("planar", this);
  m_baseVolumetricCheck = new QCheckBox("volumetric", this);
  m_baseTranslationalCheck = new QCheckBox("translational", this);
  m_baseRotationalCheck = new QCheckBox("rotational", this);
  m_baseTypeLabel = new QLabel("Type:", this);
  m_baseMovementLabel = new QLabel("Movement:", this);
  m_validator = new QDoubleValidator();
  m_messageBox = new QMessageBox(this);
  m_jointSphericalCheck = new QCheckBox("Spherical", this);
  m_jointTypeLabel = new QLabel("type:", this);
  m_jointRevoluteCheck = new QCheckBox("Revolute", this);
  m_jointNonActuatedCheck = new QCheckBox("Non Actuated", this);
  m_jointLimitsLabel = new QLabel("Limits:", this);
  m_jointLimitLabel1 = new QLabel("X:", this);
  m_jointLimitLabel2 = new QLabel("Y:", this);
  m_jointPositionLabel11 = new QLabel("Start position from", this);
  m_jointPositionLabel12 = new QLabel("last connection:", this);
  for(int j=0; j<2; j++){
    for(int i=0; i<2; i++)
      m_jointLimits[j][i] = new QDoubleSpinBox(this);
    for(int i=0; i<6; i++)
      m_jointPos[j][i] = new QDoubleSpinBox(this);
    m_jointXLabel[j] = new QLabel("X", this);
    m_jointYLabel[j] = new QLabel("Y", this);
    m_jointZLabel[j] = new QLabel("Z", this);
    m_jointAlphaLabel[j] = new QLabel(QChar(0x03B1), this);
    m_jointBetaLabel[j] = new QLabel(QChar(0x03B2), this);
    m_jointGammaLabel[j] = new QLabel(QChar(0x03B3), this);
  }
  m_jointPositionLabel21 = new QLabel("End position from", this);
  m_jointPositionLabel22 = new QLabel("last connection:", this);
  m_jointDHParametersLabel1 = new QLabel("Denavit-Hartenberg", this);
  m_jointDHParametersLabel2 = new QLabel("parameters:", this);
  m_jointAlphaLine = new QLineEdit(this);
  m_jointALine = new QLineEdit(this);
  m_jointDLine = new QLineEdit(this);
  m_jointThetaLine = new QLineEdit(this);
  m_jointAlphaDHLabel = new QLabel(QChar(0x03B1),this);
  m_jointALabel = new QLabel("a",this);
  m_jointDLabel = new QLabel("d",this);
  m_jointThetaDHLabel = new QLabel(QChar(0x03B8),this);
  m_newRobotButton = new QPushButton("Choose a new base to create a new robot", this);
  m_bodyNumberLabel = new QLabel("Body num:", this);
  m_bodyNumberLine = new QLineEdit(this);
  m_jointConnectionsLabel = new QLabel("Connected to bodies num:", this);
  m_jointConnectionsLine1 = new QLineEdit(this);
  m_jointConnectionsLine2 = new QLineEdit(this);

  connect(m_okButton, SIGNAL(clicked()), this, SLOT(Accept()));
  connect(m_baseList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(ShowBase()));
  connect(m_jointList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(ShowJoint()));
  connect(m_baseFixedCheck, SIGNAL(clicked()), this, SLOT(BaseFixedChecked()));
  connect(m_basePlanarCheck, SIGNAL(clicked()), this, SLOT(BasePlanarChecked()));
  connect(m_baseVolumetricCheck, SIGNAL(clicked()), this, SLOT(BaseVolumetricChecked()));
  connect(m_baseRotationalCheck, SIGNAL(clicked()), this, SLOT(BaseRotationalChecked()));
  connect(m_baseTranslationalCheck, SIGNAL(clicked()), this, SLOT(BaseTranslationalChecked()));
  connect(m_deleteBaseButton, SIGNAL(clicked()), this, SLOT(DeleteBase()));
  connect(m_addBaseButton, SIGNAL(clicked()), this, SLOT(AddBase()));
  connect(m_newRobotButton, SIGNAL(clicked()), this, SLOT(CreateNewRobot()));
  connect(m_jointNonActuatedCheck, SIGNAL(clicked()), this, SLOT(JointNonActuatedChecked()));
  connect(m_jointSphericalCheck, SIGNAL(clicked()), this, SLOT(JointSphericalChecked()));
  connect(m_jointRevoluteCheck, SIGNAL(clicked()), this, SLOT(JointRevoluteChecked()));
  connect(m_deleteJointButton, SIGNAL(clicked()), this, SLOT(DeleteJoint()));
  connect(m_addJointButton, SIGNAL(clicked()), this, SLOT(AddJoint()));
  connect(m_jointConnectionsLine1, SIGNAL(editingFinished()), this, SLOT(UpdateJoint()));
  connect(m_jointAlphaLine, SIGNAL(editingFinished()), this, SLOT(UpdateJoint()));
  connect(m_jointALine, SIGNAL(editingFinished()), this, SLOT(UpdateJoint()));
  connect(m_jointDLine, SIGNAL(editingFinished()), this, SLOT(UpdateJoint()));
  connect(m_jointThetaLine, SIGNAL(editingFinished()), this, SLOT(UpdateJoint()));
  for(int j=0; j<2; j++){
    for(int i=0; i<2; i++)
      connect(m_jointLimits[j][i], SIGNAL(valueChanged(double)), this, SLOT(UpdateJoint()));
    for(int i=0; i<6; i++)
      connect(m_jointPos[j][i], SIGNAL(valueChanged(double)), this, SLOT(UpdateJoint()));
  }
  connect(m_jointSphericalCheck, SIGNAL(stateChanged(int)), this, SLOT(TypeUpdateJoint(int)));
  connect(m_jointRevoluteCheck, SIGNAL(stateChanged(int)), this, SLOT(TypeUpdateJoint(int)));
  connect(m_jointNonActuatedCheck, SIGNAL(stateChanged(int)), this, SLOT(TypeUpdateJoint(int)));

  vector<MultiBodyModel*> mBody = GetVizmo().GetEnv()->GetMultiBodies();
  typedef vector<MultiBodyModel*>::const_iterator MIT;
  for(MIT mit = mBody.begin(); mit<mBody.end(); mit++){
    if((*mit)->IsActive())
      m_robotBody=*mit;
  }
  m_newRobotModel=m_robotBody->GetRobots();
  m_jointIsInit=false;
  m_baseIsInit=false;
  m_mainWindow=_mainWindow;

  SaveJointsNames();
  SetUpLayout();
  DisplayBases();
}

void
EditRobotDialog::SetUpLayout(){
  m_okButton->setGeometry(QRect(510, 440, 131, 31));
  m_okButton->setAutoDefault(false);
  m_baseAreaLabel->setGeometry(QRect(20, 70, 67, 21));
  m_jointAreaLabel->setGeometry(QRect(350, 70, 67, 21));
  m_attributesLabel->setGeometry(QRect(20, 210, 81, 21));
  m_directoryLine->setGeometry(QRect(100, 230, 451, 31));
  m_directoryLine->setEnabled(false);
  m_directoryLabel->setGeometry(QRect(30, 230, 71, 21));
  m_addBaseButton->setGeometry(QRect(220, 90, 61, 31));
  m_deleteBaseButton->setGeometry(QRect(220, 130, 61, 31));
  m_addJointButton->setGeometry(QRect(550, 100, 61, 31));
  m_deleteJointButton->setGeometry(QRect(550, 140, 61, 31));
  m_addBaseButton->setAutoDefault(false);
  m_deleteBaseButton->setAutoDefault(false);
  m_addJointButton->setAutoDefault(false);
  m_deleteJointButton->setAutoDefault(false);
  m_baseList->setGeometry(QRect(30, 90, 181, 81));
  m_baseList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  m_jointList->setGeometry(QRect(360, 90, 181, 81));
  m_jointList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  m_baseFixedCheck->setGeometry(QRect(110, 270, 71, 26));
  m_basePlanarCheck->setGeometry(QRect(190, 270, 81, 26));
  m_baseVolumetricCheck->setGeometry(QRect(280, 270, 101, 26));
  m_baseTranslationalCheck->setGeometry(QRect(110, 300, 111, 26));
  m_baseRotationalCheck->setGeometry(QRect(230, 300, 111, 26));
  m_baseTypeLabel->setGeometry(QRect(30, 270, 51, 21));
  m_baseMovementLabel->setGeometry(QRect(30, 300, 81, 21));
  m_jointSphericalCheck->setGeometry(QRect(180, 230, 91, 26));
  m_jointTypeLabel->setGeometry(QRect(30, 230, 41, 21));
  m_jointRevoluteCheck->setGeometry(QRect(80, 230, 91, 26));
  m_jointNonActuatedCheck->setGeometry(QRect(280, 230, 121, 26));
  m_jointLimitsLabel->setGeometry(QRect(30, 260, 41, 21));
  m_jointLimitLabel1->setGeometry(QRect(90, 260, 16, 21));
  m_jointLimitLabel2->setGeometry(QRect(240, 260, 16, 21));
  m_jointPositionLabel11->setGeometry(QRect(30, 300, 131, 16));
  m_jointPositionLabel12->setGeometry(QRect(30, 320, 121, 16));
  for(int j=0; j<2; j++){
    for(int i=0; i<2; i++){
      m_jointLimits[j][i]->setGeometry(QRect(110+60*(i+2*j)+30*j, 260, 61, 21));
      m_jointLimits[j][i]->setRange(-1,1);
      m_jointLimits[j][i]->setSingleStep(0.1);
    }
    for(int i=0; i<6; i++){
      m_jointPos[j][i]->setGeometry(170+80*i, 310+50*j, 71, 21);
      m_jointPos[j][i]->setRange(-1000000, 1000000);
    }
    m_jointXLabel[j]->setGeometry(QRect(190, 290+j*50, 16, 21));
    m_jointYLabel[j]->setGeometry(QRect(270, 290+j*50, 16, 21));
    m_jointZLabel[j]->setGeometry(QRect(350, 290+j*50, 16, 21));
    m_jointAlphaLabel[j]->setGeometry(QRect(440, 290+j*50, 41, 21));
    m_jointBetaLabel[j]->setGeometry(QRect(520, 290+j*50, 41, 21));
    m_jointGammaLabel[j]->setGeometry(QRect(600, 290+j*50, 41, 21));
  }
  m_jointPositionLabel22->setGeometry(QRect(30, 370, 121, 16));
  m_jointPositionLabel21->setGeometry(QRect(30, 350, 131, 16));
  m_jointDHParametersLabel1->setGeometry(QRect(30, 400, 141, 21));  m_jointDHParametersLabel2->setGeometry(QRect(30, 420, 81, 21));
  m_jointAlphaLine->setGeometry(QRect(180, 420, 41, 20));
  m_jointALine->setGeometry(QRect(240, 420, 41, 20));
  m_jointDLine->setGeometry(QRect(300, 420, 41, 20));
  m_jointThetaLine->setGeometry(QRect(360, 420, 41, 20));
  m_jointAlphaLine->setValidator(m_validator);
  m_jointALine->setValidator(m_validator);
  m_jointDLine->setValidator(m_validator);
  m_jointThetaLine->setValidator(m_validator);
  m_jointAlphaDHLabel->setGeometry(QRect(200, 400, 41, 21));
  m_jointALabel->setGeometry(QRect(260, 400, 16, 21));
  m_jointDLabel->setGeometry(QRect(320, 400, 16, 21));
  m_jointThetaDHLabel->setGeometry(QRect(380, 400, 41, 21));
  m_bodyNumberLabel->setGeometry(QRect(210, 180, 71, 21));
  m_bodyNumberLine->setGeometry(QRect(290, 180, 71, 21));
  m_bodyNumberLine->setReadOnly(true);
  m_jointConnectionsLabel->setGeometry(QRect(390, 260, 171, 21));
  m_jointConnectionsLine1->setGeometry(QRect(570, 260, 31, 21));
  m_jointConnectionsLine2->setGeometry(QRect(610, 260, 31, 21));
  m_jointConnectionsLine2->setReadOnly(true);
  m_jointConnectionsLine1->setValidator(m_validator);
  m_jointConnectionsLine2->setValidator(m_validator);
  m_newRobotButton->setGeometry(QRect(160,20,311,31));
  m_newRobotButton->setAutoDefault(false);
  DisplayHideJointAttributes(false);
  DisplayHideBaseAttributes(false);

  QDialog::show();
}

void
EditRobotDialog::DisplayHideJointAttributes(bool _visible){
  m_jointIsInit=_visible;
  m_jointSphericalCheck->setVisible(_visible);
  m_jointTypeLabel->setVisible(_visible);
  m_jointRevoluteCheck->setVisible(_visible);
  m_jointNonActuatedCheck->setVisible(_visible);
  m_jointLimitsLabel->setVisible(_visible);
  m_jointLimitLabel1->setVisible(_visible);
  m_jointLimitLabel2->setVisible(_visible);
  m_jointPositionLabel11->setVisible(_visible);
  m_jointPositionLabel12->setVisible(_visible);
  for(int j=0; j<2; j++){
    for(int i=0; i<2; i++)
      m_jointLimits[j][i]->setVisible(_visible);
    for(int i=0; i<6; i++)
      m_jointPos[j][i]->setVisible(_visible);
    m_jointXLabel[j]->setVisible(_visible);
    m_jointYLabel[j]->setVisible(_visible);
    m_jointZLabel[j]->setVisible(_visible);
    m_jointAlphaLabel[j]->setVisible(_visible);
    m_jointBetaLabel[j]->setVisible(_visible);
    m_jointGammaLabel[j]->setVisible(_visible);
  }
  m_jointPositionLabel22->setVisible(_visible);
  m_jointPositionLabel21->setVisible(_visible);
  m_jointDHParametersLabel1->setVisible(_visible);
  m_jointDHParametersLabel2->setVisible(_visible);
  m_jointAlphaLine->setVisible(_visible);
  m_jointALine->setVisible(_visible);
  m_jointDLine->setVisible(_visible);
  m_jointThetaLine->setVisible(_visible);
  m_jointAlphaDHLabel->setVisible(_visible);
  m_jointALabel->setVisible(_visible);
  m_jointDLabel->setVisible(_visible);
  m_jointThetaDHLabel->setVisible(_visible);
  m_jointConnectionsLabel->setVisible(_visible);
  m_jointConnectionsLine1->setVisible(_visible);
  m_jointConnectionsLine2->setVisible(_visible);
}

void
EditRobotDialog::ClearJointLines(){
  for(int j=0; j<2; j++){
    for(int i=0; i<2; i++)
      m_jointLimits[j][i]->clear();
    for(int i=0; i<6; i++)
      m_jointPos[j][i]->clear();
  }
  m_jointAlphaLine->clear();
  m_jointALine->clear();
  m_jointDLine->clear();
  m_jointThetaLine->clear();
  m_jointConnectionsLine1->clear();
  m_jointConnectionsLine2->clear();
}
void
EditRobotDialog::JointNonActuatedChecked(){
  m_jointSphericalCheck->setChecked(false);
  m_jointRevoluteCheck->setChecked(false);
  m_jointNonActuatedCheck->setChecked(true);
  m_jointLimits[1][0]->setEnabled(false);
  m_jointLimits[1][1]->setEnabled(false);
  JointNewTypeChecked(false);
}

void
EditRobotDialog::JointSphericalChecked(){
  m_jointSphericalCheck->setChecked(true);
  m_jointRevoluteCheck->setChecked(false);
  m_jointNonActuatedCheck->setChecked(false);
  m_jointLimits[1][0]->setEnabled(true);
  m_jointLimits[1][1]->setEnabled(true);
  JointNewTypeChecked(true);
}

void
EditRobotDialog::JointRevoluteChecked(){
  m_jointSphericalCheck->setChecked(false);
  m_jointRevoluteCheck->setChecked(true);
  m_jointNonActuatedCheck->setChecked(false);
  m_jointLimits[1][0]->setEnabled(false);
  m_jointLimits[1][1]->setEnabled(false);
  JointNewTypeChecked(true);
}

void
EditRobotDialog::JointNewTypeChecked(bool _isTrue){
  for(int j=0; j<2; j++)
    m_jointLimits[0][j]->setEnabled(_isTrue);
}

void
EditRobotDialog::DisplayHideBaseAttributes(bool _visible){
  m_baseIsInit=_visible;
  m_directoryLine->setVisible(_visible);
  m_directoryLine->setVisible(_visible);
  m_directoryLabel->setVisible(_visible);
  m_baseFixedCheck->setVisible(_visible);
  m_basePlanarCheck->setVisible(_visible);
  m_baseVolumetricCheck->setVisible(_visible);
  m_baseTranslationalCheck->setVisible(_visible);
  m_baseRotationalCheck->setVisible(_visible);
  m_baseTypeLabel->setVisible(_visible);
  m_baseMovementLabel->setVisible(_visible);
}

void
EditRobotDialog::Accept(){
  RefreshVizmo();
  accept();
}

void
EditRobotDialog::ShowJoint(){
  if(m_baseList->currentItem()!=NULL){
    m_jointIsInit=false;
    DisplayHideBaseAttributes(false);
    ClearJointLines();
    m_bodyNumberLine->clear();
    int indexBase = m_baseList->row(m_baseList->currentItem());
    int indexJoint = m_jointList->row(m_jointList->currentItem());
    typedef Joints::const_iterator MIT;
    int bodyCount=0, baseCount=0, jointCount=0;
    typedef MultiBodyModel::Robots::const_iterator RIT;
    for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++){
      if(baseCount<indexBase){
        for(MIT mit = m_newRobotModel.at(baseCount).second.begin(); mit !=
            m_newRobotModel.at(baseCount).second.end(); ++mit){
          bodyCount++;
        }
        bodyCount++;
        baseCount++;
      }
      else if(baseCount==indexBase){
        for(MIT mit = m_newRobotModel.at(baseCount).second.begin(); mit !=
            m_newRobotModel.at(baseCount).second.end(); ++mit){
          if(jointCount<indexJoint)
            bodyCount++;
          jointCount++;
        }
      }
    }
    m_bodyNumberLine->insert(QString::number(bodyCount+1));
    pair<double, double> jointLimits[2] = m_newRobotModel.at(indexBase).second.at(indexJoint)->GetJointLimits();
    ostringstream transform1, transform2;
    transform1<<m_newRobotModel.at(indexBase).second.at(indexJoint)->TransformToDHFrame();
    transform2<<m_newRobotModel.at(indexBase).second.at(indexJoint)->TransformToBody2();
    istringstream splitTransform1(transform1.str());
    istringstream splitTransform2(transform2.str());
    string splittedTransform[2][6]={{"","","","","",""},{"","","","","",""}};
    int j=0;
    do{
      splitTransform1>>splittedTransform[0][j];
      j++;
    }while(splitTransform1);
    j=0;
    do{
      splitTransform2>>splittedTransform[1][j];
      j++;
    }while(splitTransform2);
    if(!m_newRobotModel.at(indexBase).second.at(indexJoint)->IsRevolute()&&
          !m_newRobotModel.at(indexBase).second.at(indexJoint)->IsSpherical())
      JointNonActuatedChecked();
    else{
      if(m_newRobotModel.at(indexBase).second.at(indexJoint)->IsRevolute()){
        JointRevoluteChecked();
        m_jointLimits[0][0]->setValue(jointLimits[0].first);
        m_jointLimits[0][1]->setValue(jointLimits[0].second);
        m_jointLimits[1][0]->setValue(-1);
        m_jointLimits[1][1]->setValue(1);
      }
      else{
        JointSphericalChecked();
        for(int i=0; i<2; i++){
          m_jointLimits[i][0]->setValue(jointLimits[i].first);
          m_jointLimits[i][1]->setValue(jointLimits[i].second);
        }
      }
    }
    m_jointConnectionsLine1->insert(QString::number(m_newRobotModel.at(indexBase).second.at(indexJoint)->GetPreviousIndex()));
    m_jointConnectionsLine2->insert(QString::number(m_newRobotModel.at(indexBase).second.at(indexJoint)->GetNextIndex()));
    for(int j=0; j<2; j++){
      for(int i=0; i<3; i++)
        m_jointPos[j][i]->setValue(atof(splittedTransform[j][i].c_str()));
      for(int i=0; i<3; i++)
        m_jointPos[j][5-i]->setValue(atof(splittedTransform[j][i+3].c_str()));
    }
    m_jointAlphaLine->insert(QString::number(m_newRobotModel.at(indexBase).second.at(indexJoint)->GetAlpha()));
    m_jointALine->insert(QString::number(m_newRobotModel.at(indexBase).second.at(indexJoint)->GetA()));
    m_jointDLine->insert(QString::number(m_newRobotModel.at(indexBase).second.at(indexJoint)->GetD()));
    m_jointThetaLine->insert(QString::number(m_newRobotModel.at(indexBase).second.at(indexJoint)->GetTheta()));
    DisplayHideJointAttributes(true);
  }
}

void
EditRobotDialog::ShowBase(){
  if(m_baseList->currentItem()!=NULL){
    m_baseIsInit=false;
    DisplayHideJointAttributes(false);
    ClearJointLines();
    int index = m_baseList->row(m_baseList->currentItem());
    m_jointList->clear();
    m_bodyNumberLine->clear();
    typedef Joints::const_iterator MIT;
    int i=0;
    string jointString;
    int bodyCount=0, baseCount=0;
    typedef MultiBodyModel::Robots::const_iterator RIT;
    for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++){
      if(baseCount<index){
        for(MIT mit = m_newRobotModel.at(baseCount).second.begin(); mit !=
            m_newRobotModel.at(baseCount).second.end(); ++mit){
          bodyCount++;
        }
        bodyCount++;
        baseCount++;
      }
    }
    m_bodyNumberLine->insert(QString::number(bodyCount));
    for(MIT mit = m_newRobotModel.at(index).second.begin(); mit !=
          m_newRobotModel.at(index).second.end(); ++mit){
      i++;
      jointString =m_robotBody->GetBodies().at(bodyCount+i)->GetFilename();
      m_jointList->addItem(jointString.c_str());
    }
    m_directoryLine->clear();
    m_directoryLine->insert(m_newRobotModel.at(index).first->GetModelFilename().c_str());
    if(m_newRobotModel.at(index).first->IsBaseFixed())
      BaseFixedChecked();
    else if(m_newRobotModel.at(index).first->IsBaseVolumetric()||
            m_newRobotModel.at(index).first->IsBasePlanar()){
      if(m_newRobotModel.at(index).first->IsBaseVolumetric())
        BaseVolumetricChecked();
      else
        BasePlanarChecked();
      if(m_newRobotModel.at(index).first->IsBaseRotational())
        BaseRotationalChecked();
      else
        BaseTranslationalChecked();
    }
    DisplayHideBaseAttributes(true);
  }
}

void
EditRobotDialog::SaveBase(){
  if(m_baseList->currentItem()!=NULL){
    int index = m_baseList->row(m_baseList->currentItem());
    ostringstream properties;
    if(!m_baseFixedCheck->isChecked() && !m_basePlanarCheck->isChecked()
        && !m_baseVolumetricCheck->isChecked()){
      OpenMessageBox("No base type checked");
      return;
    }
    else if (m_basePlanarCheck->isChecked()||m_baseVolumetricCheck->isChecked()){
      if(!m_baseTranslationalCheck->isChecked()&&!m_baseRotationalCheck->isChecked()){
        OpenMessageBox("No base movement checked");
        return;
      }
      if(m_basePlanarCheck->isChecked())
        properties<<m_newRobotModel.at(index).first->GetFilename()<<"  "<<"PLANAR ";
      else
        properties<<m_newRobotModel.at(index).first->GetFilename()<<"  "<<"VOLUMETRIC ";
      if(m_baseTranslationalCheck->isChecked())
        properties<<"TRANSLATIONAL \n";
      else
        properties<<"ROTATIONAL \n";
    }
    else{
      properties<<m_newRobotModel.at(index).first->GetFilename()<<"  "<<"FIXED ";
      for(int j=0; j<6; j++)
          properties<<"0 ";
      properties<<endl;
    }
    istringstream streamProperties(properties.str());
    stringstream modelFilename(m_newRobotModel.at(index).first->GetModelFilename());
    vector<string> splitDirectory;
    string modelDirectory="";
    string word;
    while(getline(modelFilename, word, '/')){
      splitDirectory.push_back(word);
      splitDirectory.push_back("/");
    }
    for(int i=0; i<3; i++)
      splitDirectory.pop_back();
    reverse(splitDirectory.begin(),splitDirectory.end());
    while(!splitDirectory.empty()){
      modelDirectory+=splitDirectory.back();
      splitDirectory.pop_back();
    }
    m_newRobotModel.at(index).first->ParseActiveBody(streamProperties,
        modelDirectory, DEFAULT_COLOR);
    int bodyCount = 0, baseCount = 0;
    typedef Joints::const_iterator MIT;
    typedef MultiBodyModel::Robots::const_iterator RIT;
    for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++){
      if(baseCount<index){
        for(MIT mit = m_newRobotModel.at(baseCount).second.begin(); mit !=
            m_newRobotModel.at(baseCount).second.end(); ++mit){
          bodyCount++;
        }
        bodyCount++;
        baseCount++;
      }
    }
    m_robotBody->DeleteBody(bodyCount);
    m_robotBody->AddBody(m_newRobotModel.at(index).first, bodyCount);
    RefreshVizmo();
  }
  else{
    OpenMessageBox("No base selected");
    return;
  }
}

void
EditRobotDialog::TypeUpdateBase(int _state){
  if(_state==2)
    UpdateBase();
}

void
EditRobotDialog::UpdateBase(){
  if(m_baseIsInit)
    SaveBase();
}

void
EditRobotDialog::TypeUpdateJoint(int _state){
  if(_state==2)
    UpdateJoint();
}

void
EditRobotDialog::UpdateJoint(){
  if(m_jointIsInit)
    SaveJoint();
}

void
EditRobotDialog::SaveJoint(){
  int indexBase = m_baseList->row(m_baseList->currentItem());
  int indexJoint = m_jointList->row(m_jointList->currentItem());
  int previousBodyIndex=m_newRobotModel.at(indexBase).second.at(indexJoint)->GetPreviousIndex();
  int bodyCount=0;
  stringstream properties;
  if(!m_jointRevoluteCheck->isChecked() && !m_jointSphericalCheck->isChecked()
      && !m_jointNonActuatedCheck->isChecked()){
    OpenMessageBox("No joint type selected");
    return;
  }
  if(!JointParamChecked()){
    OpenMessageBox("Parameters are missing");
    return;
  }
  else{
    typedef Joints::const_iterator MIT;
    typedef MultiBodyModel::Robots::const_iterator RIT;
    for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++){
      for(MIT mit = rit->second.begin(); mit != rit->second.end(); ++mit){
        bodyCount++;
      }
      bodyCount++;
    }
    if(m_jointConnectionsLine1->text()==""||m_jointConnectionsLine2->text()==""||
        m_jointAlphaLine->text()==""||m_jointALine->text()==""||
        m_jointDLine->text()==""||m_jointThetaLine->text()==""){
      OpenMessageBox("Parameters are missing");
      return;
    }
    else if((atoi(m_jointConnectionsLine1->text().toStdString().c_str())>=bodyCount)||
        (atoi(m_jointConnectionsLine1->text().toStdString().c_str())<0)||
        (atoi(m_jointConnectionsLine1->text().toStdString().c_str())==atoi(
        m_bodyNumberLine->text().toStdString().c_str()))){
      OpenMessageBox("The body number entered is wrong");
      return;
    }
    else{
      properties<<m_jointConnectionsLine1->text().toStdString()<<" "
                <<m_jointConnectionsLine2->text().toStdString()<<"  ";
      if(m_jointSphericalCheck->isChecked()){
        properties<<"Spherical "<<m_jointLimits[0][0]->text().toStdString()
                  <<":"<<m_jointLimits[0][1]->text().toStdString()<<" "
                  <<m_jointLimits[1][0]->text().toStdString()
                  <<":"<<m_jointLimits[1][1]->text().toStdString()<<endl;
      }
      else if(m_jointRevoluteCheck->isChecked()) {
        properties<<"Revolute "<<m_jointLimits[0][0]->text().toStdString()
                  <<":"<<m_jointLimits[0][1]->text().toStdString()<<endl;
      }
      else
        properties<<"NonActuated"<<endl;
      for(int i=0; i<6; i++)
        properties<<m_jointPos[0][i]->text().toStdString()<<" ";
      properties<<m_jointAlphaLine->text().toStdString()<<" "
                <<m_jointALine->text().toStdString()<<" "
                <<m_jointDLine->text().toStdString()<<" "
                <<m_jointThetaLine->text().toStdString()<<"      ";
      for(int i=0; i<6; i++)
        properties<<m_jointPos[1][i]->text().toStdString()<<" ";
      properties<<endl;
    }
  }
  if(previousBodyIndex!=(atoi(m_jointConnectionsLine1->text().toStdString().c_str()))){
    m_robotBody->GetBodies().at(previousBodyIndex)->DeleteConnection(m_newRobotModel.at(indexBase).second.at(indexJoint));
    properties>>*m_newRobotModel.at(indexBase).second.at(indexJoint);
    m_robotBody->GetBodies().at(atoi(m_jointConnectionsLine1->text().toStdString().c_str()))->AddConnection(m_newRobotModel.at(indexBase).second.at(indexJoint));
  }
  else
    properties>>*m_newRobotModel.at(indexBase).second.at(indexJoint);
  RefreshVizmo();
}

bool
EditRobotDialog::JointParamChecked(){
  if(m_jointAlphaLine->text()==""||m_jointALine->text()==""||
      m_jointDLine->text()==""||m_jointThetaLine->text()=="")
    return false;
  else
    return true;
}

void
EditRobotDialog::BaseFixedChecked(){
  m_baseFixedCheck->setChecked(true);
  m_basePlanarCheck->setChecked(false);
  m_baseVolumetricCheck->setChecked(false);
  m_baseTranslationalCheck->setChecked(false);
  m_baseTranslationalCheck->setEnabled(false);
  m_baseRotationalCheck->setChecked(false);
  m_baseRotationalCheck->setEnabled(false);
  TypeUpdateBase(2);
}

void
EditRobotDialog::BasePlanarChecked(){
  BaseNotFixedChecked();
  m_basePlanarCheck->setChecked(true);
  m_baseVolumetricCheck->setChecked(false);
}

void
EditRobotDialog::BaseVolumetricChecked(){
  BaseNotFixedChecked();
  m_basePlanarCheck->setChecked(false);
  m_baseVolumetricCheck->setChecked(true);
}

void
EditRobotDialog::BaseNotFixedChecked(){
  m_baseFixedCheck->setChecked(false);
  m_baseTranslationalCheck->setChecked(false);
  m_baseTranslationalCheck->setEnabled(false);
  m_baseRotationalCheck->setChecked(false);
  m_baseRotationalCheck->setEnabled(true);
  m_baseTranslationalCheck->setChecked(false);
  m_baseTranslationalCheck->setEnabled(true);
}

void
EditRobotDialog::BaseTranslationalChecked(){
  m_baseTranslationalCheck->setChecked(true);
  m_baseRotationalCheck->setChecked(false);
  TypeUpdateBase(2);
}

void
EditRobotDialog::BaseRotationalChecked(){
  m_baseTranslationalCheck->setChecked(false);
  m_baseRotationalCheck->setChecked(true);
  TypeUpdateBase(2);
}

void
EditRobotDialog::AddBase(){
  QString fn = QFileDialog::getOpenFileName(this, "Choose a base to load",
      QString::null, "Files (*.g *.obj)");
  if (!fn.isEmpty()){
    string newBaseFilename = fn.toStdString();
    stringstream modelFilename(newBaseFilename);
    vector<string> splitDirectory;
    string modelDirectory="";
    string word;
    string newBaseName="";
    while(getline(modelFilename, word, '/')){
      splitDirectory.push_back(word);
      newBaseName=word;
      splitDirectory.push_back("/");
    }
    for(int i=0; i<3; i++)
      splitDirectory.pop_back();
    reverse(splitDirectory.begin(),splitDirectory.end());
    while(!splitDirectory.empty()){
      modelDirectory+=splitDirectory.back();
      splitDirectory.pop_back();
    }
    BodyModel* newBase= new BodyModel();
    ostringstream properties;
    properties << newBaseName<<DEFAULT_BASE_CONF;
    istringstream streamProperties(properties.str());
    newBase->ParseActiveBody(streamProperties,modelDirectory, DEFAULT_COLOR);
    typedef Joints::const_iterator MIT;
    int bodyCount=0, baseCount=0;
    typedef MultiBodyModel::Robots::const_iterator RIT;
    for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++){
      for(MIT mit = m_newRobotModel.at(baseCount).second.begin(); mit !=
            m_newRobotModel.at(baseCount).second.end(); ++mit){
          bodyCount++;
      }
      bodyCount++;
      baseCount++;
    }
    m_robotBody->AddBody(newBase, bodyCount);
    m_robotBody->AddBase(newBase);
    m_newRobotModel=m_robotBody->GetRobots();
    m_baseList->clear();
    DisplayBases();
    RefreshVizmo();
  }
}

void
EditRobotDialog::AddJoint(){
  if(m_baseList->currentItem()!=NULL){
    QString fn = QFileDialog::getOpenFileName(this, "Choose a joint to load",
        QString::null, "Files (*.g *.obj)");
    if (!fn.isEmpty()){
      string newJointFilename = fn.toStdString();
      stringstream modelFilename(newJointFilename);
      vector<string> splitDirectory;
      string modelDirectory="";
      string word;
      string newJointName="";
      while(getline(modelFilename, word, '/')){
        splitDirectory.push_back(word);
        newJointName=word;
        splitDirectory.push_back("/");
      }
      for(int i=0; i<3; i++)
        splitDirectory.pop_back();
      reverse(splitDirectory.begin(),splitDirectory.end());
      while(!splitDirectory.empty()){
        modelDirectory+=splitDirectory.back();
        splitDirectory.pop_back();
      }
      BodyModel* newJoint= new BodyModel();
      ostringstream properties;
      properties << newJointName<<" joint \n";
      istringstream streamProperties(properties.str());
      newJoint->ParseActiveBody(streamProperties,modelDirectory, DEFAULT_COLOR);
      int indexBase = m_baseList->row(m_baseList->currentItem());
      typedef Joints::const_iterator MIT;
      int bodyCount=0, baseCount=0, jointCount=0, currentBaseNum=0;
      typedef MultiBodyModel::Robots::const_iterator RIT;
      for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++){
        if(baseCount<indexBase){
          for(MIT mit = m_newRobotModel.at(baseCount).second.begin(); mit !=
              m_newRobotModel.at(baseCount).second.end(); ++mit){
              bodyCount++;
          }
          bodyCount++;
        }
        else if(baseCount==indexBase){
          for(MIT mit = m_newRobotModel.at(baseCount).second.begin(); mit !=
              m_newRobotModel.at(baseCount).second.end(); ++mit){
            bodyCount++;
            jointCount++;
          }
          bodyCount++;
          currentBaseNum=baseCount;
        }
        else{
          for(MIT mit = m_newRobotModel.at(baseCount).second.begin(); mit !=
              m_newRobotModel.at(baseCount).second.end(); ++mit)
            (*mit)->ChangeIndex(1);
        }
        baseCount++;
      }
      m_robotBody->AddBody(newJoint, bodyCount);
      ConnectionModel* c= new ConnectionModel();
      stringstream defProperties;
      defProperties<<DEFAULT_JOINT_CONF;
      defProperties>>*c;
      c->ChangeIndex(bodyCount-1);
      c->SetGlobalIndex(bodyCount-indexBase-1);
      m_robotBody->GetBodies().at(bodyCount-1)->AddConnection(c);
      m_newRobotModel.at(currentBaseNum).second.push_back(c);
      m_robotBody->AddJoint(c, currentBaseNum, jointCount, bodyCount);
      m_newRobotModel=m_robotBody->GetRobots();
      m_jointList->clear();
      ShowBase();
      RefreshVizmo();
    }
  }
  else
    OpenMessageBox("No base selected");
}

void
EditRobotDialog::DeleteJoint(){
  if(m_jointList->currentItem()!=NULL){
    int indexBase = m_baseList->row(m_baseList->currentItem());
    int indexJoint = m_jointList->row(m_jointList->currentItem());
    int bodyNumber = atoi(m_bodyNumberLine->text().toStdString().c_str());
    int bodyCount = 0, baseCount = 0;
    typedef Joints::const_iterator MIT;
    typedef MultiBodyModel::Robots::const_iterator RIT;
    for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++){
      if(baseCount==indexBase){
        for(MIT mit = m_newRobotModel.at(baseCount).second.begin(); mit !=
            m_newRobotModel.at(baseCount).second.end(); ++mit){
          if(bodyNumber==bodyCount){
            m_robotBody->GetBodies().at((*mit)->GetPreviousIndex())->DeleteConnection(*mit);
            (*mit)->ChangeIndex(-1);
          }
          else if(bodyNumber<bodyCount)
            (*mit)->ChangeIndex(-1);
          bodyCount++;
        }
      }
      else if(baseCount>indexBase){
        for(MIT mit = m_newRobotModel.at(baseCount).second.begin(); mit !=
            m_newRobotModel.at(baseCount).second.end(); ++mit){
          (*mit)->ChangeIndex(-1);
          bodyCount++;
        }
      }
      bodyCount++;
      baseCount++;
    }
    m_robotBody->DeleteJoint(indexBase, indexJoint, bodyNumber);
    m_robotBody->DeleteBody(bodyNumber);
    delete m_jointList->item(indexJoint);
    m_newRobotModel=m_robotBody->GetRobots();
    DisplayHideJointAttributes(false);
    RefreshVizmo();
  }
  else
    OpenMessageBox("No joint selected");
}

void
EditRobotDialog::DeleteBase(){
  if(m_baseList->currentItem()!=NULL){
    int index = m_baseList->row(m_baseList->currentItem());
    int bodyCount = 0, baseCount = 0, jointCount=0;
    typedef Joints::const_iterator MIT;
    typedef MultiBodyModel::Robots::const_iterator RIT;
    for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++){
      if(baseCount<index){
        for(MIT mit = m_newRobotModel.at(baseCount).second.begin(); mit !=
            m_newRobotModel.at(baseCount).second.end(); ++mit){
          bodyCount++;
          jointCount++;
        }
        bodyCount++;
        baseCount++;
      }
      else if (baseCount==index)
        baseCount++;
      else{
        for(MIT mit = m_newRobotModel.at(baseCount).second.begin(); mit !=
            m_newRobotModel.at(baseCount).second.end(); ++mit)
          (*mit)->ChangeIndex(-(m_jointList->count()+1));
        baseCount++;
      }
    }
    for(int i=0; i<m_jointList->count(); i++){
      m_robotBody->DeleteJoint(index,0, bodyCount+1);
      m_robotBody->DeleteBody(bodyCount+1);
    }
    m_robotBody->DeleteRobot(index);
    m_robotBody->DeleteBody(bodyCount);
    m_newRobotModel=m_robotBody->GetRobots();
    m_baseList->clear();
    m_jointList->clear();
    DisplayBases();
    DisplayHideBaseAttributes(false);
    RefreshVizmo();
  }
  else
    OpenMessageBox("No Base selected");
}

void
EditRobotDialog::CreateNewRobot(){
  AddBase();
  DeleteAllExceptLastBase();
  m_jointList->clear();
  m_baseList->clear();
  DisplayHideJointAttributes(false);
  DisplayHideBaseAttributes(false);
  m_newRobotModel=m_robotBody->GetRobots();
  DisplayBases();
}

void
EditRobotDialog::DisplayBases(){
  typedef MultiBodyModel::Robots::const_iterator RIT;
  for(RIT rit = m_newRobotModel.begin(); rit<m_newRobotModel.end(); rit++){
    string baseName = rit->first->GetFilename();
    m_baseList->addItem(baseName.c_str());
  }
}

void
EditRobotDialog::DeleteAllExceptLastBase(){
  typedef Joints::const_iterator MIT;
  int baseCount=0, jointCount=0, bodyNumber=0;
  typedef MultiBodyModel::Robots::const_iterator RIT;
  for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end()-1; rit++){
    for(MIT mit = m_newRobotModel.at(baseCount).second.begin(); mit !=
          m_newRobotModel.at(baseCount).second.end(); ++mit){
      jointCount++;
      bodyNumber++;
    }
    for(int i=0; i<=jointCount; i++){
      if(i!=jointCount){
      }m_robotBody->DeleteBody(0);
    }
    m_robotBody->DeleteRobot(0);
    jointCount=0;
    bodyNumber++;
    baseCount++;
  }
  m_robotBody->DeleteJoints();
}

void
EditRobotDialog::OpenMessageBox(string _message){
  m_messageBox->setText(_message.c_str());
  m_messageBox->exec();
}

void
EditRobotDialog::SaveJointsNames(){
  typedef Joints::const_iterator JIT;
  int bodyCount=0, baseCount=0;
  typedef MultiBodyModel::Robots::const_iterator RIT;
  for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++){
    for(JIT jit = m_newRobotModel.at(baseCount).second.begin(); jit !=
          m_newRobotModel.at(baseCount).second.end(); ++jit){
      bodyCount++;
      m_oldJointsNames.push_back(m_robotBody->GetBodies().at(bodyCount)->GetFilename());
      m_oldJointsDirectories.push_back(m_robotBody->GetBodies().at(bodyCount)->GetDirectory());
    }
    bodyCount++;
    baseCount++;
  }
}

void
EditRobotDialog::ChangeDOF(){
  int dof=0;
  typedef Joints::const_iterator JIT;
  typedef MultiBodyModel::Robots::const_iterator RIT;
  for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++){
    if((*rit).first->IsBasePlanar()){
      dof+=2;
      if((*rit).first->IsBaseRotational())
        dof+=1;
    }
    else if((*rit).first->IsBaseVolumetric()){
      dof+=3;
      if((*rit).first->IsBaseRotational())
        dof+=3;
    }
    for(JIT jit = (*rit).second.begin(); jit != (*rit).second.end(); ++jit){
      if((*jit)->GetJointType() == ConnectionModel::REVOLUTE)
        dof+=1;
      else if((*jit)->GetJointType() == ConnectionModel::SPHERICAL)
        dof+=2;
    }
  }
  m_robotBody->ChangeDOF(dof);
}

void
EditRobotDialog::RefreshVizmo(){
    ChangeDOF();
    GetVizmo().PlaceRobot();
    m_mainWindow->GetGLScene()->updateGL();
    m_mainWindow->GetModelSelectionWidget()->ResetLists();
}
