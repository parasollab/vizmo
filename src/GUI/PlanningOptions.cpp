#include "PlanningOptions.h"

#include "ChangePlannerDialog.h"
#include "GLWidget.h"
#include "MainMenu.h"
#include "MainWindow.h"
#include "ModelSelectionWidget.h"
#include "NodeEditDialog.h"
#include "RegionSamplerDialog.h"

#include "Models/EnvModel.h"
#include "Models/RegionBoxModel.h"
#include "Models/RegionBox2DModel.h"
#include "Models/RegionModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/RegionSphere2DModel.h"
#include "Models/RobotModel.h"
#include "Models/QueryModel.h"
#include "Models/UserPathModel.h"
#include "Models/Vizmo.h"

#include "PHANToM/Manager.h"

#include "Utilities/IO.h"

#include "Icons/AddSphereRegion.xpm"
#include "Icons/AddBoxRegion.xpm"
#include "Icons/AddNode.xpm"
#include "Icons/AttractRegion.xpm"
#include "Icons/AvoidRegion.xpm"
#include "Icons/DeleteRegion.xpm"
#include "Icons/DeleteUserPath.xpm"
#include "Icons/LoadRegion.xpm"
#include "Icons/DuplicateRegion.xpm"
#include "Icons/MapEnv.xpm"
#include "Icons/PrintUserPath.xpm"
#include "Icons/SaveRegion.xpm"
#include "Icons/UserPath.xpm"
#include "Icons/RecordPath.xpm"


PlanningOptions::
PlanningOptions(QWidget* _parent) : OptionsBase(_parent, "Planning"),
    m_regionsStarted(false), m_cfgStarted(false), m_thread(NULL) {
  CreateActions();
  SetHelpTips();
  SetUpSubmenu();
  SetUpToolTab();
}


PlanningOptions::
~PlanningOptions() {
  HaltMPThread();
}


void
PlanningOptions::
HaltMPThread() {
  /// \bug Ideally we should close
  /// the thread properly, but this is not possible under the current
  /// organization as our thread 'event loop' only runs one time. The actual
  /// loop is outsourced to Vizmo and then again to the corresponding strategy.
  /// Fixing this properly requires that the worker have some way to interrupt
  /// MPStrategy::Run() so that control will return to the worker object. Until
  /// then, we will crash and burn whenever halting a thread prematurely.
  if(m_thread) {
    delete m_thread;
    m_thread = NULL;
  }
}

/*----------------------------- GUI Management -------------------------------*/

void
PlanningOptions::
CreateActions() {
  // 1, Create Actions and add them to the map
  m_actions["addRegionSphere"] = new QAction(QPixmap(addsphereregion),
      tr("Add Spherical Region"), this);
  m_actions["addRegionBox"] = new QAction(QPixmap(addboxregion),
      tr("Add Box Region"), this);
  m_actions["deleteRegion"] = new QAction(QPixmap(deleteregion),
      tr("Delete Region"), this);
  m_actions["makeRegionAttract"] = new QAction(QPixmap(attractregion),
      tr("Make Attract Region"), this);
  m_actions["makeRegionAvoid"] = new QAction(QPixmap(avoidregion),
      tr("Make Avoid Region"), this);
  m_actions["duplicateRegion"] = new QAction(QPixmap(duplicateregion),
      tr("Duplicate Region"), this);
  m_actions["saveRegion"] = new QAction(QPixmap(saveregion),
      tr("Save Region"), this);
  m_actions["loadRegion"] = new QAction(QPixmap(loadregion),
      tr("Load Region"), this);
  m_actions["placeCfgs"] = new QAction(QPixmap(addnode),
      tr("Place Cfgs"), this);

  m_actions["addUserPathMouse"] = new QAction(QPixmap(adduserpath),
      tr("Add User Path with Mouse"), this);
  m_actions["addUserPathCamera"] = new QAction(QPixmap(recordpath),
      tr("Add User Path with Camera"), this);
  m_actions["addUserPathHaptic"] = new QAction(QPixmap(adduserpath),
      tr("Add User Path with Haptics"), this);
  m_actions["deleteUserPath"] = new QAction(QPixmap(deleteuserpath),
      tr("Delete User Path"), this);
  m_actions["printUserPath"] = new QAction(QPixmap(printuserpath),
      tr("Print User Path"), this);
  m_actions["mapEnvironment"] = new QAction(QPixmap(mapenv),
      tr("Map Environment"), this);

  // 2. Set other specifications
  m_actions["addRegionSphere"]->setEnabled(false);
  m_actions["addRegionBox"]->setEnabled(false);
  m_actions["deleteRegion"]->setEnabled(false);
  m_actions["makeRegionAttract"]->setEnabled(false);
  m_actions["makeRegionAvoid"]->setEnabled(false);
  m_actions["duplicateRegion"]->setEnabled(false);
  m_actions["mapEnvironment"]->setEnabled(false);
  m_actions["addUserPathMouse"]->setEnabled(false);
  m_actions["addUserPathCamera"]->setEnabled(false);
  m_actions["addUserPathHaptic"]->setEnabled(false);
  m_actions["deleteUserPath"]->setEnabled(false);
  m_actions["printUserPath"]->setEnabled(false);
  m_actions["saveRegion"]->setEnabled(false);
  m_actions["loadRegion"]->setEnabled(false);
  m_actions["placeCfgs"]->setEnabled(false);

  // 3. Make connections
  connect(m_actions["addRegionSphere"], SIGNAL(triggered()),
      this, SLOT(AddRegionSphere()));
  connect(m_actions["addRegionBox"], SIGNAL(triggered()),
      this, SLOT(AddRegionBox()));
  connect(m_actions["deleteRegion"], SIGNAL(triggered()),
      this, SLOT(DeleteRegion()));
  connect(m_actions["makeRegionAttract"], SIGNAL(triggered()),
      this, SLOT(MakeRegionAttract()));
  connect(m_actions["makeRegionAvoid"], SIGNAL(triggered()),
      this, SLOT(MakeRegionAvoid()));
  connect(m_actions["saveRegion"], SIGNAL(triggered()),
      this, SLOT(SaveRegion()));
  connect(m_actions["loadRegion"], SIGNAL(triggered()),
      this, SLOT(LoadRegion()));
  connect(m_actions["placeCfgs"], SIGNAL(triggered()),
      this, SLOT(PlaceCfg()));
  connect(m_actions["duplicateRegion"], SIGNAL(triggered()),
      this, SLOT(DuplicateRegion()));
  connect(m_actions["mapEnvironment"], SIGNAL(triggered()),
      this, SLOT(MapEnvironment()));
  connect(m_actions["addUserPathMouse"], SIGNAL(triggered()),
      this, SLOT(AddUserPath()));
  connect(m_actions["addUserPathCamera"], SIGNAL(triggered()),
      this, SLOT(AddUserPath()));
  connect(m_actions["addUserPathHaptic"], SIGNAL(triggered()),
      this, SLOT(AddUserPath()));
  connect(m_actions["deleteUserPath"], SIGNAL(triggered()),
      this, SLOT(DeleteUserPath()));
  connect(m_actions["printUserPath"], SIGNAL(triggered()),
      this, SLOT(PrintUserPath()));
}


void
PlanningOptions::
SetHelpTips() {
  m_actions["mapEnvironment"]->setWhatsThis(
      tr("Map an environment using selected Strategy"));
  m_actions["addRegionSphere"]->setWhatsThis(
      tr("Add a spherical region to aid planner"));
  m_actions["addRegionBox"]->setWhatsThis(
      tr("Add a box region to aid planner"));
  m_actions["deleteRegion"]->setWhatsThis(
      tr("Remove a region from the scene"));
  m_actions["makeRegionAttract"]->setWhatsThis(
      tr("Change a region to attract"));
  m_actions["makeRegionAvoid"]->setWhatsThis(
      tr("Change a region to avoid"));
  m_actions["duplicateRegion"]->setWhatsThis(
      tr("Copy of a selected region"));
  m_actions["addUserPathMouse"]->setWhatsThis(
      tr("Add an approximate path to aid planner"));
  m_actions["addUserPathCamera"]->setWhatsThis(
      tr("Add an approximate path to aid planner"));
  m_actions["addUserPathHaptic"]->setWhatsThis(
      tr("Add an approximate path to aid planner"));
  m_actions["deleteUserPath"]->setWhatsThis(
      tr("Remove an approximate path from the scene"));
  m_actions["printUserPath"]->setWhatsThis(
      tr("Print selected user path to file"));
  m_actions["saveRegion"]->setWhatsThis(
      tr("Saves the regions drawn in the scene"));
  m_actions["loadRegion"]->setWhatsThis(
      tr("Loads saved regions to the scene"));
}


void
PlanningOptions::
SetUpSubmenu() {
  m_submenu = new QMenu(m_label, this);

  m_addRegionMenu = new QMenu("Add Region", this);
  m_addRegionMenu->addAction(m_actions["addRegionSphere"]);
  m_addRegionMenu->addAction(m_actions["addRegionBox"]);
  m_submenu->addMenu(m_addRegionMenu);

  m_regionPropertiesMenu = new QMenu("Region Properties", this);
  m_regionPropertiesMenu->addAction(m_actions["makeRegionAttract"]);
  m_regionPropertiesMenu->addAction(m_actions["makeRegionAvoid"]);
  m_regionPropertiesMenu->addAction(m_actions["duplicateRegion"]);
  m_submenu->addMenu(m_regionPropertiesMenu);

  m_submenu->addAction(m_actions["placeCfgs"]);

  m_submenu->addAction(m_actions["saveRegion"]);
  m_submenu->addAction(m_actions["loadRegion"]);
  m_submenu->addAction(m_actions["deleteRegion"]);
  m_submenu->addAction(m_actions["mapEnvironment"]);

  m_pathsMenu = new QMenu("User Paths", this);
  m_pathsMenu->addAction(m_actions["addUserPathMouse"]);
  m_pathsMenu->addAction(m_actions["addUserPathCamera"]);
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
SetUpToolTab() {
  vector<string> buttonList;

  buttonList.push_back("addUserPathMouse");
  buttonList.push_back("addUserPathCamera");
  buttonList.push_back("addUserPathHaptic");
  buttonList.push_back("deleteUserPath");
  buttonList.push_back("printUserPath");

  buttonList.push_back("_separator_");

  buttonList.push_back("addRegionBox");
  buttonList.push_back("addRegionSphere");
  buttonList.push_back("duplicateRegion");

  buttonList.push_back("makeRegionAttract");
  buttonList.push_back("makeRegionAvoid");
  buttonList.push_back("deleteRegion");

  buttonList.push_back("saveRegion");
  buttonList.push_back("loadRegion");
  buttonList.push_back("placeCfgs");

  buttonList.push_back("_separator_");

  buttonList.push_back("mapEnvironment");

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
  m_actions["duplicateRegion"]->setEnabled(true);
  m_actions["mapEnvironment"]->setEnabled(true);
  m_actions["addUserPathMouse"]->setEnabled(true);
  m_actions["addUserPathCamera"]->setEnabled(true);
  if(Haptics::UsingPhantom())
    m_actions["addUserPathHaptic"]->setEnabled(true);
  m_actions["deleteUserPath"]->setEnabled(true);
  m_actions["printUserPath"]->setEnabled(true);
  m_actions["saveRegion"]->setEnabled(true);
  m_actions["loadRegion"]->setEnabled(true);
  m_actions["placeCfgs"]->setEnabled(true);
  m_pathsMenu->setEnabled(true);
  m_addRegionMenu->setEnabled(true);
  m_regionPropertiesMenu->setEnabled(true);
}
/*-------------------------- Region Functions --------------------------------*/

void
PlanningOptions::
AddRegionBox() {
  /// Automatically determines whether a 2D or 3D box is needed by checking
  /// whether the robot is planar. Additionally, the pre-regions timer will be
  /// started if it has not been already.
  if(!m_regionsStarted) {
    GetVizmo().StartClock("Pre-regions");
    m_regionsStarted = true;
  }

  // Create new box region
  RegionModelPtr r;

  // Check if robot is 2D or 3D, set region to same
  RobotModel* robot = GetVizmo().GetRobot();
  if(robot->IsPlanar())
    r = RegionModelPtr(new RegionBox2DModel());
  else
    r = RegionModelPtr(new RegionBoxModel());

  // Add region to environment
  GetVizmo().GetEnv()->AddNonCommitRegion(r);

  // Set mouse events to current region for GLWidget
  GetMainWindow()->GetGLWidget()->SetCurrentRegion(r);
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
  GetVizmo().GetSelectedModels().clear();
  GetVizmo().GetSelectedModels().push_back(r.get());
  GetMainWindow()->GetModelSelectionWidget()->Select();
}


void
PlanningOptions::
AddRegionSphere() {
  /// Automatically determines whether a 2D or 3D sphere is needed by checking
  /// whether the robot is planar. Additionally, the pre-regions timer will be
  /// started if it has not been already.
  if(!m_regionsStarted) {
    GetVizmo().StartClock("Pre-regions");
    m_regionsStarted = true;
  }
  // Create new sphere region
  RegionModelPtr r;

  // Check to see if robot is 2D or 3D, set region to the same
  RobotModel* robot = GetVizmo().GetRobot();
  if(robot->IsPlanar())
    r = RegionModelPtr(new RegionSphere2DModel());
  else
    r = RegionModelPtr(new RegionSphereModel());

  // Add region to environment
  GetVizmo().GetEnv()->AddNonCommitRegion(r);

  // Set mouse events to current region for GLWidget
  GetMainWindow()->GetGLWidget()->SetCurrentRegion(r);
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
  GetVizmo().GetSelectedModels().clear();
  GetVizmo().GetSelectedModels().push_back(r.get());
  GetMainWindow()->GetModelSelectionWidget()->Select();
}

void
PlanningOptions::
PlaceCfg() {
  if(!m_cfgStarted) {
    GetVizmo().StartClock("Pre-cfgs");
    m_cfgStarted = true;
  }

  GetVizmo().SetPMPLMap();

  NodeEditDialog* ned = new NodeEditDialog(GetMainWindow(), "New Node");
  GetMainWindow()->ShowDialog(ned);
}

void
PlanningOptions::
DuplicateRegion() {
  /// Requires that a single region is selected. The new region will be offset
  /// slightly from the original to make manipulation easier.
  if(SingleRegionSelected()) {
    vector<Model*>& sel = GetVizmo().GetSelectedModels();
    bool regionFound = false;
    typedef vector<Model*>::iterator SIT;
    for(SIT sit = sel.begin(); sit != sel.end(); ++sit) {
      if((*sit)->Name().find("Region")) {

        RegionModelPtr r;
        double dist;
        RegionModel::Shape shape = ((RegionModel*)(*sit))->GetShape();
        if(shape == RegionModel::BOX)
          r = RegionModelPtr(
              new RegionBoxModel(*static_cast<RegionBoxModel*>(*sit)));
        else if(shape == RegionModel::BOX2D)
          r = RegionModelPtr(
              new RegionBox2DModel(*static_cast<RegionBox2DModel*>(*sit)));
        else if(shape == RegionModel::SPHERE)
          r = RegionModelPtr(
              new RegionSphereModel(*static_cast<RegionSphereModel*>(*sit)));
        else if(shape == RegionModel::SPHERE2D)
          r = RegionModelPtr(
              new RegionSphere2DModel(*static_cast<RegionSphere2DModel*>(*sit)));
        if(r) {
          regionFound = true;
          dist = r->GetLongLength();
          r->SetType(RegionModel::NONCOMMIT);
          Camera* c = GetMainWindow()->GetGLWidget()->GetCurrentCamera();
          Vector3d delta = -(c->GetWindowX() + c->GetWindowY() + c->GetWindowZ());
          delta.selfNormalize();
          delta *= dist/3;
          r->ApplyOffset(delta);
          GetVizmo().GetEnv()->AddNonCommitRegion(r);
          GetMainWindow()->GetGLWidget()->SetCurrentRegion(r);
          GetMainWindow()->GetModelSelectionWidget()->ResetLists();
          sel.clear();
          sel.push_back(r.get());
          GetMainWindow()->GetModelSelectionWidget()->Select();
          break;
        }
      }
    }
    if(!regionFound)
      GetMainWindow()->AlertUser("Region not selected");
  }
}

void
PlanningOptions::
DeleteRegion() {
  /// Requires that a single region is selected.
  if(SingleRegionSelected()) {
    RegionModelPtr r = GetMainWindow()->GetGLWidget()->GetCurrentRegion();
    if(r) {
      r->SetRenderMode(INVISIBLE_MODE);
      GetVizmo().GetEnv()->DeleteRegion(r);
      GetVizmo().GetSelectedModels().clear();
      GetMainWindow()->GetModelSelectionWidget()->ResetLists();
      GetMainWindow()->GetGLWidget()->SetCurrentRegion();
    }
  }
}


void
PlanningOptions::
MakeRegionAttract() {
  /// Requires that a single non-commit region is selected.
  if(SingleRegionSelected()) {
    ChangeRegionType(true);
    ChooseSamplerStrategy();
  }
}


void
PlanningOptions::
MakeRegionAvoid() {
  /// Requires that a single non-commit region is selected.
  if(SingleRegionSelected())
    ChangeRegionType(false);
}


void
PlanningOptions::
ChangeRegionType(bool _attract) {
  /// Requires that a single region is selected. Can only change the type of
  /// \a non-commit regions to either \a attract or \a avoid.
  if(SingleRegionSelected()) {
    vector<Model*>& sel = GetVizmo().GetSelectedModels();
    EnvModel* env = GetVizmo().GetEnv();
    RegionModelPtr r = env->GetRegion(sel[0]);
    if(env->IsNonCommitRegion(r)) {
      VDRemoveRegion(r.get());
      r->SetType(_attract ? RegionModel::ATTRACT : RegionModel::AVOID);
      env->ChangeRegionType(r, _attract);
    }
  }
}


void
PlanningOptions::
SaveRegion() {
  /// The output file extension will be <tt>.regions</tt>.
  QString fn = QFileDialog::getSaveFileName(this, "Choose an region file",
      GetMainWindow()->GetLastDir(), "Region File (*.regions)");

  if(!fn.isEmpty()) {
    string filename = fn.toStdString();
    ofstream ofs(filename.c_str());
    ofs << "#####RegionFile#####" << endl;

    EnvModel* env = GetVizmo().GetEnv();
    const vector<RegionModelPtr>& nonCommit = env->GetNonCommitRegions();
    const vector<RegionModelPtr>& attractRegion = env->GetAttractRegions();
    const vector<RegionModelPtr>& avoidRegion = env->GetAvoidRegions();

    typedef vector<RegionModelPtr>::const_iterator CRIT;
    for(CRIT crit = nonCommit.begin(); crit != nonCommit.end(); ++crit)
      (*crit)->OutputDebugInfo(ofs);
    for(CRIT crit = attractRegion.begin(); crit != attractRegion.end(); ++crit)
      (*crit)->OutputDebugInfo(ofs);
    for(CRIT crit = avoidRegion.begin(); crit != avoidRegion.end(); ++crit)
      (*crit)->OutputDebugInfo(ofs);

    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
  GetMainWindow()->GetGLWidget()->updateGL();
}


void
PlanningOptions::
LoadRegion() {
  /// The input file extension must be <tt>.regions</tt>.
  QString fn = QFileDialog::getOpenFileName(this, "Choose an region file",
      GetMainWindow()->GetLastDir(), "Region File (*.regions)");

  if(!fn.isEmpty()) {

    string filename = fn.toStdString();
    ifstream ifs(filename.c_str());

    string line;
    getline(ifs,line);

    while(getline(ifs,line)) {

      istringstream iss(line);

      RegionModelPtr _mod;

      int tempType;
      iss >> tempType;

      string modelShapeName;
      iss >> modelShapeName;

      RegionModel::Type modelType = static_cast<RegionModel::Type>(tempType);

      if(modelShapeName == "BOX") {

        Point3d min, max;
        iss >> min >> max;

        pair<double, double> xPair(min[0], max[0]);
        pair<double, double> yPair(min[1], max[1]);
        pair<double, double> zPair(min[2], max[2]);

        _mod = RegionModelPtr(new RegionBoxModel(xPair, yPair, zPair));
        _mod->SetType(modelType);
        _mod->ChangeColor();
      }
      else if(modelShapeName == "BOX2D") {

        Point2d min, max;
        iss >> min >> max;

        pair<double, double> xPair(min[0], max[0]);
        pair<double, double> yPair(min[1], max[1]);

        _mod = RegionModelPtr(new RegionBox2DModel(xPair, yPair));
        _mod->SetType(modelType);
        _mod->ChangeColor();

      }
      else if(modelShapeName == "SPHERE") {

        Point3d tempCenter;
        iss >> tempCenter;

        double radius = -1;
        iss >> radius;

        _mod = RegionModelPtr(new RegionSphereModel(tempCenter, radius));
        _mod->SetType(modelType);
        _mod->ChangeColor();
      }
      else if(modelShapeName == "SPHERE2D") {

        Point3d tempCenter;
        iss >> tempCenter;

        double radius = -1;
        iss >> radius;

        _mod = RegionModelPtr(new RegionSphere2DModel(tempCenter, radius));
        _mod->SetType(modelType);
        _mod->ChangeColor();
      }

      if(_mod != NULL) {
        //Checks what type of region then adds
        EnvModel* env = GetVizmo().GetEnv();
        if(_mod->GetType() == 0)
          env->AddAttractRegion(_mod);
        else if(_mod->GetType() == 1)
          env->AddAvoidRegion(_mod);
        else if(_mod->GetType() == 2)
          env->AddNonCommitRegion(_mod);
      }
    }
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
  else
    GetMainWindow()->statusBar()->showMessage("Loading aborted", 2000);
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
  GetMainWindow()->GetGLWidget()->updateGL();
}


void
PlanningOptions::
ChooseSamplerStrategy() {
  /// Requires that a single region is selected.
  if(SingleRegionSelected()) {
    const vector<string>& samplers = GetVizmo().GetLoadedSamplers();

    if(samplers.size() == 1)
      GetMainWindow()->GetGLWidget()->GetCurrentRegion()->
        SetSampler(samplers.front());
    else {
      RegionSamplerDialog rsDialog(samplers, GetMainWindow());

      if(rsDialog.exec() != QDialog::Accepted)
        cout << "Sampler selection dialog aborted." << endl << flush;
    }
  }
}


bool
PlanningOptions::
SingleRegionSelected() {
  if(GetMainWindow()->GetGLWidget()->GetCurrentRegion() == NULL) {
    GetMainWindow()->AlertUser("Error: non-region selected!");
    return false;
  }
  if(GetVizmo().GetSelectedModels().size() != 1) {
    GetMainWindow()->AlertUser("Error: multiple objects selected!");
    return false;
  }
  return true;
}

/*------------------------ User Path Functions -------------------------------*/

void
PlanningOptions::
AddUserPath() {
  /// This slot must be connected to a QAction in order to determine the
  /// appropriate path capture method: the sender's text() is used for this
  /// purpose. Unrecognized text() defaults to mouse input.
  QAction* callee = static_cast<QAction*>(sender());
  UserPathModel* p;

  if(callee->text().toStdString() == "Add User Path with Haptics") {
    p = new UserPathModel(UserPathModel::Haptic);
    connect(GetMainWindow()->GetMainClock(), SIGNAL(timeout()),
        this, SLOT(HapticPathCapture()));
  }
  if(callee->text().toStdString() == "Add User Path with Camera") {
    p = new UserPathModel(UserPathModel::CameraPath);
    connect(GetMainWindow()->GetMainClock(), SIGNAL(timeout()),
        this, SLOT(CameraPathCapture()));
  }
  else
    p = new UserPathModel(UserPathModel::Mouse);
  GetVizmo().GetEnv()->AddUserPath(p);

  // set mouse events to current path for GLWidget
  GetMainWindow()->GetGLWidget()->SetCurrentUserPath(p);
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
  GetVizmo().GetSelectedModels().clear();
  GetVizmo().GetSelectedModels().push_back(p);
  GetMainWindow()->GetModelSelectionWidget()->Select();
}


void
PlanningOptions::
DeleteUserPath() {
  UserPathModel* p = GetMainWindow()->GetGLWidget()->GetCurrentUserPath();
  if(p) {
    GetVizmo().GetEnv()->DeleteUserPath(p);
    GetVizmo().GetSelectedModels().clear();
    GetMainWindow()->GetModelSelectionWidget()->ResetLists();
    GetMainWindow()->GetGLWidget()->SetCurrentUserPath(NULL);
  }
}


void
PlanningOptions::
PrintUserPath() {
  /// The output file will be named <tt>(base name).(x).userpath</tt>, where
  /// <tt>(base name)</tt> is the Environment base name and \c (x) is a count of
  /// the number of printed paths.
  static short userPathCount = 0;
  UserPathModel* p = GetMainWindow()->GetGLWidget()->GetCurrentUserPath();
  if(p) {
    ++userPathCount;
    string base = GetVizmo().GetEnvFileName();
    ostringstream fileName;
    fileName << base.substr(0, base.size() - 4) << "."
      << userPathCount << ".userpath";
    ofstream pos(fileName.str().c_str());
    pos << "Workspace Path" << endl << "1" << endl;
    p->PrintPath(pos);
    pos.close();
  }
}


void
PlanningOptions::
HapticPathCapture() {
  /// The position is acquired on each tick of main clock at
  /// MainWindow::GetMainClock(). The UserPathModel handles ending input by
  /// returning false from IsFinished(). Once input collection is ended, this
  /// slot disconnects itself from the main clock on the next call.
  UserPathModel* p = GetMainWindow()->GetGLWidget()->GetCurrentUserPath();
  if(p) {
    if(!p->IsFinished())
      p->SendToPath(GetVizmo().GetManager()->GetWorldPos());
    else
      disconnect(GetMainWindow()->GetMainClock(), SIGNAL(timeout()),
          this, SLOT(HapticPathCapture()));
  }
}


void
PlanningOptions::
CameraPathCapture() {
  /// The camera position is acquired on each tick of main clock at
  /// MainWindow::GetMainClock(). The UserPathModel handles ending input by
  /// returning false from IsFinished(). Once input collection is ended, this
  /// slot disconnects itself from the main clock on the next call.
  UserPathModel* p = GetMainWindow()->GetGLWidget()->GetCurrentUserPath();
  Camera* camera = GetMainWindow()->GetGLWidget()->GetCurrentCamera();
  if(p) {
    if(!p->IsFinished()) {
      p->SendToPath(camera->GetEye());
      if(!p->GetNewPos().IsValid()) {
        p->RewindPos();
        camera->Set(p->GetOldPos().GetPoint(), camera->GetDir());
      }
    }
    else
      disconnect(GetMainWindow()->GetMainClock(), SIGNAL(timeout()),
          this, SLOT(CameraPathCapture()));
  }
}

/*-------------------------- Common Planning Functions -----------------------*/

void
PlanningOptions::
ThreadDone() {
  /// Should be connected to the finish signal of \ref m_thread before executing
  /// a mapping strategy.
  m_thread = NULL;
  m_regionsStarted = false;

  // Refresh scene + GUI
  GetMainWindow()->GetGLWidget()->updateGL();
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
  GetMainWindow()->m_mainMenu->CallReset();
}


void
PlanningOptions::
MapEnvironment() {
  /// The execution of the strategy will be moved to another thread, and only
  /// one such mapping thread can be running at any time.
  if(!m_thread) {

    //Stop pre-planning region timer.
    if(!m_regionsStarted)
      //start-stop when no regions were created
      GetVizmo().StartClock("Pre-regions");
    GetVizmo().StopClock("Pre-regions");

    GetVizmo().StartClock("StrategySelection");
    ChangePlannerDialog cpDialog(GetMainWindow());

    if(!cpDialog.exec()) {
      GetVizmo().StopClock("StrategySelection");
      GetVizmo().AdjustClock("Pre-regions", "StrategySelection", "-");
      return;
    }

    string slabel = cpDialog.GetPlanner();

    //Verify that map model exist
    GetVizmo().SetPMPLMap();
    GetMainWindow()->m_mainMenu->CallReset();

    //Set up thread for execution
    m_thread = new QThread;
    MapEnvironmentWorker* mpsw = new MapEnvironmentWorker(slabel);
    mpsw->moveToThread(m_thread);
    connect(m_thread, SIGNAL(started()), mpsw, SLOT(Solve()));
    connect(m_thread, SIGNAL(finished()), mpsw, SLOT(deleteLater()));
    connect(m_thread, SIGNAL(finished()), this, SLOT(ThreadDone()));
    connect(mpsw, SIGNAL(Finished()), mpsw, SLOT(deleteLater()));
    connect(mpsw, SIGNAL(destroyed()), m_thread, SLOT(quit()));
    m_thread->start();
  }
}

/*----------------------------MapEnvironmentWorker----------------------------*/

MapEnvironmentWorker::
MapEnvironmentWorker(string _strategyLabel)
    : QObject(), m_strategyLabel(_strategyLabel) {}


void
MapEnvironmentWorker::
Solve() {
  GetVizmo().Solve(m_strategyLabel);
  emit Finished();
}
