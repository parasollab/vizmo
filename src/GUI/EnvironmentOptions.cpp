#include "EnvironmentOptions.h"

#include "ChangeBoundaryDialog.h"
#include "EditRobotDialog.h"
#include "GLWidget.h"
#include "MainMenu.h"
#include "MainWindow.h"
#include "ModelSelectionWidget.h"
#include "ObstaclePosDialog.h"

#include "Models/EnvModel.h"
#include "Models/RegionBoxModel.h"
#include "Models/RegionBox2DModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/RegionSphere2DModel.h"
#include "Models/RobotModel.h"
#include "Models/Vizmo.h"

#include "Icons/AddSphereRegion.xpm"
#include "Icons/AddBoxRegion.xpm"
#include "Icons/DeleteRegion.xpm"
#include "Icons/RandEnv.xpm"
#include "Icons/MapEnv.xpm"
#include "Icons/AttractRegion.xpm"
#include "Icons/AvoidRegion.xpm"
#include "Icons/AddObstacle.xpm"
#include "Icons/DeleteObstacle.xpm"
#include "Icons/MoveObstacle.xpm"
#include "Icons/DuplicateObstacle.xpm"
#include "Icons/ChangeBoundary.xpm"
#include "Icons/EditRobot.xpm"
#include "Icons/RefreshEnv.xpm"

#include "Utilities/AlertUser.h"

//<<<<<<< .working
EnvironmentOptions::
EnvironmentOptions(QWidget* _parent, MainWindow* _mainWindow) :
    OptionsBase(_parent, _mainWindow), m_regionsStarted(false),
    m_threadDone(true), m_thread(NULL), m_editRobotDialog(NULL),
    m_changeBoundaryDialog(NULL) {
  CreateActions();
  SetUpCustomSubmenu();
  //SetUpToolbar(); currently using tool tabs
  SetUpToolTab();
  SetHelpTips();
}

void
EnvironmentOptions::
RefreshEnv() {
  GetVizmo().GetEnv()->SetRenderMode(SOLID_MODE);
  m_mainWindow->GetModelSelectionWidget()->reset();
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  m_mainWindow->GetGLScene()->updateGL();
}

void
EnvironmentOptions::
RandomizeEnvColors() {
  GetVizmo().GetEnv()->ChangeColor();
  m_mainWindow->GetGLScene()->updateGL();
}

void
EnvironmentOptions::
AddObstacle() {
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
    ObstaclePosDialog* opd = new ObstaclePosDialog(m_mainWindow, v);
    m_mainWindow->ShowDialog(opd);
  }
  else
    m_mainWindow->statusBar()->showMessage("Loading aborted");
}

void
EnvironmentOptions::
DeleteObstacle() {
  vector<MultiBodyModel*> toDel;
  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  //grab the bodies from the selected vector
  typedef vector<Model*>::iterator SIT;
  for(SIT sit = sel.begin(); sit != sel.end(); ++sit)
    if((*sit)->Name() == "MultiBody" && !((MultiBodyModel*)(*sit))->IsActive())
      toDel.push_back((MultiBodyModel*)*sit);

  //alert that only non-active multibodies can be selected
  if(toDel.empty() || toDel.size() != sel.size())
    AlertUser("Must select one or more non-active multibodies only.");

  //successful selection, delete obstacle(s)
  else {
    typedef vector<MultiBodyModel*>::iterator MIT;
    for(MIT mit = toDel.begin(); mit != toDel.end(); ++mit)
        GetVizmo().GetEnv()->DeleteMBModel(*mit);

    GetVizmo().GetSelectedModels().clear();
    RefreshEnv();
  }
}

void
EnvironmentOptions::
MoveObstacle() {
  vector<MultiBodyModel*> toMove;
  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  //grab the bodies from the selected vector
  typedef vector<Model*>::iterator SIT;
  for(SIT sit = sel.begin(); sit != sel.end(); ++sit)
    if((*sit)->Name() == "MultiBody" && !((MultiBodyModel*)(*sit))->IsActive())
      toMove.push_back((MultiBodyModel*)*sit);

  //alert that only non-active multibodies can be selected
  if(toMove.empty() || toMove.size() != sel.size())
    AlertUser("Must select one or more non-active multibodies only.");

  //successful selection, show ObstaclePosDialog
  else {
    ObstaclePosDialog* opd = new ObstaclePosDialog(m_mainWindow, toMove);
    m_mainWindow->ShowDialog(opd);
  }
}

void
EnvironmentOptions::
DuplicateObstacles() {
  vector<MultiBodyModel*> toCopy;
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  typedef vector<Model*>::iterator SIT;
  for(SIT sit = sel.begin(); sit != sel.end(); ++sit)
    if((*sit)->Name() == "MultiBody" && !((MultiBodyModel*)(*sit))->IsActive())
      toCopy.push_back((MultiBodyModel*)(*sit));

  //alert that only non-active multibodies can be selected
  if(toCopy.empty() || toCopy.size() != sel.size())
    AlertUser("Must select one or more non-active multibodies only.");

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

    ObstaclePosDialog* opd = new ObstaclePosDialog(m_mainWindow, copies);
    m_mainWindow->ShowDialog(opd);
  }
}

void
EnvironmentOptions::
ChangeBoundaryForm() {
  if(m_changeBoundaryDialog == NULL) {
    vector<Model*>& sel = GetVizmo().GetSelectedModels();
    //alert that only the boundary should be selected
    if(sel.size() != 1 || !(sel[0]->Name() == "Bounding Box" ||
          sel[0]->Name() == "Bounding Sphere")) {
      AlertUser("Must select only the boundary.");
    }
    //successful selection, show ChangeBoundaryDialog
    else {
      m_changeBoundaryDialog = new ChangeBoundaryDialog(m_mainWindow);
      m_mainWindow->ShowDialog(m_changeBoundaryDialog);
    }
  }
}

void
EnvironmentOptions::
EditRobot() {
  if(m_editRobotDialog == NULL) {
    m_editRobotDialog = new EditRobotDialog(m_mainWindow);
    m_mainWindow->ShowDialog(m_editRobotDialog);
  }
}

void
EnvironmentOptions::
AddRegionBox() {
  if(!m_regionsStarted) {
    GetVizmo().StartClock("Pre-regions");
    m_regionsStarted = true;
  }

  //create new box region
  RegionModel* r;

  //check to see if robot is 2D or 3D, set region to the same
  RobotModel* robot = GetVizmo().GetRobot();
  if(robot->IsPlanar()) r = new RegionBox2DModel();
  else r = new RegionBoxModel();

  //add region to environment
  GetVizmo().GetEnv()->AddNonCommitRegion(r);

  //set mouse events to current region for GLWidget
  m_mainWindow->GetGLScene()->SetCurrentRegion(r);
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  GetVizmo().GetSelectedModels().clear();
  GetVizmo().GetSelectedModels().push_back(r);
  m_mainWindow->GetModelSelectionWidget()->Select();
}

void
EnvironmentOptions::
AddRegionSphere() {
  if(!m_regionsStarted) {
    GetVizmo().StartClock("Pre-regions");
    m_regionsStarted = true;
  }

  //create new sphere region
  RegionModel* r;

  //check to see if robot is 2D or 3D, set region to the same
  RobotModel* robot = GetVizmo().GetRobot();
  if(robot->IsPlanar()) r = new RegionSphere2DModel();
  else r = new RegionSphereModel();

  //add region to environment
  GetVizmo().GetEnv()->AddNonCommitRegion(r);

  //set mouse events to current region for GLWidget
  m_mainWindow->GetGLScene()->SetCurrentRegion(r);
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  GetVizmo().GetSelectedModels().clear();
  GetVizmo().GetSelectedModels().push_back(r);
  m_mainWindow->GetModelSelectionWidget()->Select();
}

void
EnvironmentOptions::
DeleteRegion() {
  RegionModel* r = m_mainWindow->GetGLScene()->GetCurrentRegion();
  if(r) {
    GetVizmo().GetEnv()->DeleteRegion(r);
    GetVizmo().GetSelectedModels().clear();
    m_mainWindow->GetModelSelectionWidget()->ResetLists();
    m_mainWindow->GetGLScene()->SetCurrentRegion(NULL);
  }
}

void
EnvironmentOptions::
MakeRegionAttract() {
  ChangeRegionType(true);
}

void
EnvironmentOptions::
MakeRegionAvoid() {
  ChangeRegionType(false);
}

void
EnvironmentOptions::
ChangeRegionType(bool _attract) {
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  //alert that only the boundary should be selected
  if(sel.size() == 1 && (sel[0]->Name() == "Box Region" || sel[0]->Name() == "Sphere Region"
        || sel[0]->Name() == "Box Region 2D" || sel[0]->Name() == "Sphere Region 2D")) {
    RegionModel* r = (RegionModel*)sel[0];
    if(GetVizmo().GetEnv()->IsNonCommitRegion(r)) {
      r->SetType(_attract ? RegionModel::ATTRACT : RegionModel::AVOID);
      GetVizmo().GetEnv()->ChangeRegionType(r, _attract);
    }
  }
}

void
EnvironmentOptions::
HandleTimer() {
  m_mainWindow->GetGLScene()->updateGL();
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
}

void
EnvironmentOptions::
ThreadDone() {
  m_threadDone = true;

  //disconnect and delete timer
  disconnect(m_timer, SIGNAL(timeout()), this, SLOT(HandleTimer()));
  delete m_timer;

  //refresh scene + gui one last time
  m_mainWindow->GetGLScene()->updateGL();
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  m_mainWindow->m_mainMenu->CallReset();

}

void
EnvironmentOptions::
MapEnvironment() {
  if(m_threadDone) {
    //stop timer for before regions
    GetVizmo().StopClock("Pre-regions");

    //before thread starts make sure map model exists
    GetVizmo().SetPMPLMap();
    m_mainWindow->m_mainMenu->CallReset();

    //set up timer to redraw and refresh gui
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(HandleTimer()));
    m_timer->start(200);

    //set up thread for mapping the environment
    m_threadDone = false;
    m_thread = new QThread;
    MapEnvironmentWorker* mpsw = new MapEnvironmentWorker;
    mpsw->moveToThread(m_thread);
    m_thread->start();
    connect(m_thread, SIGNAL(started()), mpsw, SLOT(Solve()));
    connect(mpsw, SIGNAL(Finished()), mpsw, SLOT(deleteLater()));
    connect(mpsw, SIGNAL(destroyed()), m_thread, SLOT(quit()));
    connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
    connect(m_thread, SIGNAL(finished()), this, SLOT(ThreadDone()));
  }
}

void
EnvironmentOptions::
CreateActions() {

  //1. Create actions and add them to the map
  QAction* refreshEnv = new QAction(QPixmap(refreshenv), tr("Refresh"), this);
  m_actions["refreshEnv"] = refreshEnv;
  QAction* randEnvColors = new QAction(QPixmap(randEnvIcon), tr("Randomize Environment Colors"), this);
  m_actions["randEnvColors"] = randEnvColors;
  QAction* addObstacle = new QAction(QPixmap(addobstacle), tr("Add Obstacle"), this);
  m_actions["addObstacle"] = addObstacle;
  QAction* deleteObstacle = new QAction(QPixmap(deleteobstacle), tr("Delete Obstacle(s)"), this);
  m_actions["deleteObstacle"] = deleteObstacle;
  QAction* moveObstacle = new QAction(QPixmap(moveobstacle), tr("Move Obstacle(s)"), this);
  m_actions["moveObstacle"] = moveObstacle;
  QAction* duplicateObstacle = new QAction(QPixmap(duplicateobstacle), tr("Duplicate Obstacle(s)"), this);
  m_actions["duplicateObstacle"] = duplicateObstacle;
  QAction* changeBoundary = new QAction(QPixmap(changeboundary), tr("Change Boundary"), this);
  m_actions["changeBoundary"] = changeBoundary;
  QAction* editRobot = new QAction(QPixmap(editrobot), tr("Edit the Robot "), this);
  m_actions["editRobot"] = editRobot;
  QAction* addRegionSphere = new QAction(QPixmap(addsphereregion), tr("Add Spherical Region"), this);
  m_actions["addRegionSphere"] = addRegionSphere;
  QAction* addRegionBox = new QAction(QPixmap(addboxregion), tr("Add Box Region"), this);
  m_actions["addRegionBox"] = addRegionBox;
  QAction* deleteRegion = new QAction(QPixmap(deleteregion), tr("Delete Region"), this);
  m_actions["deleteRegion"] = deleteRegion;
  QAction* makeRegionAttract = new QAction(QPixmap(attractregion), tr("Make Attract Region"), this);
  m_actions["makeRegionAttract"] = makeRegionAttract;
  QAction* makeRegionAvoid = new QAction(QPixmap(avoidregion), tr("Make Avoid Region"), this);
  m_actions["makeRegionAvoid"] = makeRegionAvoid;
  QAction* ugmp = new QAction(QPixmap(mapenv), tr("Map Environment"), this);
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
  m_actions["makeRegionAttract"]->setEnabled(false);
  m_actions["makeRegionAvoid"]->setEnabled(false);
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
  connect(m_actions["makeRegionAttract"], SIGNAL(triggered()), this, SLOT(MakeRegionAttract()));
  connect(m_actions["makeRegionAvoid"], SIGNAL(triggered()), this, SLOT(MakeRegionAvoid()));
  connect(m_actions["ugmp"], SIGNAL(triggered()), this, SLOT(MapEnvironment()));
}

void
EnvironmentOptions::
SetUpCustomSubmenu() {
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
  m_submenu->addAction(m_actions["makeRegionAttract"]);
  m_submenu->addAction(m_actions["makeRegionAvoid"]);
  m_submenu->addAction(m_actions["ugmp"]);
  m_obstacleMenu->setEnabled(false);
}

void
EnvironmentOptions::
SetUpToolbar() {
  m_toolbar = new QToolBar(m_mainWindow);
  m_toolbar->addAction(m_actions["randEnvColors"]);
  m_toolbar->addAction(m_actions["addRegionSphere"]);
  m_toolbar->addAction(m_actions["addRegionBox"]);
  m_toolbar->addAction(m_actions["deleteRegion"]);
  m_toolbar->addAction(m_actions["makeRegionAttract"]);
  m_toolbar->addAction(m_actions["makeRegionAvoid"]);
  m_toolbar->addAction(m_actions["ugmp"]);
}

void
EnvironmentOptions::
SetUpToolTab() {
  vector<string> buttonList;

  //row 1 & 2 - env modification tools
  buttonList.push_back("addObstacle");
  buttonList.push_back("deleteObstacle");
  buttonList.push_back("moveObstacle");
  buttonList.push_back("duplicateObstacle");
  buttonList.push_back("changeBoundary");
  buttonList.push_back("editRobot");
  buttonList.push_back("_separator_");

  //row 3 & 4 - region tools
  buttonList.push_back("addRegionSphere");
  buttonList.push_back("addRegionBox");
  buttonList.push_back("deleteRegion");
  buttonList.push_back("makeRegionAttract");
  buttonList.push_back("makeRegionAvoid");
  buttonList.push_back("ugmp");
  buttonList.push_back("_separator_");

  //row 5 - general env tools
  buttonList.push_back("randEnvColors");

  CreateToolTab(buttonList);
}

void
EnvironmentOptions::
Reset() {
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
  m_actions["makeRegionAttract"]->setEnabled(true);
  m_actions["makeRegionAvoid"]->setEnabled(true);
  m_actions["ugmp"]->setEnabled(true);
  m_obstacleMenu->setEnabled(true);
}

void
EnvironmentOptions::
SetHelpTips() {
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
  m_actions["makeRegionAttract"]->setWhatsThis(tr("Change a region to attract"));
  m_actions["makeRegionAvoid"]->setWhatsThis(tr("Change a region to avoid"));
  m_actions["ugmp"]->setWhatsThis(tr("Map an environment using region strategy"));
}

void
MapEnvironmentWorker::
Solve() {
  //clear any map and path currently loaded
  //delete GetVizmo().GetMap();
  //delete GetVizmo().GetPath();

  //call function somewhere to spark the UG strategy
  GetVizmo().Solve("regions");
  emit Finished();
}

