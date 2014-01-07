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
#include "Models/RegionSphereModel.h"
#include "Models/Vizmo.h"

#include "Icons/RandEnv.xpm"
#include "Icons/AddSphereRegion.xpm"
#include "Icons/DeleteRegion.xpm"

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
  QAction* moveObstacle = new QAction(tr("Move Obstacle(s)"), this);
  m_actions["moveObstacle"] = moveObstacle;
  QAction* duplicateObstacle = new QAction(tr("Duplicate Obstacle(s)"), this);
  m_actions["duplicateObstacle"] = duplicateObstacle;
  QAction* changeBoundary = new QAction(tr("Change Boundary Form"), this);
  m_actions["changeBoundary"] = changeBoundary;
  QAction* editRobot = new QAction(tr("Edit the Robot "), this);
  m_actions["editRobot"] = editRobot;
  QAction* addRegionSphere = new QAction(QPixmap(addsphereregion), tr("Add Spherical Region"), this);
  m_actions["addRegionSphere"] = addRegionSphere;
  QAction* deleteRegion = new QAction(QPixmap(deleteregion), tr("Delete Region"), this);
  m_actions["deleteRegion"] = deleteRegion;

  //2. Set other specifications as necessary
  m_actions["refreshEnv"]->setEnabled(false);
  m_actions["randEnvColors"]->setEnabled(false);
  m_actions["randEnvColors"]->setStatusTip(tr("Randomize environment colors"));
  m_actions["addObstacle"]->setEnabled(false);
  m_actions["deleteObstacle"]->setEnabled(false);
  m_actions["moveObstacle"]->setEnabled(false);
  m_actions["duplicateObstacle"]->setEnabled(false);
  m_actions["changeBoundary"]->setEnabled(false);
  m_actions["editRobot"]->setEnabled(false);
  m_actions["addRegionSphere"]->setEnabled(false);
  m_actions["deleteRegion"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["refreshEnv"], SIGNAL(triggered()), this, SLOT(RefreshEnv()));
  connect(m_actions["randEnvColors"], SIGNAL(triggered()), this, SLOT(RandomizeEnvColors()));
  connect(m_actions["addObstacle"], SIGNAL(triggered()), this, SLOT(AddObstacle()));
  connect(m_actions["deleteObstacle"], SIGNAL(triggered()), this, SLOT(DeleteObstacle()));
  connect(m_actions["moveObstacle"], SIGNAL(triggered()), this, SLOT(MoveObstacle()));
  connect(m_actions["duplicateObstacle"], SIGNAL(triggered()), this, SLOT(DuplicateObstacles()));
  connect(m_actions["changeBoundary"], SIGNAL(triggered()), this, SLOT(ChangeBoundaryForm()));
  connect(m_actions["editRobot"], SIGNAL(triggered()), this, SLOT(EditRobot()));
  connect(m_actions["addRegionSphere"], SIGNAL(triggered()), this, SLOT(AddRegionSphere()));
  connect(m_actions["deleteRegion"], SIGNAL(triggered()), this, SLOT(DeleteRegion()));
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
  m_obstacleMenu->addAction(m_actions["duplicateObstacle"]);
  m_submenu->addMenu(m_obstacleMenu);
  m_submenu->addAction(m_actions["changeBoundary"]);
  m_submenu->addAction(m_actions["editRobot"]);
  m_submenu->addAction(m_actions["addRegionSphere"]);
  m_submenu->addAction(m_actions["deleteRegion"]);
  m_obstacleMenu->setEnabled(false);
}

void
EnvironmentOptions::SetUpToolbar(){
  m_toolbar = new QToolBar(m_mainWindow);
  m_toolbar->addAction(m_actions["randEnvColors"]);
  m_toolbar->addAction(m_actions["addRegionSphere"]);
  m_toolbar->addAction(m_actions["deleteRegion"]);
}

void
EnvironmentOptions::Reset(){
  m_actions["refreshEnv"]->setEnabled(true);
  m_actions["randEnvColors"]->setEnabled(true);
  m_actions["addObstacle"]->setEnabled(true);
  m_actions["deleteObstacle"]->setEnabled(true);
  m_actions["moveObstacle"]->setEnabled(true);
  m_actions["duplicateObstacle"]->setEnabled(true);
  m_actions["changeBoundary"]->setEnabled(true);
  m_actions["editRobot"]->setEnabled(true);
  m_actions["addRegionSphere"]->setEnabled(true);
  m_actions["deleteRegion"]->setEnabled(true);
  m_obstacleMenu->setEnabled(true);
}

void
EnvironmentOptions::SetHelpTips(){
  m_actions["randEnvColors"]->setWhatsThis(tr("Click this button to"
        " randomize the colors of the environment objects."));
  m_actions["addObstacle"]->setWhatsThis(tr("Add obstacle button"));
  m_actions["deleteObstacle"]->setWhatsThis(tr("Delete obstacle button"));
  m_actions["moveObstacle"]->setWhatsThis(tr("Move obstacle button"));
  m_actions["duplicateObstacle"]->setWhatsThis(tr("duplicate obstacle button"));
  m_actions["changeBoundary"]->setWhatsThis(tr("Change the form of the boundary"));
  m_actions["editRobot"]->setWhatsThis(tr("Edit the robot"));
  m_actions["addRegionSphere"]->setWhatsThis(tr("Add a spherical region to aid planner"));
  m_actions["deleteRegion"]->setWhatsThis(tr("Remove a region from the scene"));
}

//Slots
void
EnvironmentOptions::ChangeBoundaryForm(){
  ChangeBoundaryDialog c(this);
  if(c.exec() != QDialog::Accepted)
    return;
  while(GetVizmo().GetSelectedModels().size()!=0)
    GetVizmo().GetSelectedModels().pop_back();
  GetVizmo().GetSelectedModels().push_back(GetVizmo().GetEnv()->GetBoundary());
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
  while(GetVizmo().GetSelectedModels().size()!=0)
    GetVizmo().GetSelectedModels().pop_back();
  RefreshEnv();
  m_modelSelectionWidget=m_mainWindow->GetModelSelectionWidget();
  m_modelSelectionWidget->ResetLists();
}

void
EnvironmentOptions::MoveObstacle(){
  vector<MultiBodyModel*> mBody = GetVizmo().GetEnv()->GetMultiBodies();
  vector<MultiBodyModel*> mBodiesToMove;
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  typedef vector<Model*>::iterator SI;
  for(SI i = sel.begin(); i!= sel.end(); i++){
    for(size_t j=0;j<mBody.size();j++){
      if(*i==mBody[j]&&!mBody[j]->IsActive()){
        mBodiesToMove.push_back(mBody[j]);
      }
    }
  }
  ObstaclePosDialog o(mBodiesToMove, m_mainWindow, this);
  o.exec();
  RefreshEnv();
  m_modelSelectionWidget=m_mainWindow->GetModelSelectionWidget();
  m_modelSelectionWidget->ResetLists();
}

void
EnvironmentOptions::DuplicateObstacles(){
  vector<MultiBodyModel*> mBody = GetVizmo().GetEnv()->GetMultiBodies();
  vector<MultiBodyModel*> mBodiesToCopy;
  int sizeMB=mBody.size();
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  typedef vector<Model*>::iterator SI;
  for(SI i = sel.begin(); i!= sel.end(); i++){
    for(int j=0;j<sizeMB;j++){
      if(*i==mBody[j]&&!mBody[j]->IsActive()){
        stringstream properties;
        properties<<"Passive \n #VIZMO_COLOR ";
        vector<BodyModel*> bodies = mBody[j]->GetBodies();
        properties<<" "<<bodies.back()->GetColor()[0]
                  <<" "<<bodies.back()->GetColor()[1]
                  <<" "<<bodies.back()->GetColor()[2]<<endl;
        properties<<bodies.back()->GetFilename()<<"  ";
        ostringstream transform;
        transform<<bodies.back()->GetTransform();
        string transformString=transform.str();
        istringstream splitTransform(transformString);
        string splittedTransform[6]={"","","","","",""};
        int j=0;
        do{
          splitTransform>>splittedTransform[j];
          j++;
        }while(splitTransform);
        string temp;
        temp=splittedTransform[3];
        splittedTransform[3]=splittedTransform[5];
        splittedTransform[5]=temp;
        for(int i=0; i<6; i++)
          properties<<splittedTransform[i]<<" ";
        properties<<endl;
        m_multiBodyModel = new MultiBodyModel();
        m_multiBodyModel->ParseMultiBody(properties,bodies.back()->GetDirectory());
        mBodiesToCopy.push_back(m_multiBodyModel);
        GetVizmo().GetEnv()->AddMBModel(m_multiBodyModel);
      }
    }
  }
  while(GetVizmo().GetSelectedModels().size()!=0)
    GetVizmo().GetSelectedModels().pop_back();
  typedef vector<MultiBodyModel*>::iterator MB;
  for(MB i = mBodiesToCopy.begin(); i!= mBodiesToCopy.end(); i++)
    GetVizmo().GetSelectedModels().push_back(*i);
  ObstaclePosDialog o(mBodiesToCopy, m_mainWindow, this);
  o.exec();
  RefreshEnv();
  m_modelSelectionWidget=m_mainWindow->GetModelSelectionWidget();
  m_modelSelectionWidget->ResetLists();
}

void
EnvironmentOptions::AddObstacle(){
  QString fn = QFileDialog::getOpenFileName(this, "Choose an obstacle to load",
      QString::null, "Files  (*.g *.obj)");
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
    while(GetVizmo().GetSelectedModels().size()!=0)
      GetVizmo().GetSelectedModels().pop_back();
    GetVizmo().GetSelectedModels().push_back(m_multiBodyModel);
    RefreshEnv();
    m_modelSelectionWidget=m_mainWindow->GetModelSelectionWidget();
    m_modelSelectionWidget->ResetLists();
    vector<MultiBodyModel*> vMB;
    vMB.push_back(m_multiBodyModel);
    ObstaclePosDialog o(vMB, m_mainWindow, this);
    if(o.exec() != QDialog::Accepted)
      return;
  }
  else
    m_mainWindow->statusBar()->showMessage("Loading aborted");
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

void
EnvironmentOptions::AddRegionSphere() {
  //create new spherical region and add to environment
  RegionSphereModel* r = new RegionSphereModel();
  GetVizmo().GetEnv()->AddRegion(r);

  //set mouse events to current region for GLWidget
  m_mainWindow->GetGLScene()->SetCurrentRegion(r);
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  GetVizmo().GetSelectedModels().clear();
  GetVizmo().GetSelectedModels().push_back(r);
  m_mainWindow->GetModelSelectionWidget()->Select();
}

void
EnvironmentOptions::DeleteRegion() {
  RegionModel* r = m_mainWindow->GetGLScene()->GetCurrentRegion();
  if(r) {
    GetVizmo().GetEnv()->DeleteRegion(r);
    GetVizmo().GetSelectedModels().clear();
    m_mainWindow->GetModelSelectionWidget()->ResetLists();
    m_mainWindow->GetGLScene()->SetCurrentRegion(NULL);
  }
}
