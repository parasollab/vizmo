#include "EditRobotDialog.h"

#include <iostream>

#include "MainWindow.h"
#include "GLWidget.h"
#include "ModelSelectionWidget.h"
#include "Models/BodyModel.h"
#include "Models/ConnectionModel.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"
#include "Utilities/AlertUser.h"

#define DEFAULT_COLOR Color4(0.0,0.0,0.0,1)
#define DEFAULT_BASE_CONF "  FIXED  0 0 0 0 0 0 \n"
#define DEFAULT_JOINT_CONF "0 1  REVOLUTE -1:1\n0 0 0 0 0 0  0 0 0 0  0 0 0 0 0 0\n"

EditRobotDialog::EditRobotDialog(MainWindow* _mainWindow, QWidget* _parent) : QDialog(_parent) {
  setWindowTitle("Edit Robot");

  m_okButton = new QPushButton("Ok", this);

  m_directory = new QLabel(this);

  m_addBaseButton = new QPushButton("Add", this);
  m_deleteBaseButton = new QPushButton("Delete", this);
  m_addJointButton = new QPushButton("Add", this);
  m_deleteJointButton = new QPushButton("Delete", this);

  m_baseList = new QListWidget(this);
  m_jointList = new QListWidget(this);

  m_baseFixedCheck = new QCheckBox("Fixed", this);
  m_basePlanarCheck = new QCheckBox("Planar", this);
  m_baseVolumetricCheck = new QCheckBox("Volumetric", this);

  m_baseTranslationalCheck = new QCheckBox("Translational", this);
  m_baseRotationalCheck = new QCheckBox("Rotational", this);

  m_jointNonActuatedCheck = new QCheckBox("Non Actuated", this);
  m_jointRevoluteCheck = new QCheckBox("Revolute", this);
  m_jointSphericalCheck = new QCheckBox("Spherical", this);
  for(int j=0; j<2; j++){
    for(int i=0; i<2; i++)
      m_jointLimits[j][i] = new QDoubleSpinBox(this);
    for(int i=0; i<6; i++)
      m_jointPos[j][i] = new QDoubleSpinBox(this);
  }
  m_jointAlphaLine = new QLineEdit(this);
  m_jointALine = new QLineEdit(this);
  m_jointDLine = new QLineEdit(this);
  m_jointThetaLine = new QLineEdit(this);
  m_newRobotButton = new QPushButton("New robot", this);
  m_bodyNumberLine = new QLineEdit(this);
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
  connect(m_jointSphericalCheck, SIGNAL(clicked(bool)), this, SLOT(UpdateJoint(bool)));
  connect(m_jointRevoluteCheck, SIGNAL(clicked(bool)), this, SLOT(UpdateJoint(bool)));
  connect(m_jointNonActuatedCheck, SIGNAL(clicked(bool)), this, SLOT(UpdateJoint(bool)));

  vector<MultiBodyModel*> mBody = GetVizmo().GetEnv()->GetMultiBodies();
  typedef vector<MultiBodyModel*>::const_iterator MIT;
  for(MIT mit = mBody.begin(); mit<mBody.end(); mit++)
    if((*mit)->IsActive())
      m_robotBody = *mit;

  m_newRobotModel = m_robotBody->GetRobots();
  m_jointIsInit = false;
  m_baseIsInit = false;
  m_mainWindow = _mainWindow;

  SaveJointsNames();
  SetUpLayout();
  DisplayBases();
}

void
EditRobotDialog::SetUpLayout(){
  QVBoxLayout* layout = new QVBoxLayout(this);
  setLayout(layout);

  layout->addWidget(m_newRobotButton, 0, Qt::AlignCenter);
  m_newRobotButton->setFixedWidth(250);

  //set up base/joint lists section
  QHBoxLayout* baseJointLabels = new QHBoxLayout();
  baseJointLabels->addWidget(new QLabel("<b>Bases:<b>", this));
  baseJointLabels->addSpacing(50);
  baseJointLabels->addWidget(new QLabel("<b>Joints:<b>", this));
  layout->addLayout(baseJointLabels);

  QHBoxLayout* baseJointLists = new QHBoxLayout();

  QVBoxLayout* baseAddDel = new QVBoxLayout();
  baseAddDel->addStretch(1);
  baseAddDel->addWidget(m_addBaseButton);
  baseAddDel->addWidget(m_deleteBaseButton);
  baseAddDel->addStretch(1);

  QVBoxLayout* jointAddDel = new QVBoxLayout();
  jointAddDel->addStretch(1);
  jointAddDel->addWidget(m_addJointButton);
  jointAddDel->addWidget(m_deleteJointButton);
  jointAddDel->addStretch(1);

  baseJointLists->addWidget(m_baseList);
  baseJointLists->addLayout(baseAddDel);
  baseJointLists->addSpacing(50);
  baseJointLists->addWidget(m_jointList);
  baseJointLists->addLayout(jointAddDel);
  m_baseList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  m_jointList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  layout->addLayout(baseJointLists);

  //set up body num
  QHBoxLayout* bodyNum = new QHBoxLayout();
  bodyNum->addStretch(1);
  bodyNum->addWidget(new QLabel("Body num:", this));
  bodyNum->addWidget(m_bodyNumberLine);
  bodyNum->addStretch(1);
  m_bodyNumberLine->setReadOnly(true);
  layout->addLayout(bodyNum);

  //attributes
  layout->addWidget(new QLabel("<b>Attributes:<b>", this), 0, Qt::AlignLeft);

  //hidden layout for base stuff
  QGridLayout* baseAttributes = new QGridLayout();

  //directory
  baseAttributes->addWidget(new QLabel("Directory:", this), 0, 0);
  baseAttributes->addWidget(m_directory, 0, 1, 1, 3);

  //base type
  baseAttributes->addWidget(new QLabel("Type:", this), 1, 0);
  baseAttributes->addWidget(m_baseFixedCheck, 1, 1);
  baseAttributes->addWidget(m_basePlanarCheck, 1, 2);
  baseAttributes->addWidget(m_baseVolumetricCheck, 1, 3);

  //base movement
  baseAttributes->addWidget(new QLabel("Movement:", this), 2, 0);
  baseAttributes->addWidget(m_baseTranslationalCheck, 2, 1);
  baseAttributes->addWidget(m_baseRotationalCheck, 2, 2);

  m_baseWidget = new QWidget(this);
  m_baseWidget->setLayout(baseAttributes);

  //layout->addLayout(baseAttributes);
  layout->addWidget(m_baseWidget);

  //hidden layout for joint stuff
  QGridLayout* jointAttributes = new QGridLayout();

  //joint type
  jointAttributes->addWidget(new QLabel("Type:", this), 0, 0);
  jointAttributes->addWidget(m_jointNonActuatedCheck, 0, 1);
  jointAttributes->addWidget(m_jointRevoluteCheck, 0, 2);
  jointAttributes->addWidget(m_jointSphericalCheck, 0, 3);

  //joint limits
  jointAttributes->addWidget(new QLabel("Limits:", this), 1, 0);
  jointAttributes->addWidget(new QLabel("X:", this), 1, 1);
  jointAttributes->addWidget(m_jointLimits[0][0], 1, 2);
  jointAttributes->addWidget(m_jointLimits[0][1], 1, 3);
  jointAttributes->addWidget(new QLabel("Y:", this), 1, 4);
  jointAttributes->addWidget(m_jointLimits[1][0], 1, 5);
  jointAttributes->addWidget(m_jointLimits[1][1], 1, 6);
  for(size_t i = 0; i<2; ++i) {
    for(size_t j = 0; j<2; ++j) {
      m_jointLimits[i][j]->setRange(-1, 1);
      m_jointLimits[i][j]->setSingleStep(0.1);
    }
  }
  jointAttributes->addWidget(new QLabel("Connected to bodies num:", this), 1, 7);
  jointAttributes->addWidget(m_jointConnectionsLine1, 1, 8);
  jointAttributes->addWidget(m_jointConnectionsLine2, 1, 9);
  m_jointConnectionsLine2->setReadOnly(true);
  m_jointConnectionsLine1->setValidator(new QIntValidator(0, numeric_limits<size_t>::max()));

  //transform to DH Frame
  jointAttributes->addWidget(new QLabel("Transform to", this), 2, 0);
  jointAttributes->addWidget(new QLabel("X", this), 2, 1);
  jointAttributes->addWidget(new QLabel("Y", this), 2, 2);
  jointAttributes->addWidget(new QLabel("Z", this), 2, 3);
  jointAttributes->addWidget(new QLabel(QChar(0x03B1), this), 2, 4);
  jointAttributes->addWidget(new QLabel(QChar(0x03B2), this), 2, 5);
  jointAttributes->addWidget(new QLabel(QChar(0x03B3), this), 2, 6);
  jointAttributes->addWidget(new QLabel("DH Frame:", this), 3, 0);
  jointAttributes->addWidget(m_jointPos[0][0], 3, 1);
  jointAttributes->addWidget(m_jointPos[0][1], 3, 2);
  jointAttributes->addWidget(m_jointPos[0][2], 3, 3);
  jointAttributes->addWidget(m_jointPos[0][3], 3, 4);
  jointAttributes->addWidget(m_jointPos[0][4], 3, 5);
  jointAttributes->addWidget(m_jointPos[0][5], 3, 6);

  //DH parameters
  jointAttributes->addWidget(new QLabel("Denavit-Hartenberg (DH)", this), 4, 0);
  jointAttributes->addWidget(new QLabel(QChar(0x03B1),this), 4, 1);
  jointAttributes->addWidget(new QLabel("a",this), 4, 2);
  jointAttributes->addWidget(new QLabel("d",this), 4, 3);
  jointAttributes->addWidget(new QLabel(QChar(0x03B8),this), 4, 4);
  jointAttributes->addWidget(new QLabel("Parameters:", this), 5, 0);
  jointAttributes->addWidget(m_jointAlphaLine, 5, 1);
  jointAttributes->addWidget(m_jointALine, 5, 2);
  jointAttributes->addWidget(m_jointDLine, 5, 3);
  jointAttributes->addWidget(m_jointThetaLine, 5, 4);
  m_jointAlphaLine->setValidator(new QDoubleValidator());
  m_jointALine->setValidator(new QDoubleValidator());
  m_jointDLine->setValidator(new QDoubleValidator());
  m_jointThetaLine->setValidator(new QDoubleValidator());

  //transform to body2
  jointAttributes->addWidget(new QLabel("Transform to", this), 6, 0);
  jointAttributes->addWidget(new QLabel("X", this), 6, 1);
  jointAttributes->addWidget(new QLabel("Y", this), 6, 2);
  jointAttributes->addWidget(new QLabel("Z", this), 6, 3);
  jointAttributes->addWidget(new QLabel(QChar(0x03B1), this), 6, 4);
  jointAttributes->addWidget(new QLabel(QChar(0x03B2), this), 6, 5);
  jointAttributes->addWidget(new QLabel(QChar(0x03B3), this), 6, 6);
  jointAttributes->addWidget(new QLabel("Body 2:", this), 7, 0);
  jointAttributes->addWidget(m_jointPos[1][0], 7, 1);
  jointAttributes->addWidget(m_jointPos[1][1], 7, 2);
  jointAttributes->addWidget(m_jointPos[1][2], 7, 3);
  jointAttributes->addWidget(m_jointPos[1][3], 7, 4);
  jointAttributes->addWidget(m_jointPos[1][4], 7, 5);
  jointAttributes->addWidget(m_jointPos[1][5], 7, 6);

  m_jointWidget = new QWidget(this);
  m_jointWidget->setLayout(jointAttributes);

  //layout->addLayout(jointAttributes);
  layout->addWidget(m_jointWidget);

  //cancel button
  layout->addWidget(m_okButton, 0, Qt::AlignRight);

  DisplayHideJointAttributes(false);
  DisplayHideBaseAttributes(false);

  QDialog::show();
}

void
EditRobotDialog::DisplayHideJointAttributes(bool _visible){
  m_jointIsInit=_visible;
  m_jointWidget->setVisible(_visible);
}

void
EditRobotDialog::JointNonActuatedChecked(){
  m_jointSphericalCheck->setChecked(false);
  m_jointRevoluteCheck->setChecked(false);
  m_jointNonActuatedCheck->setChecked(true);
  m_jointLimits[0][0]->setEnabled(false);
  m_jointLimits[0][1]->setEnabled(false);
  m_jointLimits[1][0]->setEnabled(false);
  m_jointLimits[1][1]->setEnabled(false);
}

void
EditRobotDialog::JointSphericalChecked(){
  m_jointSphericalCheck->setChecked(true);
  m_jointRevoluteCheck->setChecked(false);
  m_jointNonActuatedCheck->setChecked(false);
  m_jointLimits[0][0]->setEnabled(true);
  m_jointLimits[0][1]->setEnabled(true);
  m_jointLimits[1][0]->setEnabled(true);
  m_jointLimits[1][1]->setEnabled(true);
}

void
EditRobotDialog::JointRevoluteChecked(){
  m_jointSphericalCheck->setChecked(false);
  m_jointRevoluteCheck->setChecked(true);
  m_jointNonActuatedCheck->setChecked(false);
  m_jointLimits[0][0]->setEnabled(true);
  m_jointLimits[0][1]->setEnabled(true);
  m_jointLimits[1][0]->setEnabled(false);
  m_jointLimits[1][1]->setEnabled(false);
}

void
EditRobotDialog::DisplayHideBaseAttributes(bool _visible){
  m_baseIsInit=_visible;
  m_baseWidget->setVisible(_visible);
}

void
EditRobotDialog::Accept(){
  RefreshVizmo();
  accept();
}

void
EditRobotDialog::ShowJoint(){
  if(m_baseList->currentItem()) {
    m_jointIsInit = false;
    DisplayHideBaseAttributes(false);

    int indexBase = m_baseList->row(m_baseList->currentItem());
    int indexJoint = m_jointList->row(m_jointList->currentItem());
    typedef Joints::const_iterator MIT;

    int bodyCount = 0;
    typedef MultiBodyModel::Robots::const_iterator RIT;
    for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.begin() + indexBase; rit++)
      bodyCount += rit->second.size() + 1;
    bodyCount += indexJoint;

    m_bodyNumberLine->setText(QString::number(bodyCount+1));
    ConnectionModel* conn = m_newRobotModel[indexBase].second[indexJoint];
    pair<double, double> jointLimits[2] = conn->GetJointLimits();
    ostringstream transform1, transform2;
    transform1 << conn->TransformToDHFrame();
    transform2 << conn->TransformToBody2();
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
    if(conn->IsNonActuated())
      JointNonActuatedChecked();
    else{
      if(conn->IsRevolute()) {
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
    m_jointConnectionsLine1->setText(QString::number(conn->GetPreviousIndex()));
    m_jointConnectionsLine2->setText(QString::number(conn->GetNextIndex()));
    for(int j=0; j<2; j++){
      for(int i=0; i<3; i++)
        m_jointPos[j][i]->setValue(atof(splittedTransform[j][i].c_str()));
      for(int i=0; i<3; i++)
        m_jointPos[j][5-i]->setValue(atof(splittedTransform[j][i+3].c_str()));
    }
    m_jointAlphaLine->setText(QString::number(conn->GetAlpha()));
    m_jointALine->setText(QString::number(conn->GetA()));
    m_jointDLine->setText(QString::number(conn->GetD()));
    m_jointThetaLine->setText(QString::number(conn->GetTheta()));

    DisplayHideJointAttributes(true);
  }
}

void
EditRobotDialog::ShowBase(){
  if(m_baseList->currentItem()) {
    m_baseIsInit = false;
    DisplayHideJointAttributes(false);

    int index = m_baseList->row(m_baseList->currentItem());

    int bodyCount = 0;
    typedef MultiBodyModel::Robots::const_iterator RIT;
    for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.begin() + index; rit++)
      bodyCount += rit->second.size() + 1;

    m_bodyNumberLine->setText(QString::number(bodyCount));

    m_jointList->clear();
    int i=0;
    typedef Joints::const_iterator MIT;
    for(MIT mit = m_newRobotModel[index].second.begin();
        mit != m_newRobotModel[index].second.end(); ++mit) {
      i++;
      m_jointList->addItem(m_robotBody->GetBodies()[bodyCount+i]->GetFilename().c_str());
    }

    BodyModel* body = m_newRobotModel[index].first;
    m_directory->setText(body->GetModelFilename().c_str());

    if(body->IsBaseFixed())
      BaseFixedChecked();
    else {
      if(body->IsBaseVolumetric())
        BaseVolumetricChecked();
      else
        BasePlanarChecked();

      if(body->IsBaseRotational())
        BaseRotationalChecked();
      else
        BaseTranslationalChecked();
    }

    DisplayHideBaseAttributes(true);
  }
}

void
EditRobotDialog::SaveBase() {
  if(m_baseList->currentItem()) {
    int index = m_baseList->row(m_baseList->currentItem());
    ostringstream properties;
    BodyModel* body = m_newRobotModel[index].first;
    properties << body->GetFilename() << " ";
    if(!m_baseFixedCheck->isChecked()) {
      if(m_basePlanarCheck->isChecked())
        properties << "PLANAR ";
      else
        properties << "VOLUMETRIC ";

      if(m_baseTranslationalCheck->isChecked())
        properties << "TRANSLATIONAL";
      else
        properties << "ROTATIONAL";
      properties << endl;
    }
    else
      properties << "FIXED 0 0 0 0 0 0" << endl;

    istringstream streamProperties(properties.str());
    string modelFilename = body->GetModelFilename();
    size_t s = modelFilename.rfind('/');
    string modelDirectory = s == string::npos ? "" : modelFilename.substr(0, s);
    body->ParseActiveBody(streamProperties, modelDirectory, DEFAULT_COLOR);
    int bodyCount = 0;
    typedef Joints::const_iterator MIT;
    typedef MultiBodyModel::Robots::const_iterator RIT;
    for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.begin() + index; ++rit)
      bodyCount += rit->second.size() + 1;
    m_robotBody->DeleteBody(bodyCount);
    m_robotBody->AddBody(body, bodyCount);
    RefreshVizmo();
  }
  else{
    AlertUser("No base selected");
    return;
  }
}

void
EditRobotDialog::UpdateBase(){
  if(m_baseIsInit)
    SaveBase();
}

void
EditRobotDialog::UpdateJoint(bool _clicked){
  if(_clicked && m_jointIsInit)
    SaveJoint();
}

void
EditRobotDialog::SaveJoint(){
  int indexBase = m_baseList->row(m_baseList->currentItem());
  int indexJoint = m_jointList->row(m_jointList->currentItem());
  int previousBodyIndex = m_newRobotModel[indexBase].second[indexJoint]->GetPreviousIndex();
  int bodyCount = 0;

  stringstream properties;
  typedef Joints::const_iterator MIT;
  typedef MultiBodyModel::Robots::const_iterator RIT;

  for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++)
    bodyCount += rit->second.size() + 1;

  int bodyNum = m_jointConnectionsLine1->text().toInt();
  int bodyNum2 = m_jointConnectionsLine2->text().toInt();
  if(bodyNum >= bodyCount || bodyNum < 0 || bodyNum == bodyNum2){
    AlertUser("Invalid body number specified");
    return;
  }
  else {
    properties << bodyNum << " " << bodyNum2 << " ";
    double jointLim0min = m_jointLimits[0][0]->value(), jointLim0max = m_jointLimits[0][1]->value(),
           jointLim1min = m_jointLimits[1][0]->value(), jointLim1max = m_jointLimits[1][1]->value();

    if(m_jointSphericalCheck->isChecked())
      properties << "Spherical "
        << jointLim0min << ":" << jointLim0max
        << " " << jointLim1min << ":" << jointLim1max << endl;
    else if(m_jointRevoluteCheck->isChecked())
      properties << "Revolute "
        << jointLim0min << ":" << jointLim0max << endl;
    else
      properties << "NonActuated" << endl;

    for(int i=0; i<6; i++)
      properties << m_jointPos[0][i]->value() << " ";

    properties << m_jointAlphaLine->text().toStdString() << " "
      << m_jointALine->text().toStdString() << " "
      << m_jointDLine->text().toStdString() << " "
      << m_jointThetaLine->text().toStdString() << " ";

    for(int i=0; i<6; i++)
      properties << m_jointPos[1][i]->value() << " ";

    properties<<endl;
  }

  if(previousBodyIndex != bodyNum) {
    m_robotBody->GetBodies()[previousBodyIndex]->DeleteConnection(m_newRobotModel[indexBase].second[indexJoint]);
    properties >> *m_newRobotModel[indexBase].second[indexJoint];
    m_robotBody->GetBodies()[bodyNum]->AddConnection(m_newRobotModel[indexBase].second[indexJoint]);
  }
  else
    properties >> *m_newRobotModel[indexBase].second[indexJoint];

  RefreshVizmo();
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
  UpdateBase();
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
  UpdateBase();
}

void
EditRobotDialog::BaseRotationalChecked(){
  m_baseTranslationalCheck->setChecked(false);
  m_baseRotationalCheck->setChecked(true);
  UpdateBase();
}

void
EditRobotDialog::AddBase(){
  QString fn = QFileDialog::getOpenFileName(this, "Choose a base to load",
      QString::null, "Files (*.g *.obj)");
  if(!fn.isEmpty()) {
    string newBaseFilename = fn.toStdString();
    size_t s = newBaseFilename.rfind('/');
    string modelDirectory = s == string::npos ? "" : newBaseFilename.substr(0, s);
    string newBaseName = s == string::npos ? newBaseFilename : newBaseFilename.substr(s+1);

    BodyModel* newBase= new BodyModel();

    ostringstream properties;
    properties << newBaseName<<DEFAULT_BASE_CONF;

    istringstream streamProperties(properties.str());
    newBase->ParseActiveBody(streamProperties, modelDirectory, DEFAULT_COLOR);

    typedef Joints::const_iterator MIT;
    int bodyCount = 0;
    typedef MultiBodyModel::Robots::const_iterator RIT;
    for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++)
      bodyCount += rit->second.size() + 1;
    m_robotBody->AddBody(newBase, bodyCount);
    m_robotBody->AddBase(newBase);
    m_newRobotModel=m_robotBody->GetRobots();
    m_baseList->clear();
    DisplayBases();
    RefreshVizmo();
  }
}

void
EditRobotDialog::AddJoint() {
  if(m_baseList->currentItem()) {
    QString fn = QFileDialog::getOpenFileName(this, "Choose a joint to load",
        QString::null, "Files (*.g *.obj)");
    if(!fn.isEmpty()) {
      string newJointFilename = fn.toStdString();
      size_t s = newJointFilename.rfind('/');
      string modelDirectory = s == string::npos ? "" : newJointFilename.substr(0, s);
      string newJointName = s == string::npos ? newJointFilename : newJointFilename.substr(s+1);

      BodyModel* newJoint= new BodyModel();
      ostringstream properties;
      properties << newJointName << " Joint";

      istringstream streamProperties(properties.str());
      newJoint->ParseActiveBody(streamProperties, modelDirectory, DEFAULT_COLOR);

      int indexBase = m_baseList->row(m_baseList->currentItem());
      typedef Joints::const_iterator MIT;
      int bodyCount = 0, baseCount = 0, jointCount = 0;
      typedef MultiBodyModel::Robots::const_iterator RIT;
      for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); ++rit){
        if(baseCount <= indexBase) {
          bodyCount += rit->second.size() + 1;
          jointCount = rit->second.size();
        }
        else
          for(MIT mit = rit->second.begin(); mit != rit->second.end(); ++mit)
            (*mit)->ChangeIndex(1);
        baseCount++;
      }

      m_robotBody->AddBody(newJoint, bodyCount);
      ConnectionModel* c = new ConnectionModel();
      istringstream defProperties(DEFAULT_JOINT_CONF);
      defProperties >> *c;
      c->ChangeIndex(bodyCount-1);
      c->SetGlobalIndex(bodyCount-indexBase-1);
      m_robotBody->GetBodies()[bodyCount-1]->AddConnection(c);
      m_newRobotModel[indexBase].second.push_back(c);
      m_robotBody->AddJoint(c, indexBase, jointCount, bodyCount);
      m_newRobotModel = m_robotBody->GetRobots();
      m_jointList->clear();
      ShowBase();
      RefreshVizmo();
    }
  }
  else
    AlertUser("No base selected");
}

void
EditRobotDialog::DeleteJoint(){
  if(m_jointList->currentItem()) {
    int indexBase = m_baseList->row(m_baseList->currentItem());
    int indexJoint = m_jointList->row(m_jointList->currentItem());
    int bodyNumber = m_bodyNumberLine->text().toInt();
    int bodyCount = 0, baseCount = 0;
    typedef Joints::const_iterator MIT;
    typedef MultiBodyModel::Robots::const_iterator RIT;
    for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); ++rit) {
      if(baseCount == indexBase) {
        for(MIT mit = rit->second.begin(); mit != rit->second.end(); ++mit) {
          if(bodyNumber == bodyCount){
            m_robotBody->GetBodies()[(*mit)->GetPreviousIndex()]->DeleteConnection(*mit);
            (*mit)->ChangeIndex(-1);
          }
          else if(bodyNumber < bodyCount)
            (*mit)->ChangeIndex(-1);
          bodyCount++;
        }
      }
      else if(baseCount > indexBase) {
        for(MIT mit = rit->second.begin(); mit != rit->second.end(); ++mit){
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
    m_newRobotModel = m_robotBody->GetRobots();
    DisplayHideJointAttributes(false);
    RefreshVizmo();
  }
  else
    AlertUser("No joint selected");
}

void
EditRobotDialog::DeleteBase() {
  if(m_baseList->currentItem()) {
    int index = m_baseList->row(m_baseList->currentItem());
    size_t bodyCount = 0;
    typedef Joints::const_iterator MIT;
    typedef MultiBodyModel::Robots::const_iterator RIT;
    RIT beginning = m_newRobotModel.begin();
    for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++) {
      int dist = distance(beginning, rit);
      if(dist < index)
        bodyCount += rit->second.size() + 1;
      else if(dist > index)
        for(MIT mit = rit->second.begin(); mit != rit->second.end(); ++mit)
          (*mit)->ChangeIndex(-(m_jointList->count()+1));
    }
    for(int i = 0; i < m_jointList->count(); ++i) {
      m_robotBody->DeleteJoint(index, 0, bodyCount+1);
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
    AlertUser("No Base selected");
}

void
EditRobotDialog::CreateNewRobot(){
  AddBase();
  DeleteAllExceptLastBase();
  m_jointList->clear();
  m_baseList->clear();
  DisplayHideJointAttributes(false);
  DisplayHideBaseAttributes(false);
  m_newRobotModel = m_robotBody->GetRobots();
  DisplayBases();
}

void
EditRobotDialog::DisplayBases() {
  typedef MultiBodyModel::Robots::const_iterator RIT;
  for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); ++rit)
    m_baseList->addItem(rit->first->GetFilename().c_str());
}

void
EditRobotDialog::DeleteAllExceptLastBase() {
  typedef Joints::const_iterator MIT;
  typedef MultiBodyModel::Robots::const_iterator RIT;
  for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end()-1; ++rit) {
    for(size_t i = 0; i < rit->second.size(); ++i)
      m_robotBody->DeleteBody(0);
    m_robotBody->DeleteRobot(0);
  }
  m_robotBody->DeleteJoints();
}

void
EditRobotDialog::SaveJointsNames(){
  typedef Joints::const_iterator JIT;
  size_t bodyCount = 0;
  typedef MultiBodyModel::Robots::const_iterator RIT;
  vector<BodyModel*>& bodies = m_robotBody->GetBodies();
  for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); ++rit) {
    for(JIT jit = rit->second.begin(); jit != rit->second.end(); ++jit) {
      bodyCount++;
      m_oldJointsNames.push_back(bodies[bodyCount]->GetFilename());
      m_oldJointsDirectories.push_back(bodies[bodyCount]->GetDirectory());
    }
    bodyCount++;
  }
}

void
EditRobotDialog::ChangeDOF(){
  size_t dof = 0;
  typedef Joints::const_iterator JIT;
  typedef MultiBodyModel::Robots::const_iterator RIT;
  for(RIT rit = m_newRobotModel.begin(); rit != m_newRobotModel.end(); rit++){
    if(rit->first->IsBasePlanar()){
      dof+=2;
      if(rit->first->IsBaseRotational())
        dof+=1;
    }
    else if(rit->first->IsBaseVolumetric()){
      dof+=3;
      if(rit->first->IsBaseRotational())
        dof+=3;
    }
    for(JIT jit = rit->second.begin(); jit != rit->second.end(); ++jit){
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
