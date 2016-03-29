#include "EnvironmentOptions.h"

#include "ChangeBoundaryDialog.h"
#include "EditRobotDialog.h"
#include "GLWidget.h"
#include "MainMenu.h"
#include "MainWindow.h"
#include "ModelSelectionWidget.h"
#include "ObstaclePosDialog.h"

#include "Models/EnvModel.h"
#include "Models/QueryModel.h"
#include "Models/RegionBoxModel.h"
#include "Models/RegionBox2DModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/RegionSphere2DModel.h"
#include "Models/StaticMultiBodyModel.h"
#include "Models/Vizmo.h"

#include "Icons/AddObstacle.xpm"
#include "Icons/ChangeBoundary.xpm"
#include "Icons/DeleteObstacle.xpm"
#include "Icons/DuplicateObstacle.xpm"
#include "Icons/EditRobot.xpm"
#include "Icons/MoveObstacle.xpm"
#include "Icons/RandEnv.xpm"
#include "Icons/RefreshEnv.xpm"

#include "Icons/PointMode.xpm"
#include "Icons/RobotMode.xpm"


EnvironmentOptions::
EnvironmentOptions(QWidget* _parent) : OptionsBase(_parent, "Environment"),
    m_changeBoundaryDialog(NULL), m_editRobotDialog(NULL) {
  CreateActions();
  SetHelpTips();
  SetUpSubmenu();
  SetUpToolTab();
}

/*------------------------- GUI Management -----------------------------------*/

void
EnvironmentOptions::
CreateActions() {
  //1. Create actions and add them to the map
  m_actions["refreshEnv"] = new QAction(QPixmap(refreshenv),
      tr("Refresh"), this);
  m_actions["randEnvColors"] = new QAction(QPixmap(randEnvIcon),
      tr("Randomize Environment Colors"), this);
  m_actions["addObstacle"] = new QAction(QPixmap(addobstacle),
      tr("Add Obstacle"), this);
  m_actions["deleteObstacle"] = new QAction(QPixmap(deleteobstacle),
      tr("Delete Obstacle(s)"), this);
  m_actions["moveObstacle"] = new QAction(QPixmap(moveobstacle),
      tr("Move Obstacle(s)"), this);
  m_actions["duplicateObstacle"] = new QAction(QPixmap(duplicateobstacle),
      tr("Duplicate Obstacle(s)"), this);
  m_actions["changeBoundary"] = new QAction(QPixmap(changeboundary),
      tr("Change Boundary"), this);
  m_actions["editRobot"] = new QAction(QPixmap(editrobot),
      tr("Edit the Robot"), this);
  m_actions["robotView"] = new QAction(QPixmap(robotmode), tr("Robot"), this);
  m_actions["pointView"] = new QAction(QPixmap(pointmode), tr("Point"), this);

  //2. Set other specifications as necessary
  m_actions["refreshEnv"]->setEnabled(false);
  m_actions["robotView"]->setEnabled(false);
  m_actions["pointView"]->setEnabled(false);
  m_actions["addObstacle"]->setEnabled(false);
  m_actions["deleteObstacle"]->setEnabled(false);
  m_actions["moveObstacle"]->setEnabled(false);
  m_actions["duplicateObstacle"]->setEnabled(false);
  m_actions["changeBoundary"]->setEnabled(false);
  m_actions["editRobot"]->setEnabled(false);
  m_actions["randEnvColors"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["refreshEnv"], SIGNAL(triggered()),
      this, SLOT(RefreshEnv()));
  connect(m_actions["randEnvColors"], SIGNAL(triggered()),
      this, SLOT(RandomizeEnvColors()));
  connect(m_actions["robotView"], SIGNAL(triggered()),
      this, SLOT(ClickRobot()));
  connect(m_actions["pointView"], SIGNAL(triggered()),
      this, SLOT(ClickPoint()));
  connect(m_actions["addObstacle"], SIGNAL(triggered()),
      this, SLOT(AddObstacle()));
  connect(m_actions["deleteObstacle"], SIGNAL(triggered()),
      this, SLOT(DeleteObstacle()));
  connect(m_actions["moveObstacle"], SIGNAL(triggered()),
      this, SLOT(MoveObstacle()));
  connect(m_actions["duplicateObstacle"], SIGNAL(triggered()),
      this, SLOT(DuplicateObstacles()));
  connect(m_actions["changeBoundary"], SIGNAL(triggered()),
      this, SLOT(ChangeBoundaryForm()));
  connect(m_actions["editRobot"], SIGNAL(triggered()),
      this, SLOT(EditRobot()));
}


void
EnvironmentOptions::
SetHelpTips() {
  m_actions["robotView"]->setStatusTip(tr("Display nodes in robot mode"));
  m_actions["robotView"]->setWhatsThis(tr("Click this button to vizualize"
        " the nodes in <b>Robot</b> mode."));

  m_actions["pointView"]->setStatusTip(tr("Display nodes in point mode"));
  m_actions["pointView"]->setWhatsThis(tr("Click this button to vizualize"
        " the nodes in <b>Point</b> mode."));

  m_actions["randEnvColors"]->setStatusTip(tr("Randomize environment colors"));
  m_actions["randEnvColors"]->setWhatsThis(tr("Click this button to"
        " randomize the colors of the environment objects."));

  m_actions["addObstacle"]->setWhatsThis(tr("Add obstacle button"));
  m_actions["deleteObstacle"]->setWhatsThis(tr("Delete obstacle button"));
  m_actions["moveObstacle"]->setWhatsThis(tr("Move obstacle button"));
  m_actions["duplicateObstacle"]->setWhatsThis(tr("duplicate obstacle button"));
  m_actions["changeBoundary"]->setWhatsThis(tr("Change the boundary"));
  m_actions["editRobot"]->setWhatsThis(tr("Edit the robot"));
}


void
EnvironmentOptions::
SetUpSubmenu() {
  m_submenu = new QMenu(m_label, this);

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
SetUpToolTab() {
  vector<string> buttonList;

  buttonList.push_back("robotView");
  buttonList.push_back("pointView");
  buttonList.push_back("randEnvColors");
  buttonList.push_back("_separator_");

  buttonList.push_back("addObstacle");
  buttonList.push_back("duplicateObstacle");
  buttonList.push_back("deleteObstacle");
  buttonList.push_back("moveObstacle");
  buttonList.push_back("_separator_");

  buttonList.push_back("changeBoundary");
  buttonList.push_back("_separator_");

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

  ClickRobot();

  m_actions["refreshEnv"]->setEnabled(true);
  m_actions["addObstacle"]->setEnabled(true);
  m_actions["deleteObstacle"]->setEnabled(true);
  m_actions["moveObstacle"]->setEnabled(true);
  m_actions["duplicateObstacle"]->setEnabled(true);
  m_actions["changeBoundary"]->setEnabled(true);
  m_actions["editRobot"]->setEnabled(true);
  m_obstacleMenu->setEnabled(true);
}

/*----------------------- Environment Editing --------------------------------*/

void
EnvironmentOptions::
AddObstacle() {
  //get the name of an obstacle file
  QString fn = QFileDialog::getOpenFileName(this, "Choose an obstacle to load",
      GetMainWindow()->GetLastDir(), "Files  (*.g *.obj)");

  if(!fn.isEmpty()) {
    //save last directory
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());

    //add the new obstacle to the environment and select it
    shared_ptr<StaticMultiBodyModel> m = GetVizmo().GetEnv()->AddObstacle(
        fi.absolutePath().toStdString(),
        fi.fileName().toStdString(),
        Transformation());
    RefreshEnv();

    //Select new obstacle
    GetVizmo().GetSelectedModels().clear();
    GetVizmo().GetSelectedModels().push_back(m.get());

    //open the obstacle position dialog for the new obstacle
    vector<StaticMultiBodyModel*> v(1, m.get());
    ObstaclePosDialog* opd = new ObstaclePosDialog(GetMainWindow(), v);
    GetMainWindow()->ShowDialog(opd);
  }
  else
    GetMainWindow()->statusBar()->showMessage("Loading aborted");
}


void
EnvironmentOptions::
DeleteObstacle() {
  vector<MultiBodyModel*> toDel;
  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  //grab the bodies from the selected vector
  for(auto& s : sel)
    if(s->Name().find("MultiBody") != string::npos &&
        s->Name() != "ActiveMultiBody")
      toDel.push_back((MultiBodyModel*)s);

  //alert that only non-active multibodies can be selected
  if(toDel.empty() || toDel.size() != sel.size())
    GetMainWindow()->AlertUser(
        "Must select one or more non-active multibodies only.");

  //successful selection, delete obstacle(s)
  else {
    /*typedef vector<MultiBodyModel*>::iterator MIT;
    for(MIT mit = toDel.begin(); mit != toDel.end(); ++mit)
      GetVizmo().GetEnv()->DeleteMBModel(*mit);*/

    GetVizmo().GetSelectedModels().clear();
    RefreshEnv();
  }
}


void
EnvironmentOptions::
MoveObstacle() {
  vector<StaticMultiBodyModel*> toMove;
  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  //grab the bodies from the selected vector
  for(auto& s : sel)
    if(s->Name().find("MultiBody") != string::npos &&
        s->Name() != "ActiveMultiBody")
      toMove.push_back((StaticMultiBodyModel*)s);

  //alert that only non-active multibodies can be selected
  if(toMove.empty() || toMove.size() != sel.size())
    GetMainWindow()->AlertUser(
        "Must select one or more non-active multibodies only.");

  //successful selection, show ObstaclePosDialog
  else {
    ObstaclePosDialog* opd = new ObstaclePosDialog(GetMainWindow(), toMove);
    GetMainWindow()->ShowDialog(opd);
  }
}


void
EnvironmentOptions::
DuplicateObstacles() {
  vector<StaticMultiBodyModel*> toCopy;
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  for(auto& s : sel)
    if(s->Name().find("MultiBody") != string::npos &&
        s->Name() != "ActiveMultiBody")
      toCopy.push_back((StaticMultiBodyModel*)s);

  //alert that only non-active multibodies can be selected
  if(toCopy.empty() || toCopy.size() != sel.size())
    GetMainWindow()->AlertUser(
        "Must select one or more non-active multibodies only.");

  //successful selection, copy and show ObstaclePosDialog
  else {
    vector<StaticMultiBodyModel*> copies;
    /*typedef vector<MultiBodyModel*>::iterator MIT;
    for(MIT mit = toCopy.begin(); mit != toCopy.end(); ++mit) {
      MultiBodyModel* m = new MultiBodyModel(**mit);
      copies.push_back(m);
      GetVizmo().GetEnv()->AddMBModel(m);
    }*/
    sel.clear();
    copy(copies.begin(), copies.end(), back_inserter(sel));

    ObstaclePosDialog* opd = new ObstaclePosDialog(GetMainWindow(), copies);
    GetMainWindow()->ShowDialog(opd);
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
      GetMainWindow()->AlertUser("Must select only the boundary.");
    }
    //successful selection, show ChangeBoundaryDialog
    else {
      m_changeBoundaryDialog = new ChangeBoundaryDialog(GetMainWindow());
      GetMainWindow()->ShowDialog(m_changeBoundaryDialog);
    }
  }
}


void
EnvironmentOptions::
EditRobot() {
  if(m_editRobotDialog == NULL) {
    m_editRobotDialog = new EditRobotDialog(GetMainWindow());
    GetMainWindow()->ShowDialog(m_editRobotDialog);
  }
}

/*----------------------- Environment Display --------------------------------*/

void
EnvironmentOptions::
RefreshEnv() {
  GetVizmo().GetEnv()->SetRenderMode(SOLID_MODE);
  GetMainWindow()->GetModelSelectionWidget()->reset();
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
}


void
EnvironmentOptions::
ClickRobot() {
  CfgModel::SetShape(CfgModel::Robot);
  if(GetVizmo().IsQueryLoaded())
    GetVizmo().GetQry()->Build();
}


void
EnvironmentOptions::
ClickPoint() {
  CfgModel::SetShape(CfgModel::Point);
  if(GetVizmo().IsQueryLoaded())
    GetVizmo().GetQry()->Build();
}


void
EnvironmentOptions::
RandomizeEnvColors() {
  GetVizmo().GetEnv()->ChangeColor();
}
