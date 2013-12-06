#include "EnvironmentOptions.h"

#include <iostream>
#include <fstream>
#include <QAction>
#include <QToolBar>
#include <QPixmap>
#include <QTranslator>
#include <QFileDialog>
#include <QStatusBar>

#include "GLWidget.h"
#include "ModelSelectionWidget.h"
#include "ObstaclePosDialog.h"
#include "ChangeBoundaryDialog.h"
#include "EditRobotDialog.h"
#include "MainWindow.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

#include "Icons/RandEnv.xpm"

EnvironmentOptions::EnvironmentOptions(QWidget* _parent, MainWindow* _mainWindow)
  : OptionsBase(_parent, _mainWindow) {
    CreateActions();
    SetUpCustomSubmenu();
    SetUpToolbar();
    SetHelpTips();
  }

void
EnvironmentOptions::CreateActions(){

  //1. Create actions and add them to the map
  QAction* refreshEnv = new QAction(tr("Refresh"), this);
  m_actions["refreshEnv"] = refreshEnv;
  QAction* randEnvColors = new QAction(QPixmap(randEnvIcon), tr("Randomize Environment Colors"), this);
  m_actions["randEnvColors"] = randEnvColors;
  QAction* addObstacle = new QAction(tr("Add Obstacle"), this);
  m_actions["addObstacle"] = addObstacle;
  QAction* deleteObstacle = new QAction(tr("Delete Obstacle(s)"), this);
  m_actions["deleteObstacle"] = deleteObstacle;
  QAction* moveObstacle = new QAction(tr("Move Obstacle"), this);
  m_actions["moveObstacle"] = moveObstacle;
  QAction* changeBoundary = new QAction(tr("Change Boundary Form"), this);
  m_actions["changeBoundary"] = changeBoundary;
  QAction* editRobot = new QAction(tr("Edit the Robot "), this);
  m_actions["editRobot"] = editRobot;

  //2. Set other specifications as necessary
  m_actions["refreshEnv"]->setEnabled(false);
  m_actions["randEnvColors"]->setEnabled(false);
  m_actions["randEnvColors"]->setStatusTip(tr("Randomize environment colors"));
  m_actions["addObstacle"]->setEnabled(false);
  m_actions["deleteObstacle"]->setEnabled(false);
  m_actions["moveObstacle"]->setEnabled(false);
  m_actions["changeBoundary"]->setEnabled(false);
  m_actions["editRobot"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["refreshEnv"], SIGNAL(triggered()), this, SLOT(RefreshEnv()));
  connect(m_actions["randEnvColors"], SIGNAL(triggered()), this, SLOT(RandomizeEnvColors()));
  connect(m_actions["addObstacle"], SIGNAL(triggered()), this, SLOT(AddObstacle()));
  connect(m_actions["deleteObstacle"], SIGNAL(triggered()), this, SLOT(DeleteObstacle()));
  connect(m_actions["moveObstacle"], SIGNAL(triggered()), this, SLOT(MoveObstacle()));
  connect(m_actions["changeBoundary"], SIGNAL(triggered()), this, SLOT(ChangeBoundaryForm()));
  connect(m_actions["editRobot"], SIGNAL(triggered()), this, SLOT(EditRobot()));
}

void
EnvironmentOptions::SetUpCustomSubmenu(){
  m_submenu = new QMenu("Environment", this);
  m_submenu->addAction(m_actions["refreshEnv"]);
  m_submenu->addAction(m_actions["randEnvColors"]);
  m_obstacleMenu = new QMenu("Obstacle Tools", this);
  m_obstacleMenu->addAction(m_actions["addObstacle"]);
  m_obstacleMenu->addAction(m_actions["deleteObstacle"]);
  m_obstacleMenu->addAction(m_actions["moveObstacle"]);
  m_submenu->addMenu(m_obstacleMenu);
  m_submenu->addAction(m_actions["changeBoundary"]);
  m_submenu->addAction(m_actions["editRobot"]);
  m_obstacleMenu->setEnabled(false);
}

void
EnvironmentOptions::SetUpToolbar(){
  m_toolbar = new QToolBar(m_mainWindow);
  m_toolbar->addAction(m_actions["randEnvColors"]);
}

void
EnvironmentOptions::Reset(){
  m_actions["refreshEnv"]->setEnabled(true);
  m_actions["randEnvColors"]->setEnabled(true);
  m_actions["addObstacle"]->setEnabled(true);
  m_actions["deleteObstacle"]->setEnabled(true);
  m_actions["moveObstacle"]->setEnabled(true);
  m_actions["changeBoundary"]->setEnabled(true);
  m_actions["editRobot"]->setEnabled(true);
  m_obstacleMenu->setEnabled(true);
}

void
EnvironmentOptions::SetHelpTips(){
  m_actions["randEnvColors"]->setWhatsThis(tr("Click this button to"
        " randomize the colors of the environment objects."));
  m_actions["addObstacle"]->setWhatsThis(tr("Add obstacle button"));
  m_actions["deleteObstacle"]->setWhatsThis(tr("Delete obstacle button"));
  m_actions["moveObstacle"]->setWhatsThis(tr("Move obstacle button"));
  m_actions["changeBoundary"]->setWhatsThis(tr("Change the form of the boundary"));
  m_actions["editRobot"]->setWhatsThis(tr("Edit the robot"));
}

//Slots
void
EnvironmentOptions::ChangeBoundaryForm(){
  ChangeBoundaryDialog c(this);
  if(c.exec() != QDialog::Accepted)
    return;
  RefreshEnv();
  m_modelSelectionWidget=m_mainWindow->GetModelSelectionWidget();
  m_modelSelectionWidget->ResetLists();
}

void EnvironmentOptions::EditRobot(){
  EditRobotDialog e(m_mainWindow, this);
  if(e.exec() != QDialog::Accepted)
    return;
  RefreshEnv();
}

void
EnvironmentOptions::RefreshEnv(){
  GetVizmo().GetEnv()->SetRenderMode(SOLID_MODE);
  m_mainWindow->GetModelSelectionWidget()->reset();
  m_mainWindow->GetGLScene()->updateGL();
}

void
EnvironmentOptions::RandomizeEnvColors(){
  GetVizmo().GetEnv()->ChangeColor();
  m_mainWindow->GetGLScene()->updateGL();
}

void
EnvironmentOptions::DeleteObstacle(){
  vector<MultiBodyModel*> mBody = GetVizmo().GetEnv()->GetMultiBodies();
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  typedef vector<Model*>::iterator SI;
  for(SI i = sel.begin(); i!= sel.end(); i++){
    for(size_t j=0;j<mBody.size();j++){
      if(*i==mBody[j]&&!mBody[j]->IsActive())
        GetVizmo().GetEnv()->DeleteMBModel(mBody[j]);
    }
  }
  RefreshEnv();
  m_modelSelectionWidget=m_mainWindow->GetModelSelectionWidget();
  m_modelSelectionWidget->ResetLists();
}
void
EnvironmentOptions::MoveObstacle(){
  vector<MultiBodyModel*> mBody = GetVizmo().GetEnv()->GetMultiBodies();
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  typedef vector<Model*>::iterator SI;
  for(SI i = sel.begin(); i!= sel.end(); i++){
    for(size_t j=0;j<mBody.size();j++){
      if(*i==mBody[j]&&!mBody[j]->IsActive()){
        ObstaclePosDialog o(mBody[j], m_mainWindow, this);
        o.exec();
      }
    }
  }
  RefreshEnv();
  m_modelSelectionWidget=m_mainWindow->GetModelSelectionWidget();
  m_modelSelectionWidget->ResetLists();
}

void
EnvironmentOptions::AddObstacle(){
  QString fn = QFileDialog::getOpenFileName(this, "Choose an obstacle to load",
      QString::null, "Files (*.g)");
  if (!fn.isEmpty()){
    m_modelFilename = fn.toStdString();
    m_filename = GetFilename(m_modelFilename);
    string stringAttributes = ("Passive \n#VIZMO_COLOR 0.0 0.0 1 \n" + m_filename
                                + "  " + "0 0 0 0 0 0" + "\n");
    istringstream isStringAttributes(stringAttributes);
    m_modelFileDir = GetFileDir(m_modelFilename,m_filename);
    m_multiBodyModel = new MultiBodyModel();
    m_multiBodyModel->ParseMultiBody(isStringAttributes,m_modelFileDir);
    GetVizmo().GetEnv()->AddMBModel(m_multiBodyModel);
    RefreshEnv();
    m_modelSelectionWidget=m_mainWindow->GetModelSelectionWidget();
    m_modelSelectionWidget->ResetLists();
    ObstaclePosDialog o(m_multiBodyModel, m_mainWindow, this);
    if(o.exec() != QDialog::Accepted)
      return;
  }
  else
    m_mainWindow->statusBar()->showMessage("Loading aborded");
}

string
EnvironmentOptions::GetFilename(string _modelFilename){
  string filename;
  stringstream stream(_modelFilename);
  while(getline(stream, filename, '/')){}
  return filename;
}

string
EnvironmentOptions::GetFileDir(string _modelFilename, string _filename){
  int modelFilenameSize = _modelFilename.size();
  int filenameSize = _filename.size();
  string modelFileDir = _modelFilename;
  modelFileDir.replace(modelFilenameSize-filenameSize, filenameSize, "");
  return modelFileDir;
}
