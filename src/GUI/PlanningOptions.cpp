#include "PlanningOptions.h"

#include "ChangePlannerDialog.h"
#include "GLWidget.h"
#include "MainMenu.h"
#include "MainWindow.h"
#include "ModelSelectionWidget.h"
#include "RegionSamplerDialog.h"

#include "Utilities/IO.h"

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

#include "Icons/AddSphereRegion.xpm"
#include "Icons/AddBoxRegion.xpm"
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

// Sets up toolbar
void
PlanningOptions::
SetUpCustomSubmenu() {

  m_submenu = new QMenu("Planning", this);

  m_addRegionMenu = new QMenu("Add Region", this);
  m_addRegionMenu->addAction(m_actions["addRegionSphere"]);
  m_addRegionMenu->addAction(m_actions["addRegionBox"]);
  m_submenu->addMenu(m_addRegionMenu);

  m_regionPropertiesMenu = new QMenu("Region Properties", this);
  m_regionPropertiesMenu->addAction(m_actions["makeRegionAttract"]);
  m_regionPropertiesMenu->addAction(m_actions["makeRegionAvoid"]);
  m_regionPropertiesMenu->addAction(m_actions["duplicateRegion"]);
  m_submenu->addMenu(m_regionPropertiesMenu);

  m_submenu->addAction(m_actions["saveRegion"]);
  m_submenu->addAction(m_actions["loadRegion"]);
  m_submenu->addAction(m_actions["deleteRegion"]);
  m_submenu->addAction(m_actions["MapEvironment"]);

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
SetUpToolbar() {
  m_toolbar = new QToolBar(m_mainWindow);
  m_toolbar->addAction(m_actions["addUserPathMouse"]);
  m_toolbar->addAction(m_actions["addUserPathCamera"]);
  m_toolbar->addAction(m_actions["addUserPathHaptic"]);
  m_toolbar->addAction(m_actions["deleteUserPath"]);
  m_toolbar->addAction(m_actions["printUserPath"]);

}

// Places the buttons in order
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
  m_pathsMenu->setEnabled(true);
  m_addRegionMenu->setEnabled(true);
  m_regionPropertiesMenu->setEnabled(true);
}

// Setting up "whats this" helper message
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
  m_mainWindow->GetGLWidget()->SetCurrentRegion(r);
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
  m_mainWindow->GetGLWidget()->SetCurrentRegion(r);
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  GetVizmo().GetSelectedModels().clear();
  GetVizmo().GetSelectedModels().push_back(r);
  m_mainWindow->GetModelSelectionWidget()->Select();
}

void
PlanningOptions::
DeleteRegion() {
  RegionModel* r = m_mainWindow->GetGLWidget()->GetCurrentRegion();
  if(r) {
    GetVizmo().GetEnv()->DeleteRegion(r);
    GetVizmo().GetSelectedModels().clear();
    m_mainWindow->GetModelSelectionWidget()->ResetLists();
    m_mainWindow->GetGLWidget()->SetCurrentRegion(NULL);
  }
}

void
PlanningOptions::
ChooseSamplerStrategy() {
  const vector<string>& samplers = GetVizmo().GetLoadedSamplers();

  RegionSamplerDialog rsDialog(samplers, m_mainWindow);

  //TODO allow canceling from region sampler dialog.
  if(rsDialog.exec() != QDialog::Accepted)
    cout << "The Dialog Doesnt work" << endl << flush;
}

void
PlanningOptions::
MakeRegionAttract() {
  ChangeRegionType(true);
  ChooseSamplerStrategy();
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
DuplicateRegion() {
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  bool regionFound = false;
  typedef vector<Model*>::iterator SIT;
  for(SIT sit = sel.begin(); sit != sel.end(); ++sit) {
    if((*sit)->Name().find("Region")) {

      RegionModel* r = NULL;
      double dist;
      RegionModel::Shape shape = ((RegionModel*)(*sit))->GetShape();
      if(shape == RegionModel::BOX) {
        r = new RegionBoxModel(*static_cast<RegionBoxModel*>(*sit));
      }
      else if(shape == RegionModel::BOX2D) {
        r = new RegionBox2DModel(*static_cast<RegionBox2DModel*>(*sit));
      }
      else if(shape == RegionModel::SPHERE ) {
        r = new RegionSphereModel(*static_cast<RegionSphereModel*>(*sit));
      }
      else if(shape == RegionModel::SPHERE2D) {
        r = new RegionSphere2DModel(*static_cast<RegionSphere2DModel*>(*sit));
      }
      if(r) {
        regionFound = true;
        dist = r->GetLongLength();
        r->SetType(RegionModel::NONCOMMIT);
        Camera* c = m_mainWindow->GetGLWidget()->GetCurrentCamera();
        Vector3d delta = -(c->GetWindowX() + c->GetWindowY() + c->GetWindowZ());
        delta.selfNormalize();
        delta *= dist/3;
        r->ApplyOffset(delta);
        GetVizmo().GetEnv()->AddNonCommitRegion(r);
        m_mainWindow->GetModelSelectionWidget()->ResetLists();
        sel.clear();
        sel.push_back(r);
        break;
      }
    }
  }
  if(!regionFound)
    GetMainWindow()->AlertUser("Region not selected");
}

void
PlanningOptions::
ThreadDone() {
  m_threadDone = true;
  m_regionsStarted = false;

  // Refresh scene + GUI
  m_mainWindow->GetGLWidget()->updateGL();
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  m_mainWindow->m_mainMenu->CallReset();
}

void
PlanningOptions::
MapEnvironment() {
  if(m_threadDone) {

    ChangePlannerDialog cpDialog(m_mainWindow);

    if(!cpDialog.exec())
      return;

    string slabel = cpDialog.GetPlanner();

    //Stop timer for before regions
    if(!m_regionsStarted)
      //start-stop when no regions were created
      GetVizmo().StartClock("Pre-Regions");
    GetVizmo().StopClock("Pre-Regions");

    //Verify that map model exist
    GetVizmo().SetPMPLMap();
    m_mainWindow->m_mainMenu->CallReset();

    //Set up thread for execution
    m_threadDone = false;
    m_thread = new QThread;
    MapEnvironmentWorker* mpsw;
    mpsw = new MapEnvironmentWorker(slabel);
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
  //this slot must be connected to a QAction
  QAction* callee = static_cast<QAction*>(sender());
  UserPathModel* p;

  if(callee->text().toStdString() == "Add User Path with Haptics") {
    p = new UserPathModel(m_mainWindow, UserPathModel::Haptic);
    connect(m_mainWindow->GetMainClock(), SIGNAL(timeout()),
        this, SLOT(HapticPathCapture()));
  }
  if(callee->text().toStdString() == "Add User Path with Camera") {
    p = new UserPathModel(m_mainWindow, UserPathModel::CameraPath);
    connect(m_mainWindow->GetMainClock(), SIGNAL(timeout()),
        this, SLOT(CameraPathCapture()));
  }
  else
    p = new UserPathModel(m_mainWindow, UserPathModel::Mouse);
  GetVizmo().GetEnv()->AddUserPath(p);

  // set mouse events to current path for GLWidget
  m_mainWindow->GetGLWidget()->SetCurrentUserPath(p);
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  GetVizmo().GetSelectedModels().clear();
  GetVizmo().GetSelectedModels().push_back(p);
  m_mainWindow->GetModelSelectionWidget()->Select();
}

void
PlanningOptions::
DeleteUserPath() {
  UserPathModel* p = m_mainWindow->GetGLWidget()->GetCurrentUserPath();
  if(p) {
    GetVizmo().GetEnv()->DeleteUserPath(p);
    GetVizmo().GetSelectedModels().clear();
    m_mainWindow->GetModelSelectionWidget()->ResetLists();
    m_mainWindow->GetGLWidget()->SetCurrentUserPath(NULL);
  }
}

void
PlanningOptions::
PrintUserPath() {
  UserPathModel* p = m_mainWindow->GetGLWidget()->GetCurrentUserPath();
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
  UserPathModel* p = m_mainWindow->GetGLWidget()->GetCurrentUserPath();
  if(p) {
    if(!p->IsFinished())
      p->SendToPath(GetVizmo().GetManager()->GetWorldPos());
    else
      disconnect(m_mainWindow->GetMainClock(), SIGNAL(timeout()),
          this, SLOT(HapticPathCapture()));
  }
}

void
PlanningOptions::
CameraPathCapture() {
  UserPathModel* p = m_mainWindow->GetGLWidget()->GetCurrentUserPath();
  Camera* camera = m_mainWindow->GetGLWidget()->GetCurrentCamera();
  if(p) {
    if(!p->IsFinished()) {
      p->SendToPath(camera->GetEye());
      if (p->m_checkCollision && !p->GetNewPos().IsValid()) {
        p->RewindPos();
        camera->Set(p->GetOldPos().GetPoint(), camera->GetDir());
      }
    }
    else
      disconnect(m_mainWindow->GetMainClock(), SIGNAL(timeout()),
          this, SLOT(CameraPathCapture()));
  }
}

void
PlanningOptions::
SaveRegion() {
  QString fn = QFileDialog::getSaveFileName(this,
      "Choose an region file", QString::null,"Region File (*.regions)");
  if(!fn.isEmpty()) {

    string filename = fn.toStdString();
    ofstream ofs(filename.c_str());
    ofs << "#####RegionFile#####" << endl;

    const vector<RegionModel*>& nonCommit = GetVizmo().GetEnv()->GetNonCommitRegions();
    const vector<RegionModel*>& attractRegion = GetVizmo().GetEnv()->GetAttractRegions();
    const vector<RegionModel*>& avoidRegion = GetVizmo().GetEnv()->GetAvoidRegions();

    typedef vector<RegionModel*>::const_iterator CRIT;
    for(CRIT crit = nonCommit.begin(); crit != nonCommit.end(); ++crit)
      (*crit)->OutputDebugInfo(ofs);
    for(CRIT crit = attractRegion.begin(); crit != attractRegion.end(); ++crit)
      (*crit)->OutputDebugInfo(ofs);
    for(CRIT crit = avoidRegion.begin(); crit != avoidRegion.end(); ++crit)
      (*crit)->OutputDebugInfo(ofs);
  }
  m_mainWindow->GetGLWidget()->updateGL();
}

void
PlanningOptions::
LoadRegion() {
  QString fn = QFileDialog::getOpenFileName(this,
      "Choose an region file", QString::null,"Region File (*.regions)");

  if(!fn.isEmpty()) {

    string filename = fn.toStdString();
    ifstream ifs(filename.c_str());

    string line;
    getline(ifs,line);

    while(getline(ifs,line)) {

      istringstream iss(line);

      RegionModel* _mod = NULL;

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

        _mod = new RegionBoxModel(xPair, yPair, zPair);
        _mod->SetType(modelType);
        _mod->ChangeColor();
      }
      else if(modelShapeName == "BOX2D") {

        Point2d min, max;
        iss >> min >> max;

        pair<double, double> xPair(min[0], max[0]);
        pair<double, double> yPair(min[1], max[1]);

        _mod = new RegionBox2DModel(xPair, yPair);
        _mod->SetType(modelType);
        _mod->ChangeColor();

      }
      else if(modelShapeName == "SPHERE") {

        Point3d tempCenter;
        iss >> tempCenter;

        double radius = -1;
        iss >> radius;

        _mod = new RegionSphereModel(tempCenter, radius);
        _mod->SetType(modelType);
        _mod->ChangeColor();
      }
      else if(modelShapeName == "SPHERE2D") {

        Point3d tempCenter;
        iss >> tempCenter;

        double radius = -1;
        iss >> radius;

        _mod = new RegionSphere2DModel(tempCenter, radius);
        _mod->SetType(modelType);
        _mod->ChangeColor();
      }

      if(_mod != NULL) {
        //Checks what type of region then adds
        if(_mod->GetType() == 0) {
          GetVizmo().GetEnv()->AddAttractRegion(_mod);
        }
        else if(_mod->GetType() == 1) {
          GetVizmo().GetEnv()->AddAvoidRegion(_mod);
        }
        else if(_mod->GetType() == 2) {
          GetVizmo().GetEnv()->AddNonCommitRegion(_mod);
        }
      }
      m_mainWindow->GetGLWidget()->SetCurrentRegion(_mod);
      m_mainWindow->GetModelSelectionWidget()->ResetLists();
      GetVizmo().GetSelectedModels().clear();
      GetVizmo().GetSelectedModels().push_back(_mod);
      m_mainWindow->GetModelSelectionWidget()->Select();
    }
  }
  else{
    m_mainWindow->statusBar()->showMessage("Loading aborted", 2000);
  }
  m_mainWindow->GetGLWidget()->updateGL();
}

MapEnvironmentWorker::
MapEnvironmentWorker(string _strategyLabel) : QObject(), m_strategyLabel(_strategyLabel) {}

void
MapEnvironmentWorker::
Solve() {
  GetVizmo().Solve(m_strategyLabel);
  emit Finished();
}
