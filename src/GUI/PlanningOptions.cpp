#include "PlanningOptions.h"

#include "GLWidget.h"
#include "MainMenu.h"
#include "MainWindow.h"
#include "ModelSelectionWidget.h"

#include "Utilities/IO.h"

#include "Models/EnvModel.h"
#include "Models/RegionBoxModel.h"
#include "Models/RegionBox2DModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/RegionSphere2DModel.h"
#include "Models/RobotModel.h"
#include "Models/QueryModel.h"
#include "Models/UserPathModel.h"
#include "Models/Vizmo.h"

#include "PHANToM/Manager.h"

#include "Icons/AddSphereRegion.xpm"
#include "Icons/AddBoxRegion.xpm"
#include "Icons/DeleteRegion.xpm"
#include "Icons/AttractRegion.xpm"
#include "Icons/AvoidRegion.xpm"
#include "Icons/MapEnv.xpm"
#include "Icons/UserPath.xpm"
#include "Icons/DeleteUserPath.xpm"
#include "Icons/PrintUserPath.xpm"

PlanningOptions::
PlanningOptions(QWidget* _parent, MainWindow* _mainWindow) :
  OptionsBase(_parent, _mainWindow), m_regionsStarted(false),
  m_threadDone(true), m_thread(NULL), m_userPathCount(0) {

    CreateActions();
    SetUpCustomSubmenu();
    SetUpToolTab();
    SetHelpTips();
  }

void
PlanningOptions::
CreateActions() {
  // 1, Create Actions and add them to the map
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
  QAction* mapEnv = new QAction(QPixmap(mapenv), tr("Map Environment or Path"), this);
  m_actions["mapEnv"] = mapEnv;

  m_actions["addUserPathMouse"] = new QAction(QPixmap(adduserpath),
      tr("Add User Path with Mouse"), this);
  m_actions["addUserPathHaptic"] = new QAction(QPixmap(adduserpath),
      tr("Add User Path with Haptics"), this);
  m_actions["deleteUserPath"] = new QAction(QPixmap(deleteuserpath),
      tr("Delete User Path"), this);
  m_actions["printUserPath"] = new QAction(QPixmap(printuserpath),
      tr("Print User Path"), this);

  // 2. Set other specifications
  m_actions["addRegionSphere"]->setEnabled(false);
  m_actions["addRegionBox"]->setEnabled(false);
  m_actions["deleteRegion"]->setEnabled(false);
  m_actions["makeRegionAttract"]->setEnabled(false);
  m_actions["makeRegionAvoid"]->setEnabled(false);
  m_actions["mapEnv"]->setEnabled(false);
  m_actions["addUserPathMouse"]->setEnabled(false);
  m_actions["addUserPathHaptic"]->setEnabled(false);
  m_actions["deleteUserPath"]->setEnabled(false);
  m_actions["printUserPath"]->setEnabled(false);

  // 3. Make connections
  connect(m_actions["addRegionSphere"], SIGNAL(triggered()), this, SLOT(AddRegionSphere()));
  connect(m_actions["addRegionBox"], SIGNAL(triggered()), this, SLOT(AddRegionBox()));
  connect(m_actions["deleteRegion"], SIGNAL(triggered()), this, SLOT(DeleteRegion()));
  connect(m_actions["makeRegionAttract"], SIGNAL(triggered()), this, SLOT(MakeRegionAttract()));
  connect(m_actions["makeRegionAvoid"], SIGNAL(triggered()), this, SLOT(MakeRegionAvoid()));
  connect(m_actions["mapEnv"], SIGNAL(triggered()),
      this, SLOT(MapEnvironment()));
  connect(m_actions["addUserPathMouse"], SIGNAL(triggered()),
      this, SLOT(AddUserPath()));
  connect(m_actions["addUserPathHaptic"], SIGNAL(triggered()),
      this, SLOT(AddUserPath()));
  connect(m_actions["deleteUserPath"], SIGNAL(triggered()),
      this, SLOT(DeleteUserPath()));
  connect(m_actions["printUserPath"], SIGNAL(triggered()),
      this, SLOT(PrintUserPath()));
}

void PlanningOptions::
SetUpCustomSubmenu() {

  m_submenu = new QMenu("Planning", this);

  m_addRegionMenu = new QMenu("Add Region", this);
  m_addRegionMenu->addAction(m_actions["addRegionSphere"]);
  m_addRegionMenu->addAction(m_actions["addRegionBox"]);
  m_submenu->addMenu(m_addRegionMenu);

  m_regionPropertiesMenu = new QMenu("Region Properties", this);
  m_regionPropertiesMenu->addAction(m_actions["makeRegionAttract"]);
  m_regionPropertiesMenu->addAction(m_actions["makeRegionAvoid"]);
  m_submenu->addMenu(m_regionPropertiesMenu);

  m_submenu->addAction(m_actions["deleteRegion"]);

  m_submenu->addAction(m_actions["mapEnv"]);

  m_pathsMenu = new QMenu("User Paths", this);
  m_pathsMenu->addAction(m_actions["addUserPathMouse"]);
  m_pathsMenu->addAction(m_actions["addUserPathHaptic"]);
  m_pathsMenu->addAction(m_actions["deleteUserPath"]);
  m_pathsMenu->addAction(m_actions["printUserPath"]);

  m_submenu->addMenu(m_pathsMenu);

  m_pathsMenu->setEnabled(false);
  m_addRegionMenu->setEnabled(false);
  m_regionPropertiesMenu->setEnabled(false);
}

void
PlanningOptions::
SetUpToolbar() {
  m_toolbar = new QToolBar(m_mainWindow);

  m_toolbar->addAction(m_actions["addUserPathMouse"]);
  m_toolbar->addAction(m_actions["addUserPathHaptic"]);
  m_toolbar->addAction(m_actions["deleteUserPath"]);
  m_toolbar->addAction(m_actions["printUserPath"]);
}

void
PlanningOptions::
SetUpToolTab() {

  vector<string> buttonList;

  buttonList.push_back("addUserPathMouse");
  buttonList.push_back("addUserPathHaptic");
  buttonList.push_back("deleteUserPath");
  buttonList.push_back("printUserPath");

  buttonList.push_back("_separator_");

  buttonList.push_back("addRegionBox");
  buttonList.push_back("addRegionSphere");
  buttonList.push_back("deleteRegion");

  buttonList.push_back("makeRegionAttract");
  buttonList.push_back("makeRegionAvoid");
  buttonList.push_back("_separator_");

  buttonList.push_back("mapEnv");

  CreateToolTab(buttonList);
}

void
PlanningOptions::
Reset() {
  m_actions["addRegionSphere"]->setEnabled(true);
  m_actions["addRegionBox"]->setEnabled(true);
  m_actions["deleteRegion"]->setEnabled(true);
  m_actions["makeRegionAttract"]->setEnabled(true);
  m_actions["makeRegionAvoid"]->setEnabled(true);
  m_actions["mapEnv"]->setEnabled(true);
  m_actions["addUserPathMouse"]->setEnabled(true);
  if(Haptics::UsingPhantom())
    m_actions["addUserPathHaptic"]->setEnabled(true);
  m_actions["deleteUserPath"]->setEnabled(true);
  m_actions["printUserPath"]->setEnabled(true);
  m_pathsMenu->setEnabled(true);
  m_addRegionMenu->setEnabled(true);
  m_regionPropertiesMenu->setEnabled(true);
}

void
PlanningOptions::
SetHelpTips() {
  m_actions["mapEnv"]->setWhatsThis(tr("Map an environment using region strategy, or the current path using path strategy"));
  m_actions["addRegionSphere"]->setWhatsThis(tr("Add a spherical region to aid planner"));
  m_actions["addRegionBox"]->setWhatsThis(tr("Add a box region to aid planner"));
  m_actions["deleteRegion"]->setWhatsThis(tr("Remove a region from the scene"));
  m_actions["makeRegionAttract"]->setWhatsThis(tr("Change a region to attract"));
  m_actions["makeRegionAvoid"]->setWhatsThis(tr("Change a region to avoid"));
  m_actions["addUserPathMouse"]->setWhatsThis(tr("Add an approximate path to aid planner"));
  m_actions["addUserPathHaptic"]->setWhatsThis(tr("Add an approximate path to aid planner"));
  m_actions["deleteUserPath"]->setWhatsThis(tr("Remove an approximate path from the scene"));
  m_actions["printUserPath"]->setWhatsThis(tr("Print selected user path to file"));
}

void
PlanningOptions::
AddRegionBox() {

  if(!m_regionsStarted) {
    GetVizmo().StartClock("Pre-regions");
    m_regionsStarted = true;
  }

  // Create new box region
  RegionModel* r;

  // Check if robot is 2D or 3D, set region to same
  RobotModel* robot = GetVizmo().GetRobot();
  if(robot->IsPlanar())
    r = new RegionBox2DModel();
  else
    r = new RegionBoxModel();

  // Add region to environment
  GetVizmo().GetEnv()->AddNonCommitRegion(r);

  // Set mouse events to current region for GLWidget
  m_mainWindow->GetGLScene()->SetCurrentRegion(r);
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  GetVizmo().GetSelectedModels().clear();
  GetVizmo().GetSelectedModels().push_back(r);
  m_mainWindow->GetModelSelectionWidget()->Select();
}

void
PlanningOptions::
AddRegionSphere() {

  if(!m_regionsStarted) {
    GetVizmo().StartClock("Pre-regions");
    m_regionsStarted = true;
  }

  // Create new sphere region
  RegionModel* r;

  // Check to see if robot is 2D or 3D, set region to the same
  RobotModel* robot = GetVizmo().GetRobot();
  if(robot->IsPlanar())
    r = new RegionSphere2DModel();
  else
    r = new RegionSphereModel();

  // Add region to environment
  GetVizmo().GetEnv()->AddNonCommitRegion(r);

  // Set mouse events to current region for GLWidget
  m_mainWindow->GetGLScene()->SetCurrentRegion(r);
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  GetVizmo().GetSelectedModels().clear();
  GetVizmo().GetSelectedModels().push_back(r);
  m_mainWindow->GetModelSelectionWidget()->Select();
}

void
PlanningOptions::
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
PlanningOptions::
MakeRegionAttract() {
  ChangeRegionType(true);
}

void
PlanningOptions::
MakeRegionAvoid() {
  ChangeRegionType(false);
}

void
PlanningOptions::
ChangeRegionType(bool _attract) {
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  // Alert that only the boundry model should be selected
  if(sel.size() == 1 && (sel[0]->Name() == "Box Region" || sel[0]->Name() == "Sphere Region"
        || sel[0]->Name() == "Box Region 2D" || sel[0]->Name() == "Sphere Region 2D")) {
    RegionModel* r = (RegionModel*)sel[0];
    if(GetVizmo().GetEnv()->IsNonCommitRegion(r)) {
      VDRemoveRegion(r);
      r->SetType(_attract ? RegionModel::ATTRACT : RegionModel::AVOID);
      GetVizmo().GetEnv()->ChangeRegionType(r, _attract);
    }
  }
}

void
PlanningOptions::
HandleTimer() {
  m_mainWindow->GetGLScene()->updateGL();
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
}

void
PlanningOptions::
ThreadDone() {
  m_threadDone = true;

  // Disconnect and delete timer
  disconnect(m_timer, SIGNAL(timeout()), this, SLOT(HandleTimer()));
  delete m_timer;

  // Refresh scene + GUI
  m_mainWindow->GetGLScene()->updateGL();
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  m_mainWindow->m_mainMenu->CallReset();
}

void
PlanningOptions::
MapEnvironment() {
  if(m_threadDone) {
    // Stop timer for before regions
    GetVizmo().StopClock("Pre-regions");

    // Before thread starts, make sure map model exists
    GetVizmo().SetPMPLMap();
    m_mainWindow->m_mainMenu->CallReset();

    // Set up timer to redraw and refresh GUI
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(HandleTimer()));
    m_timer->start(200);

    // Set up thread for mapping the environment
    m_threadDone = false;
    m_thread = new QThread;
    MapEnvironmentWorker* mpsw;
    if(m_mainWindow->GetGLScene()->GetCurrentUserPath())
      mpsw = new MapEnvironmentWorker("paths");
    else
      mpsw = new MapEnvironmentWorker("regions");
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
PlanningOptions::
AddUserPath() {
  QAction* callee = dynamic_cast<QAction*>(sender());
  UserPathModel* p;

  if(callee->text().toStdString() == "Add User Path with Haptics") {
    p = new UserPathModel(m_mainWindow, UserPathModel::Haptic);
    connect(m_mainWindow->GetMainClock(), SIGNAL(timeout()),
        this, SLOT(HapticPathCapture()));
  }
  else
    p = new UserPathModel(m_mainWindow, UserPathModel::Mouse);
  GetVizmo().GetEnv()->AddUserPath(p);

  // set mouse events to current path for GLWidget
  m_mainWindow->GetGLScene()->SetCurrentUserPath(p);
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  GetVizmo().GetSelectedModels().clear();
  GetVizmo().GetSelectedModels().push_back(p);
  m_mainWindow->GetModelSelectionWidget()->Select();
}

void
PlanningOptions::
DeleteUserPath() {
  UserPathModel* p = m_mainWindow->GetGLScene()->GetCurrentUserPath();
  if(p) {
    GetVizmo().GetEnv()->DeleteUserPath(p);
    GetVizmo().GetSelectedModels().clear();
    m_mainWindow->GetModelSelectionWidget()->ResetLists();
    m_mainWindow->GetGLScene()->SetCurrentUserPath(NULL);
  }
}

void
PlanningOptions::
PrintUserPath() {
  UserPathModel* p = m_mainWindow->GetGLScene()->GetCurrentUserPath();
  if(p) {
    ++m_userPathCount;
    string base = GetVizmo().GetEnvFileName();
    ostringstream fileName;
    fileName << base.substr(0, base.size() - 4) << "."
      << m_userPathCount << ".user.path";
    ofstream pos(fileName.str().c_str());
    pos << "Workspace Path" << endl << "1" << endl;
    p->PrintPath(pos);
    pos.close();
  }
}

void
PlanningOptions::
HapticPathCapture() {
  UserPathModel* p = m_mainWindow->GetGLScene()->GetCurrentUserPath();
  if(p) {
    if(!p->IsFinished())
      p->SendToPath(GetVizmo().GetManager()->GetWorldPos());
    else
      disconnect(m_mainWindow->GetMainClock(), SIGNAL(timeout()),
          this, SLOT(HapticPathCapture()));
  }
}

MapEnvironmentWorker::
MapEnvironmentWorker(string _strategyLabel) : QObject(),
    m_strategyLabel(_strategyLabel) {}

void
MapEnvironmentWorker::
Solve() {
  GetVizmo().Solve(m_strategyLabel);
  emit Finished();
}
