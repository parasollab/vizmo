#include "EnvironmentOptions.h"

#include "ChangeBoundaryDialog.h"
#include "EditRobotDialog.h"
#include "GLWidget.h"
#include "MainWindow.h"
#include "ModelSelectionWidget.h"
#include "ObstaclePosDialog.h"
#include "Models/EnvModel.h"
#include "Models/RegionBoxModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/Vizmo.h"

#include "Icons/AddSphereRegion.xpm"
#include "Icons/AddBoxRegion.xpm"
#include "Icons/DeleteRegion.xpm"
#include "Icons/RandEnv.xpm"

EnvironmentOptions::EnvironmentOptions(QWidget* _parent, MainWindow* _mainWindow)
  : OptionsBase(_parent, _mainWindow) {
    CreateActions();
    SetUpCustomSubmenu();
    SetUpToolbar();
    SetHelpTips();
  }

void
EnvironmentOptions::RefreshEnv(){
  GetVizmo().GetEnv()->SetRenderMode(SOLID_MODE);
  m_mainWindow->GetModelSelectionWidget()->reset();
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  m_mainWindow->GetGLScene()->updateGL();
}

void
EnvironmentOptions::RandomizeEnvColors(){
  GetVizmo().GetEnv()->ChangeColor();
  m_mainWindow->GetGLScene()->updateGL();
}

void
EnvironmentOptions::AddObstacle(){
  QString fn = QFileDialog::getOpenFileName(this, "Choose an obstacle to load",
      QString::null, "Files  (*.g *.obj)");

  if(!fn.isEmpty()) {
    string modelFilename = fn.toStdString();
    string directory = GetPathName(modelFilename);
    size_t pos = modelFilename.rfind('/');
    string filename = pos == string::npos ? modelFilename : modelFilename.substr(pos+1, string::npos);

    MultiBodyModel* m = new MultiBodyModel(GetVizmo().GetEnv(), directory, filename, Transformation());
    GetVizmo().GetEnv()->AddMBModel(m);

    GetVizmo().GetSelectedModels().clear();
    GetVizmo().GetSelectedModels().push_back(m);

    RefreshEnv();

    vector<MultiBodyModel*> v(1, m);
    ObstaclePosDialog o(v, m_mainWindow, this);
    if(o.exec() != QDialog::Accepted)
      return;
  }
  else
    m_mainWindow->statusBar()->showMessage("Loading aborted");
}

void
EnvironmentOptions::DeleteObstacle(){
  vector<MultiBodyModel*> toDel;
  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  //grab the bodies from the selected vector
  typedef vector<Model*>::iterator SIT;
  for(SIT sit = sel.begin(); sit != sel.end(); ++sit)
    if((*sit)->Name() == "MultiBody" && !((MultiBodyModel*)(*sit))->IsActive())
      toDel.push_back((MultiBodyModel*)*sit);

  //alert that only non-active multibodies can be selected
  if(toDel.empty() || toDel.size() != sel.size()) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Alert");
    msgBox.setText("Must select one or more non-active multibodies only.");
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.exec();
  }
  //successful selection, show ObstaclePosDialog
  else {
    typedef vector<MultiBodyModel*>::iterator MIT;
    for(MIT mit = toDel.begin(); mit != toDel.end(); ++mit)
        GetVizmo().GetEnv()->DeleteMBModel(*mit);

    GetVizmo().GetSelectedModels().clear();
    RefreshEnv();
  }
}

void
EnvironmentOptions::MoveObstacle(){
  vector<MultiBodyModel*> toMove;
  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  //grab the bodies from the selected vector
  typedef vector<Model*>::iterator SIT;
  for(SIT sit = sel.begin(); sit != sel.end(); ++sit)
    if((*sit)->Name() == "MultiBody" && !((MultiBodyModel*)(*sit))->IsActive())
      toMove.push_back((MultiBodyModel*)*sit);

  //alert that only non-active multibodies can be selected
  if(toMove.empty() || toMove.size() != sel.size()) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Alert");
    msgBox.setText("Must select one or more non-active multibodies only.");
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.exec();
  }
  //successful selection, show ObstaclePosDialog
  else {
    ObstaclePosDialog o(toMove, m_mainWindow, this);
    o.exec();
    RefreshEnv();
  }
}

void
EnvironmentOptions::DuplicateObstacles() {
  vector<MultiBodyModel*> toCopy;
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  typedef vector<Model*>::iterator SIT;
  for(SIT sit = sel.begin(); sit != sel.end(); ++sit)
    if((*sit)->Name() == "MultiBody" && !((MultiBodyModel*)(*sit))->IsActive())
      toCopy.push_back((MultiBodyModel*)(*sit));

  //alert that only non-active multibodies can be selected
  if(toCopy.empty() || toCopy.size() != sel.size()) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Alert");
    msgBox.setText("Must select one or more non-active multibodies only.");
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.exec();
  }
  //successful selection, copy and show ObstaclePosDialog
  else {
    vector<MultiBodyModel*> copies;
    typedef vector<MultiBodyModel*>::iterator MIT;
    for(MIT mit = toCopy.begin(); mit != toCopy.end(); ++mit) {
      MultiBodyModel* m = new MultiBodyModel(**mit);
      copies.push_back(m);
      GetVizmo().GetEnv()->AddMBModel(m);
    }
    sel.clear();
    copy(copies.begin(), copies.end(), back_inserter(sel));

    ObstaclePosDialog o(copies, m_mainWindow, this);
    o.exec();

    RefreshEnv();
  }
}

void
EnvironmentOptions::ChangeBoundaryForm(){
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  //alert that only the boundary should be selected
  if(sel.size() != 1 || !(sel[0]->Name() == "Bounding Box" || sel[0]->Name() == "Bounding Sphere")) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Alert");
    msgBox.setText("Must select only the boundary.");
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.exec();
  }
  //successful selection, show ChangeBoundaryDialog
  else {
    ChangeBoundaryDialog c(this);
    if(c.exec() != QDialog::Accepted)
      return;
    sel.clear();
    sel.push_back(GetVizmo().GetEnv()->GetBoundary());
    RefreshEnv();
  }
}

void EnvironmentOptions::EditRobot() {
  EditRobotDialog e(m_mainWindow, this);
  if(e.exec() != QDialog::Accepted)
    return;
  RefreshEnv();
}

void
EnvironmentOptions::AddRegionBox() {
  //create new spherical region and add to environment
  RegionBoxModel* r = new RegionBoxModel();
  GetVizmo().GetEnv()->AddRegion(r);

  //set mouse events to current region for GLWidget
  m_mainWindow->GetGLScene()->SetCurrentRegion(r);
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  GetVizmo().GetSelectedModels().clear();
  GetVizmo().GetSelectedModels().push_back(r);
  m_mainWindow->GetModelSelectionWidget()->Select();
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

void
EnvironmentOptions::MapEnvironment() {
  //clear any map and path currently loaded
  //delete GetVizmo().GetMap();
  //delete GetVizmo().GetPath();
  //call function somewhere to spark the UG strategy
  GetVizmo().Solve("regions");
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
  QAction* changeBoundary = new QAction(tr("Change Boundary"), this);
  m_actions["changeBoundary"] = changeBoundary;
  QAction* editRobot = new QAction(tr("Edit the Robot "), this);
  m_actions["editRobot"] = editRobot;
  QAction* addRegionSphere = new QAction(QPixmap(addsphereregion), tr("Add Spherical Region"), this);
  m_actions["addRegionSphere"] = addRegionSphere;
  QAction* addRegionBox = new QAction(QPixmap(addboxregion), tr("Add Box Region"), this);
  m_actions["addRegionBox"] = addRegionBox;
  QAction* deleteRegion = new QAction(QPixmap(deleteregion), tr("Delete Region"), this);
  m_actions["deleteRegion"] = deleteRegion;
  QAction* ugmp = new QAction(QPixmap(randEnvIcon), tr("Map Environment"), this);
  m_actions["ugmp"] = ugmp;

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
  m_actions["addRegionBox"]->setEnabled(false);
  m_actions["deleteRegion"]->setEnabled(false);
  m_actions["ugmp"]->setEnabled(false);

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
  connect(m_actions["addRegionBox"], SIGNAL(triggered()), this, SLOT(AddRegionBox()));
  connect(m_actions["deleteRegion"], SIGNAL(triggered()), this, SLOT(DeleteRegion()));
  connect(m_actions["ugmp"], SIGNAL(triggered()), this, SLOT(MapEnvironment()));
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
  m_submenu->addAction(m_actions["addRegionBox"]);
  m_submenu->addAction(m_actions["deleteRegion"]);
  m_submenu->addAction(m_actions["ugmp"]);
  m_obstacleMenu->setEnabled(false);
}

void
EnvironmentOptions::SetUpToolbar(){
  m_toolbar = new QToolBar(m_mainWindow);
  m_toolbar->addAction(m_actions["randEnvColors"]);
  m_toolbar->addAction(m_actions["addRegionSphere"]);
  m_toolbar->addAction(m_actions["addRegionBox"]);
  m_toolbar->addAction(m_actions["deleteRegion"]);
  m_toolbar->addAction(m_actions["ugmp"]);
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
  m_actions["addRegionBox"]->setEnabled(true);
  m_actions["deleteRegion"]->setEnabled(true);
  m_actions["ugmp"]->setEnabled(true);
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
  m_actions["changeBoundary"]->setWhatsThis(tr("Change the boundary"));
  m_actions["editRobot"]->setWhatsThis(tr("Edit the robot"));
  m_actions["addRegionSphere"]->setWhatsThis(tr("Add a spherical region to aid planner"));
  m_actions["addRegionBox"]->setWhatsThis(tr("Add a box region to aid planner"));
  m_actions["deleteRegion"]->setWhatsThis(tr("Remove a region from the scene"));
  m_actions["ugmp"]->setWhatsThis(tr("Map an environment using region strategy"));
}

