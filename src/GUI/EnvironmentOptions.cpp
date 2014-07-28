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
#include "Models/QueryModel.h"
#include "Models/Vizmo.h"

#include "Icons/AddObstacle.xpm"
#include "Icons/DeleteObstacle.xpm"
#include "Icons/MoveObstacle.xpm"
#include "Icons/DuplicateObstacle.xpm"
#include "Icons/ChangeBoundary.xpm"
#include "Icons/EditRobot.xpm"
#include "Icons/RefreshEnv.xpm"
#include "Icons/RandEnv.xpm"

#include "Icons/RobotMode.xpm"
#include "Icons/PointMode.xpm"

#include "Utilities/AlertUser.h"

EnvironmentOptions::
EnvironmentOptions(QWidget* _parent, MainWindow* _mainWindow) :
  OptionsBase(_parent, _mainWindow),  m_editRobotDialog(NULL), m_changeBoundaryDialog(NULL) {
    CreateActions();
    SetUpCustomSubmenu();
    SetUpToolTab();
    SetHelpTips();
  }

void
EnvironmentOptions::
RefreshEnv() {
  GetVizmo().GetEnv()->SetRenderMode(SOLID_MODE);
  m_mainWindow->GetModelSelectionWidget()->reset();
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  m_mainWindow->GetGLWidget()->updateGL();
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

  // Added For ROBOT and point view
  QAction* robotView = new QAction(QPixmap(robotmode), tr("Robot"), this);
  m_actions["robotView"] = robotView;

  QAction* pointView = new QAction(QPixmap(pointmode), tr("Point"), this);
  m_actions["pointView"] = pointView;


  //2. Set other specifications as necessary
  m_actions["refreshEnv"]->setEnabled(false);

  // Added for robot and point view
  m_actions["robotView"]->setEnabled(false);
  m_actions["robotView"]->setStatusTip(tr("Display nodes in robot mode"));

  m_actions["pointView"]->setEnabled(false);
  m_actions["pointView"]->setStatusTip(tr("Display nodes in point mode"));

  m_actions["addObstacle"]->setEnabled(false);
  m_actions["deleteObstacle"]->setEnabled(false);
  m_actions["moveObstacle"]->setEnabled(false);
  m_actions["duplicateObstacle"]->setEnabled(false);
  m_actions["changeBoundary"]->setEnabled(false);
  m_actions["editRobot"]->setEnabled(false);

  m_actions["randEnvColors"]->setEnabled(false);
  m_actions["randEnvColors"]->setStatusTip(tr("Randomize environment colors"));

  //3. Make connections
  connect(m_actions["refreshEnv"], SIGNAL(triggered()), this, SLOT(RefreshEnv()));
  connect(m_actions["randEnvColors"], SIGNAL(triggered()), this, SLOT(RandomizeEnvColors()));
  // Added for robot and point modes
  connect(m_actions["robotView"], SIGNAL(triggered()), this, SLOT(ClickRobot()));
  connect(m_actions["pointView"], SIGNAL(triggered()), this, SLOT(ClickPoint()));

  connect(m_actions["addObstacle"], SIGNAL(triggered()), this, SLOT(AddObstacle()));
  connect(m_actions["deleteObstacle"], SIGNAL(triggered()), this, SLOT(DeleteObstacle()));
  connect(m_actions["moveObstacle"], SIGNAL(triggered()), this, SLOT(MoveObstacle()));
  connect(m_actions["duplicateObstacle"], SIGNAL(triggered()), this, SLOT(DuplicateObstacles()));
  connect(m_actions["changeBoundary"], SIGNAL(triggered()), this, SLOT(ChangeBoundaryForm()));
  connect(m_actions["editRobot"], SIGNAL(triggered()), this, SLOT(EditRobot()));
}

void
EnvironmentOptions::
SetUpCustomSubmenu() {
  m_submenu = new QMenu("Environment", this);

  m_submenu->addAction(m_actions["refreshEnv"]);

  m_nodeShape = new QMenu("Change Node Shape", this);
  m_nodeShape->addAction(m_actions["robotView"]);
  m_nodeShape->addAction(m_actions["pointView"]);
  m_submenu->addMenu(m_nodeShape);

  m_obstacleMenu = new QMenu("Obstacle Tools", this);
  m_obstacleMenu->addAction(m_actions["addObstacle"]);
  m_obstacleMenu->addAction(m_actions["deleteObstacle"]);
  m_obstacleMenu->addAction(m_actions["moveObstacle"]);
  m_obstacleMenu->addAction(m_actions["duplicateObstacle"]);
  m_submenu->addMenu(m_obstacleMenu);
  m_submenu->addAction(m_actions["changeBoundary"]);
  m_submenu->addAction(m_actions["editRobot"]);
  m_obstacleMenu->setEnabled(false);

  m_nodeShape->setEnabled(false);
}

void
EnvironmentOptions::
SetUpToolbar() {
  m_toolbar = new QToolBar(m_mainWindow);
  m_toolbar->addAction(m_actions["robotView"]);
  m_toolbar->addAction(m_actions["pointView"]);
  m_toolbar->addAction(m_actions["randEnvColors"]);
  m_toolbar->addSeparator();
}

void
EnvironmentOptions::
SetUpToolTab() {
  vector<string> buttonList;

  //row 1 - node shape settings && env colors
  buttonList.push_back("robotView");
  buttonList.push_back("pointView");
  buttonList.push_back("randEnvColors");

  buttonList.push_back("addObstacle");
  buttonList.push_back("deleteObstacle");
  buttonList.push_back("moveObstacle");
  buttonList.push_back("duplicateObstacle");
  buttonList.push_back("changeBoundary");
  buttonList.push_back("editRobot");
  buttonList.push_back("_separator_");

  CreateToolTab(buttonList);
}

void
EnvironmentOptions::
Reset() {

  m_nodeShape->setEnabled(true);
  m_actions["robotView"]->setEnabled(true);
  m_actions["pointView"]->setEnabled(true);
  m_actions["randEnvColors"]->setEnabled(true);

  ClickPoint();

  m_actions["refreshEnv"]->setEnabled(true);
  m_actions["addObstacle"]->setEnabled(true);
  m_actions["deleteObstacle"]->setEnabled(true);
  m_actions["moveObstacle"]->setEnabled(true);
  m_actions["duplicateObstacle"]->setEnabled(true);
  m_actions["changeBoundary"]->setEnabled(true);
  m_actions["editRobot"]->setEnabled(true);
  m_obstacleMenu->setEnabled(true);
}

void
EnvironmentOptions::
SetHelpTips() {

  m_actions["robotView"]->setWhatsThis(tr("Click this button to vizualize"
        " the nodes in <b>Robot</b> mode."));
  m_actions["pointView"]->setWhatsThis(tr("Click this button to vizualize"
        " the nodes in <b>Point</b> mode."));

  m_actions["randEnvColors"]->setWhatsThis(tr("Click this button to"
        " randomize the colors of the environment objects."));
  m_actions["addObstacle"]->setWhatsThis(tr("Add obstacle button"));
  m_actions["deleteObstacle"]->setWhatsThis(tr("Delete obstacle button"));
  m_actions["moveObstacle"]->setWhatsThis(tr("Move obstacle button"));
  m_actions["duplicateObstacle"]->setWhatsThis(tr("duplicate obstacle button"));
  m_actions["changeBoundary"]->setWhatsThis(tr("Change the boundary"));
  m_actions["editRobot"]->setWhatsThis(tr("Edit the robot"));
}

// Slots
void
EnvironmentOptions::
ClickRobot() {
  CfgModel::SetShape(CfgModel::Robot);
  if(GetVizmo().IsQueryLoaded())
    GetVizmo().GetQry()->Build();
  m_mainWindow->GetGLWidget()->updateGL();
}

void
EnvironmentOptions::
ClickPoint() {
  CfgModel::SetShape(CfgModel::Point);
  if(GetVizmo().IsQueryLoaded())
    GetVizmo().GetQry()->Build();
  m_mainWindow->GetGLWidget()->updateGL();
}

void
EnvironmentOptions::
RandomizeEnvColors() {
  GetVizmo().GetEnv()->ChangeColor();
  m_mainWindow->GetGLWidget()->updateGL();
}
