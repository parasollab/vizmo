#include "EditRobotDialog.h"

#include <iostream>
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
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
#include "Models/Vizmo.h"

#define DEFAULT_COLOR Color4(0.0,0.0,0.0,1)
#define DEFAULT_BASE_CONF "  FIXED  0 0 0 0 0 0 \n"
#define DEFAULT_JOINT_CONF "0 1  REVOLUTE -1:1\n0 0 0 0 0 0  0 0 0 0  0 0 0 0 0 0\n"

EditRobotDialog::EditRobotDialog(MainWindow* _mainWindow, QWidget* _parent) : QDialog(_parent) {
  resize(709, 540);
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
  m_basePositionLabel = new QLabel("Position:", this);
  m_baseRotationLabel = new QLabel("Rotation:", this);
  m_baseXLine = new QLineEdit(this);
  m_baseXLabel = new QLabel("X", this);
  m_baseYLine = new QLineEdit(this);
  m_baseYLabel = new QLabel("Y", this);
  m_baseZLine = new QLineEdit(this);
  m_baseZLabel = new QLabel("Z", this);
  m_baseXRotLine = new QLineEdit(this);
  m_baseXRotLabel = new QLabel("X", this);
  m_baseYRotLine = new QLineEdit(this);
  m_baseYRotLabel = new QLabel("Y", this);
  m_baseZRotLine = new QLineEdit(this);
  m_baseZRotLabel = new QLabel("Z", this);
  m_saveAttributesButton = new QPushButton("Save Attributes", this);
  m_restaureAttributesButton = new QPushButton("Restaure Attributes", this);
  m_validator = new QDoubleValidator();
  m_messageBox = new QMessageBox(this);
  m_jointSaveAttributesButton = new QPushButton("Save Attributes", this);
  m_jointRestaureAttributesButton = new QPushButton("Restaure Attributes", this);
  m_jointSphericalCheck = new QCheckBox("Spherical", this);
  m_jointTypeLabel = new QLabel("type:", this);
  m_jointRevoluteCheck = new QCheckBox("Revolute", this);
  m_jointNonActuatedCheck = new QCheckBox("Non Actuated", this);
  m_jointLimitsLabel = new QLabel("Limits:", this);
  m_jointLimitsLine11 = new QLineEdit(this);
  m_jointLimitsLine12 = new QLineEdit(this);
  m_jointLimitsLine21 = new QLineEdit(this);
  m_jointLimitsLine22 = new QLineEdit(this);
  m_jointLimitLabel1 = new QLabel("X:", this);
  m_jointLimitLabel2 = new QLabel("Y:", this);
  m_jointPositionLabel11 = new QLabel("Start position from", this);
  m_jointPositionLabel12 = new QLabel("last connection:", this);
  m_jointX1Line = new QLineEdit(this);
  m_jointY1Line = new QLineEdit(this);
  m_jointZ1Line = new QLineEdit(this);
  m_jointXRot1Line = new QLineEdit(this);
  m_jointYRot1Line = new QLineEdit(this);
  m_jointZRot1Line = new QLineEdit(this);
  m_jointX1Label = new QLabel("X", this);
  m_jointY1Label = new QLabel("Y", this);
  m_jointZ1Label = new QLabel("Z", this);
  m_jointXRot1Label = new QLabel("XRot", this);
  m_jointYRot1Label = new QLabel("YRot", this);
  m_jointZRot1Label = new QLabel("ZRot", this);
  m_jointPositionLabel21 = new QLabel("End position from", this);
  m_jointPositionLabel22 = new QLabel("last connection:", this);
  m_jointX2Line = new QLineEdit(this);
  m_jointY2Line = new QLineEdit(this);
  m_jointZ2Line = new QLineEdit(this);
  m_jointXRot2Line = new QLineEdit(this);
  m_jointYRot2Line = new QLineEdit(this);
  m_jointZRot2Line = new QLineEdit(this);
  m_jointX2Label = new QLabel("X", this);
  m_jointY2Label = new QLabel("Y", this);
  m_jointZ2Label = new QLabel("Z", this);
  m_jointXRot2Label = new QLabel("XRot", this);
  m_jointYRot2Label = new QLabel("YRot", this);
  m_jointZRot2Label = new QLabel("ZRot", this);
  m_jointDHParametersLabel1 = new QLabel("Denavit-Hartenberg", this);
  m_jointDHParametersLabel2 = new QLabel("parameters:", this);
  m_jointAlphaLine = new QLineEdit(this);
  m_jointALine = new QLineEdit(this);
  m_jointDLine = new QLineEdit(this);
  m_jointThetaLine = new QLineEdit(this);
  m_jointAlphaLabel = new QLabel("Alpha",this);
  m_jointALabel = new QLabel("a",this);
  m_jointDLabel = new QLabel("d",this);
  m_jointThetaLabel = new QLabel("Theta",this);
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
  connect(m_restaureAttributesButton, SIGNAL(clicked()), this, SLOT(ShowBase()));
  connect(m_saveAttributesButton, SIGNAL(clicked()), this, SLOT(SaveBase()));
  connect(m_deleteBaseButton, SIGNAL(clicked()), this, SLOT(DeleteBase()));
  connect(m_addBaseButton, SIGNAL(clicked()), this, SLOT(AddBase()));
  connect(m_newRobotButton, SIGNAL(clicked()), this, SLOT(CreateNewRobot()));
  connect(m_jointNonActuatedCheck, SIGNAL(clicked()), this, SLOT(JointNonActuatedChecked()));
  connect(m_jointSphericalCheck, SIGNAL(clicked()), this, SLOT(JointSphericalChecked()));
  connect(m_jointRevoluteCheck, SIGNAL(clicked()), this, SLOT(JointRevoluteChecked()));
  connect(m_jointRestaureAttributesButton, SIGNAL(clicked()), this, SLOT(ShowJoint()));
  connect(m_deleteJointButton, SIGNAL(clicked()), this, SLOT(DeleteJoint()));
  connect(m_addJointButton, SIGNAL(clicked()), this, SLOT(AddJoint()));
  connect(m_jointSaveAttributesButton, SIGNAL(clicked()), this, SLOT(SaveJoint()));

  vector<MultiBodyModel*> mBody = GetVizmo().GetEnv()->GetMultiBodies();
  typedef vector<MultiBodyModel*>::const_iterator MIT;
  for(MIT mit = mBody.begin(); mit<mBody.end(); mit++){
    if((*mit)->IsActive())
      m_robotBody=*mit;
  }
  m_newRobotModel=m_robotBody->GetRobots();
  m_mainWindow=_mainWindow;
  SaveJointsNames();
  SetUpLayout();
  DisplayBases();
}

void
EditRobotDialog::SetUpLayout(){
  m_okButton->setGeometry(QRect(570, 490, 131, 31));
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
  m_baseMovementLabel->setGeometry(QRect(30, 310, 81, 21));
  m_basePositionLabel->setGeometry(QRect(30, 330, 61, 21));
  m_baseRotationLabel->setGeometry(QRect(30, 360, 61, 21));
  m_baseXLine->setGeometry(QRect(120, 330, 61, 21));
  m_baseXLabel->setGeometry(QRect(110, 330, 16, 21));
  m_baseYLine->setGeometry(QRect(200, 330, 61, 21));
  m_baseYLabel->setGeometry(QRect(190, 330, 16, 21));
  m_baseZLine->setGeometry(QRect(280, 330, 61, 21));
  m_baseZLabel->setGeometry(QRect(270, 330, 16, 21));
  m_baseXRotLine->setGeometry(QRect(120, 360, 61, 21));
  m_baseZRotLabel->setGeometry(QRect(270, 360, 16, 21));
  m_baseYRotLabel->setGeometry(QRect(190, 360, 16, 21));
  m_baseXRotLabel->setGeometry(QRect(110, 360, 16, 21));
  m_baseYRotLine->setGeometry(QRect(200, 360, 61, 21));
  m_baseZRotLine->setGeometry(QRect(280, 360, 61, 21));
  m_baseXLine->setValidator(m_validator);
  m_baseYLine->setValidator(m_validator);
  m_baseZLine->setValidator(m_validator);
  m_baseXRotLine->setValidator(m_validator);
  m_baseYRotLine->setValidator(m_validator);
  m_baseZRotLine->setValidator(m_validator);
  m_saveAttributesButton->setGeometry(QRect(410, 310, 201, 31));
  m_restaureAttributesButton->setGeometry(QRect(410, 350, 201, 31));
  m_jointSaveAttributesButton->setGeometry(QRect(490, 310, 201, 31));
  m_jointRestaureAttributesButton->setGeometry(QRect(490, 350, 201, 31));
  m_jointSphericalCheck->setGeometry(QRect(180, 230, 91, 26));
  m_jointTypeLabel->setGeometry(QRect(30, 230, 41, 21));
  m_jointRevoluteCheck->setGeometry(QRect(80, 230, 91, 26));
  m_jointNonActuatedCheck->setGeometry(QRect(280, 230, 121, 26));
  m_jointLimitsLabel->setGeometry(QRect(30, 260, 41, 21));
  m_jointLimitsLine11->setGeometry(QRect(110, 260, 31, 21));
  m_jointLimitsLine12->setGeometry(QRect(150, 260, 31, 21));
  m_jointLimitsLine21->setGeometry(QRect(220, 260, 31, 21));
  m_jointLimitsLine22->setGeometry(QRect(260, 260, 31, 21));
  m_jointLimitsLine11->setValidator(m_validator);
  m_jointLimitsLine12->setValidator(m_validator);
  m_jointLimitsLine21->setValidator(m_validator);
  m_jointLimitsLine22->setValidator(m_validator);
  m_jointLimitLabel1->setGeometry(QRect(90, 260, 16, 21));
  m_jointLimitLabel2->setGeometry(QRect(200, 260, 16, 21));
  m_jointPositionLabel11->setGeometry(QRect(30, 300, 131, 16));
  m_jointPositionLabel12->setGeometry(QRect(30, 320, 121, 16));
  m_jointX1Line->setGeometry(QRect(170, 310, 31, 21));
  m_jointY1Line->setGeometry(QRect(210, 310, 31, 21));
  m_jointZ1Line->setGeometry(QRect(250, 310, 31, 21));
  m_jointXRot1Line->setGeometry(QRect(300, 310, 51, 21));
  m_jointYRot1Line->setGeometry(QRect(360, 310, 51, 21));
  m_jointZRot1Line->setGeometry(QRect(420, 310, 51, 21));
  m_jointXRot1Line->setMaxLength(4);
  m_jointYRot1Line->setMaxLength(4);
  m_jointZRot1Line->setMaxLength(4);
  m_jointX1Line->setValidator(m_validator);
  m_jointY1Line->setValidator(m_validator);
  m_jointZ1Line->setValidator(m_validator);
  m_jointXRot1Line->setValidator(m_validator);
  m_jointYRot1Line->setValidator(m_validator);
  m_jointXRot1Line->setValidator(m_validator);
  m_jointX1Label->setGeometry(QRect(180, 290, 16, 21));
  m_jointY1Label->setGeometry(QRect(220, 290, 16, 21));
  m_jointZ1Label->setGeometry(QRect(260, 290, 16, 21));
  m_jointXRot1Label->setGeometry(QRect(300, 290, 41, 21));
  m_jointYRot1Label->setGeometry(QRect(360, 290, 41, 21));
  m_jointZRot1Label->setGeometry(QRect(420, 290, 41, 21));
  m_jointY2Line->setGeometry(QRect(210, 360, 31, 21));
  m_jointPositionLabel22->setGeometry(QRect(30, 370, 121, 16));
  m_jointZ2Line->setGeometry(QRect(250, 360, 31, 21));
  m_jointY2Label->setGeometry(QRect(220, 340, 16, 21));
  m_jointZRot2Line->setGeometry(QRect(420, 360, 51, 21));
  m_jointPositionLabel21->setGeometry(QRect(30, 350, 131, 16));
  m_jointZRot2Label->setGeometry(QRect(420, 340, 41, 21));
  m_jointX2Label->setGeometry(QRect(180, 340, 16, 21));
  m_jointXRot2Label->setGeometry(QRect(300, 340, 41, 21));
  m_jointYRot2Label->setGeometry(QRect(360, 340, 41, 21));
  m_jointYRot2Line->setGeometry(QRect(360, 360, 51, 21));
  m_jointX2Line->setGeometry(QRect(170, 360, 31, 21));
  m_jointZ2Label->setGeometry(QRect(260, 340, 16, 21));
  m_jointXRot2Line->setGeometry(QRect(300, 360, 51, 21));
  m_jointX2Line->setValidator(m_validator);
  m_jointY2Line->setValidator(m_validator);
  m_jointZ2Line->setValidator(m_validator);
  m_jointXRot2Line->setMaxLength(4);
  m_jointYRot2Line->setMaxLength(4);
  m_jointZRot2Line->setMaxLength(4);
  m_jointXRot2Line->setValidator(m_validator);
  m_jointYRot2Line->setValidator(m_validator);
  m_jointXRot2Line->setValidator(m_validator);
  m_jointDHParametersLabel1->setGeometry(QRect(30, 400, 141, 21));
  m_jointDHParametersLabel2->setGeometry(QRect(30, 420, 81, 21));
  m_jointAlphaLine->setGeometry(QRect(180, 420, 41, 20));
  m_jointALine->setGeometry(QRect(240, 420, 41, 20));
  m_jointDLine->setGeometry(QRect(300, 420, 41, 20));
  m_jointThetaLine->setGeometry(QRect(360, 420, 41, 20));
  m_jointAlphaLine->setValidator(m_validator);
  m_jointALine->setValidator(m_validator);
  m_jointDLine->setValidator(m_validator);
  m_jointThetaLine->setValidator(m_validator);
  m_jointAlphaLabel->setGeometry(QRect(180, 400, 41, 21));
  m_jointALabel->setGeometry(QRect(260, 400, 16, 21));
  m_jointDLabel->setGeometry(QRect(320, 400, 16, 21));
  m_jointThetaLabel->setGeometry(QRect(360, 400, 41, 21));
  m_bodyNumberLabel->setGeometry(QRect(210, 180, 71, 21));
  m_bodyNumberLine->setGeometry(QRect(290, 180, 71, 21));
  m_bodyNumberLine->setReadOnly(true);
  m_jointConnectionsLabel->setGeometry(QRect(320, 260, 171, 21));
  m_jointConnectionsLine1->setGeometry(QRect(500, 260, 31, 21));
  m_jointConnectionsLine2->setGeometry(QRect(540, 260, 31, 21));
  m_jointConnectionsLine1->setValidator(m_validator);
  m_jointConnectionsLine2->setValidator(m_validator);
  m_newRobotButton->setGeometry(QRect(30,20,311,31));
  DisplayHideJointAttributes(false);
  DisplayHideBaseAttributes(false);

  QDialog::show();
}

void
EditRobotDialog::DisplayHideJointAttributes(bool _visible){
  m_jointSaveAttributesButton->setVisible(_visible);
  m_jointRestaureAttributesButton->setVisible(_visible);
  m_jointSphericalCheck->setVisible(_visible);
  m_jointTypeLabel->setVisible(_visible);
  m_jointRevoluteCheck->setVisible(_visible);
  m_jointNonActuatedCheck->setVisible(_visible);
  m_jointLimitsLabel->setVisible(_visible);
  m_jointLimitsLine11->setVisible(_visible);
  m_jointLimitsLine12->setVisible(_visible);
  m_jointLimitsLine21->setVisible(_visible);
  m_jointLimitsLine22->setVisible(_visible);
  m_jointLimitLabel1->setVisible(_visible);
  m_jointLimitLabel2->setVisible(_visible);
  m_jointPositionLabel11->setVisible(_visible);
  m_jointPositionLabel12->setVisible(_visible);
  m_jointX1Line->setVisible(_visible);
  m_jointY1Line->setVisible(_visible);
  m_jointZ1Line->setVisible(_visible);
  m_jointXRot1Line->setVisible(_visible);
  m_jointYRot1Line->setVisible(_visible);
  m_jointZRot1Line->setVisible(_visible);
  m_jointX1Label->setVisible(_visible);
  m_jointY1Label->setVisible(_visible);
  m_jointZ1Label->setVisible(_visible);
  m_jointXRot1Label->setVisible(_visible);
  m_jointYRot1Label->setVisible(_visible);
  m_jointZRot1Label->setVisible(_visible);
  m_jointY2Line->setVisible(_visible);
  m_jointPositionLabel22->setVisible(_visible);
  m_jointZ2Line->setVisible(_visible);
  m_jointY2Label->setVisible(_visible);
  m_jointZRot2Line->setVisible(_visible);
  m_jointPositionLabel21->setVisible(_visible);
  m_jointZRot2Label->setVisible(_visible);
  m_jointX2Label->setVisible(_visible);
  m_jointXRot2Label->setVisible(_visible);
  m_jointYRot2Label->setVisible(_visible);
  m_jointYRot2Line->setVisible(_visible);
  m_jointX2Line->setVisible(_visible);
  m_jointZ2Label->setVisible(_visible);
  m_jointXRot2Line->setVisible(_visible);
  m_jointDHParametersLabel1->setVisible(_visible);
  m_jointDHParametersLabel2->setVisible(_visible);
  m_jointAlphaLine->setVisible(_visible);
  m_jointALine->setVisible(_visible);
  m_jointDLine->setVisible(_visible);
  m_jointThetaLine->setVisible(_visible);
  m_jointAlphaLabel->setVisible(_visible);
  m_jointALabel->setVisible(_visible);
  m_jointDLabel->setVisible(_visible);
  m_jointThetaLabel->setVisible(_visible);
  m_jointConnectionsLabel->setVisible(_visible);
  m_jointConnectionsLine1->setVisible(_visible);
  m_jointConnectionsLine2->setVisible(_visible);
}

void
EditRobotDialog::ClearJointLines(){
  m_jointLimitsLine11->clear();
  m_jointLimitsLine12->clear();
  m_jointLimitsLine21->clear();
  m_jointLimitsLine22->clear();
  m_jointX1Line->clear();
  m_jointY1Line->clear();
  m_jointZ1Line->clear();
  m_jointXRot1Line->clear();
  m_jointYRot1Line->clear();
  m_jointZRot1Line->clear();
  m_jointX2Line->clear();
  m_jointY2Line->clear();
  m_jointZ2Line->clear();
  m_jointXRot2Line->clear();
  m_jointYRot2Line->clear();
  m_jointZRot2Line->clear();
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
  m_jointLimitsLine21->setEnabled(false);
  m_jointLimitsLine22->setEnabled(false);
  JointNewTypeChecked(false);
  ClearJointLines();
}

void
EditRobotDialog::JointSphericalChecked(){
  m_jointSphericalCheck->setChecked(true);
  m_jointRevoluteCheck->setChecked(false);
  m_jointNonActuatedCheck->setChecked(false);
  m_jointLimitsLine21->setEnabled(true);
  m_jointLimitsLine22->setEnabled(true);
  JointNewTypeChecked(true);
}

void
EditRobotDialog::JointRevoluteChecked(){
  m_jointSphericalCheck->setChecked(false);
  m_jointRevoluteCheck->setChecked(true);
  m_jointNonActuatedCheck->setChecked(false);
  m_jointLimitsLine21->setEnabled(false);
  m_jointLimitsLine22->setEnabled(false);
  m_jointLimitsLine21->clear();
  m_jointLimitsLine22->clear();
  JointNewTypeChecked(true);
}

void
EditRobotDialog::JointNewTypeChecked(bool _isTrue){
  m_jointLimitsLine11->setEnabled(_isTrue);
  m_jointLimitsLine12->setEnabled(_isTrue);
  m_jointX1Line->setEnabled(_isTrue);
  m_jointY1Line->setEnabled(_isTrue);
  m_jointZ1Line->setEnabled(_isTrue);
  m_jointXRot1Line->setEnabled(_isTrue);
  m_jointYRot1Line->setEnabled(_isTrue);
  m_jointZRot1Line->setEnabled(_isTrue);
  m_jointX2Line->setEnabled(_isTrue);
  m_jointY2Line->setEnabled(_isTrue);
  m_jointZ2Line->setEnabled(_isTrue);
  m_jointXRot2Line->setEnabled(_isTrue);
  m_jointYRot2Line->setEnabled(_isTrue);
  m_jointZRot2Line->setEnabled(_isTrue);
  m_jointAlphaLine->setEnabled(_isTrue);
  m_jointALine->setEnabled(_isTrue);
  m_jointDLine->setEnabled(_isTrue);
  m_jointThetaLine->setEnabled(_isTrue);
  m_jointConnectionsLine1->setEnabled(_isTrue);
  m_jointConnectionsLine2->setEnabled(_isTrue);
}

void
EditRobotDialog::DisplayHideBaseAttributes(bool _visible){
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
  m_basePositionLabel->setVisible(_visible);
  m_baseRotationLabel->setVisible(_visible);
  m_baseXLine->setVisible(_visible);
  m_baseXLabel->setVisible(_visible);
  m_baseYLine->setVisible(_visible);
  m_baseYLabel->setVisible(_visible);
  m_baseZLine->setVisible(_visible);
  m_baseZLabel->setVisible(_visible);
  m_baseXRotLine->setVisible(_visible);
  m_baseZRotLabel->setVisible(_visible);
  m_baseYRotLabel->setVisible(_visible);
  m_baseXRotLabel->setVisible(_visible);
  m_baseYRotLine->setVisible(_visible);
  m_baseZRotLine->setVisible(_visible);
  m_saveAttributesButton->setVisible(_visible);
  m_restaureAttributesButton->setVisible(_visible);
}

void
EditRobotDialog::Accept(){
  RefreshVizmo();
  accept();
}

void
EditRobotDialog::ShowJoint(){
  if(m_baseList->currentItem()!=NULL){
    DisplayHideBaseAttributes(false);
    ClearBaseLines();
    ClearJointLines();
    DisplayHideJointAttributes(true);
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
    string splittedTransform1[6]={"","","","","",""};
    string splittedTransform2[6]={"","","","","",""};
    int j=0;
    do{
      splitTransform1>>splittedTransform1[j];
      j++;
    }while(splitTransform1);
    j=0;
    do{
      splitTransform2>>splittedTransform2[j];
      j++;
    }while(splitTransform2);
    if(!m_newRobotModel.at(indexBase).second.at(indexJoint)->IsRevolute()&&
          !m_newRobotModel.at(indexBase).second.at(indexJoint)->IsSpherical())
      JointNonActuatedChecked();
    else{
      if(m_newRobotModel.at(indexBase).second.at(indexJoint)->IsRevolute()){
        JointRevoluteChecked();
        m_jointLimitsLine11->insert(QString::number(jointLimits[0].first));
        m_jointLimitsLine12->insert(QString::number(jointLimits[0].second));
      }
      else{
        JointSphericalChecked();
        m_jointLimitsLine11->insert(QString::number(jointLimits[0].first));
        m_jointLimitsLine12->insert(QString::number(jointLimits[0].second));
        m_jointLimitsLine21->insert(QString::number(jointLimits[1].first));
        m_jointLimitsLine22->insert(QString::number(jointLimits[1].second));
      }
      m_jointConnectionsLine1->insert(QString::number(m_newRobotModel.at(indexBase).second.at(indexJoint)->GetPreviousIndex()));
      m_jointConnectionsLine2->insert(QString::number(m_newRobotModel.at(indexBase).second.at(indexJoint)->GetNextIndex()));
      m_jointX1Line->insert(splittedTransform1[0].c_str());
      m_jointY1Line->insert(splittedTransform1[1].c_str());
      m_jointZ1Line->insert(splittedTransform1[2].c_str());
      m_jointXRot1Line->insert(splittedTransform1[3].c_str());
      m_jointYRot1Line->insert(splittedTransform1[4].c_str());
      m_jointZRot1Line->insert(splittedTransform1[5].c_str());
      m_jointX2Line->insert(splittedTransform2[0].c_str());
      m_jointY2Line->insert(splittedTransform2[1].c_str());
      m_jointZ2Line->insert(splittedTransform2[2].c_str());
      m_jointXRot2Line->insert(splittedTransform2[3].c_str());
      m_jointYRot2Line->insert(splittedTransform2[4].c_str());
      m_jointZRot2Line->insert(splittedTransform2[5].c_str());
      m_jointAlphaLine->insert(QString::number(m_newRobotModel.at(indexBase).second.at(indexJoint)->GetAlpha()));
      m_jointALine->insert(QString::number(m_newRobotModel.at(indexBase).second.at(indexJoint)->GetA()));
      m_jointDLine->insert(QString::number(m_newRobotModel.at(indexBase).second.at(indexJoint)->GetD()));
      m_jointThetaLine->insert(QString::number(m_newRobotModel.at(indexBase).second.at(indexJoint)->GetTheta()));
    }
  }
}

void
EditRobotDialog::ShowBase(){
  if(m_baseList->currentItem()!=NULL){
    DisplayHideJointAttributes(false);
    ClearJointLines();
    ClearBaseLines();
    DisplayHideBaseAttributes(true);
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
    if(m_newRobotModel.at(index).first->IsBaseFixed()){
      BaseFixedChecked();
      ostringstream transform;
      transform<<m_newRobotModel.at(index).first->GetTransform();
      string transformString=transform.str();
      istringstream splitTransform(transformString);
      string splittedTransform[6]={"","","","","",""};
      int j=0;
      do{
        splitTransform>>splittedTransform[j];
        j++;
      }while(splitTransform);
      m_baseXLine->insert(splittedTransform[0].c_str());
      m_baseYLine->insert(splittedTransform[1].c_str());
      m_baseZLine->insert(splittedTransform[2].c_str());
      m_baseXRotLine->insert(splittedTransform[3].c_str());
      m_baseYRotLine->insert(splittedTransform[4].c_str());
      m_baseZRotLine->insert(splittedTransform[5].c_str());
    }
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
  }
}

void
EditRobotDialog::SaveBase(){
  if(m_baseList->currentItem()!=NULL){
    int index = m_baseList->row(m_baseList->currentItem());
    ostringstream properties;
    Color4 color= DEFAULT_COLOR;
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
    else if(m_baseXLine->text()==""||m_baseYLine->text()==""||
        m_baseZLine->text()==""||m_baseXRotLine->text()==""||
        m_baseYRotLine->text()==""||m_baseZRotLine->text()==""){
      OpenMessageBox("Some coordinates are missing");
      return;
    }
    else{
      properties<<m_newRobotModel.at(index).first->GetFilename()<<"  "<<"FIXED "
        <<m_baseXLine->text().toStdString() <<" "
        <<m_baseYLine->text().toStdString() <<" "
        <<m_baseZLine->text().toStdString() <<" "
        <<m_baseXRotLine->text().toStdString() <<" "
        <<m_baseYRotLine->text().toStdString() <<" "
        <<m_baseZRotLine->text().toStdString() <<"/n";
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
EditRobotDialog::SaveJoint(){
  int indexBase = m_baseList->row(m_baseList->currentItem());
  int indexJoint = m_jointList->row(m_jointList->currentItem());
  int previousBodyIndex=m_newRobotModel.at(indexBase).second.at(indexJoint)->GetPreviousIndex();
  stringstream properties;
  size_t connectionGlobalIndex=m_newRobotModel.at(indexBase).second.at(indexJoint)->GetGlobalIndex();
  if(!m_jointRevoluteCheck->isChecked() && !m_jointSphericalCheck->isChecked()
      && !m_jointNonActuatedCheck->isChecked()){
    OpenMessageBox("No joint type selected");
    return;
  }
  else if(m_jointRevoluteCheck->isChecked() || m_jointSphericalCheck->isChecked()){
    if(!JointParamChecked()){
      OpenMessageBox("Parameters are missing");
      return;
    }
    else if(m_jointLimitsLine11->text()==""||m_jointLimitsLine12->text()==""){
      OpenMessageBox("Limits are missing");
      return;
    }
    else if(m_jointSphericalCheck->isChecked()&&(m_jointLimitsLine21->text()==""||m_jointLimitsLine22->text()=="")){
      OpenMessageBox("Limits are missing");
      return;
    }
    else{
      if(m_jointConnectionsLine1->text()==""||m_jointConnectionsLine2->text()==""||
          m_jointX1Line->text()==""||m_jointY1Line->text()==""||m_jointZ1Line->text()==""||
          m_jointXRot1Line->text()==""||m_jointYRot1Line->text()==""||m_jointZRot1Line->text()==""||
          m_jointX2Line->text()==""||m_jointY2Line->text()==""||m_jointZ2Line->text()==""||
          m_jointXRot2Line->text()==""||m_jointYRot2Line->text()==""||m_jointZRot2Line->text()==""||
          m_jointAlphaLine->text()==""||m_jointALine->text()==""||
          m_jointDLine->text()==""||m_jointThetaLine->text()==""){
        OpenMessageBox("Parameters are missing");
        return;
      }
      else{
        properties<<m_jointConnectionsLine1->text().toStdString()<<" "
                  <<m_jointConnectionsLine2->text().toStdString()<<"  ";
        if(m_jointSphericalCheck->isChecked()){
          properties<<"Spherical "<<m_jointLimitsLine11->text().toStdString()
                    <<":"<<m_jointLimitsLine12->text().toStdString()<<" "
                    <<m_jointLimitsLine21->text().toStdString()
                    <<":"<<m_jointLimitsLine12->text().toStdString()<<endl;
        }
        else{
          properties<<"Revolute "<<m_jointLimitsLine11->text().toStdString()
                    <<":"<<m_jointLimitsLine12->text().toStdString()<<endl;
        }
        properties<<m_jointX1Line->text().toStdString()<<" "
                  <<m_jointY1Line->text().toStdString()<<" "
                  <<m_jointZ1Line->text().toStdString()<<" "
                  <<m_jointXRot1Line->text().toStdString()<<" "
                  <<m_jointYRot1Line->text().toStdString()<<" "
                  <<m_jointZRot1Line->text().toStdString()<<"      "
                  <<m_jointAlphaLine->text().toStdString()<<" "
                  <<m_jointALine->text().toStdString()<<" "
                  <<m_jointDLine->text().toStdString()<<" "
                  <<m_jointThetaLine->text().toStdString()<<"      "
                  <<m_jointX2Line->text().toStdString()<<" "
                  <<m_jointY2Line->text().toStdString()<<" "
                  <<m_jointZ2Line->text().toStdString()<<" "
                  <<m_jointXRot2Line->text().toStdString()<<" "
                  <<m_jointYRot2Line->text().toStdString()<<" "
                  <<m_jointZRot2Line->text().toStdString()<<endl;
      }
    }
  }
  else
    properties<<"0 0  NonActuated"<<endl;
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
  if(m_jointX1Line->text()==""||m_jointY1Line->text()==""||m_jointZ1Line->text()==""||
      m_jointXRot1Line->text()==""||m_jointYRot1Line->text()==""||m_jointZRot1Line->text()=="")
    return false;
  else if(m_jointX2Line->text()==""||m_jointY2Line->text()==""||m_jointZ2Line->text()==""||
      m_jointXRot2Line->text()==""||m_jointYRot2Line->text()==""||m_jointZRot2Line->text()=="")
    return false;
  else if(m_jointAlphaLine->text()==""||m_jointALine->text()==""||
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
    ClearBaseLines();
    m_baseXLine->setEnabled(true);
    m_baseYLine->setEnabled(true);
    m_baseZLine->setEnabled(true);
    m_baseXRotLine->setEnabled(true);
    m_baseYRotLine->setEnabled(true);
    m_baseZRotLine->setEnabled(true);
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
  ClearBaseLines();
  m_baseXLine->setEnabled(false);
  m_baseYLine->setEnabled(false);
  m_baseZLine->setEnabled(false);
  m_baseXRotLine->setEnabled(false);
  m_baseYRotLine->setEnabled(false);
  m_baseZRotLine->setEnabled(false);
}

void
EditRobotDialog::BaseTranslationalChecked(){
  m_baseTranslationalCheck->setChecked(true);
  m_baseRotationalCheck->setChecked(false);
}

void
EditRobotDialog::BaseRotationalChecked(){
  m_baseTranslationalCheck->setChecked(false);
  m_baseRotationalCheck->setChecked(true);
}

void
EditRobotDialog::ClearBaseLines(){
  m_baseXLine->clear();
  m_baseYLine->clear();
  m_baseZLine->clear();
  m_baseXRotLine->clear();
  m_baseYRotLine->clear();
  m_baseZRotLine->clear();
}

void
EditRobotDialog::AddBase(){
  QString fn = QFileDialog::getOpenFileName(this, "Choose a base to load",
      QString::null, "Files (*.g)");
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
    int indexBase = m_baseList->row(m_baseList->currentItem());
    int indexJoint = m_jointList->row(m_jointList->currentItem());
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
        QString::null, "Files (*.g)");
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
      int indexJoint = m_jointList->row(m_jointList->currentItem());
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
    size_t connectionGlobalIndex=m_newRobotModel.at(indexBase).second.at(indexJoint)->GetGlobalIndex();
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
  int indexBase = m_baseList->row(m_baseList->currentItem());
  int indexJoint = m_jointList->row(m_jointList->currentItem());
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
