#include "EnvironmentOptions.h"

#include <QAction>
#include <QToolBar>
#include <QPixmap>
#include <QTranslator>

#include "MainWin.h"
#include "SceneWin.h"
#include "ItemSelectionGUI.h"
#include "Models/Vizmo.h"

#include "Icons/RandEnv.xpm"

  EnvironmentOptions::EnvironmentOptions(QWidget* _parent, VizmoMainWin* _mainWin)
:OptionsBase(_parent, _mainWin)
{
  CreateActions();
  SetUpSubmenu("Environment");
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

  //2. Set other specifications as necessary
  m_actions["refreshEnv"]->setEnabled(false);
  m_actions["randEnvColors"]->setEnabled(false);
  m_actions["randEnvColors"]->setStatusTip(tr("Randomize environment colors"));
  m_actions["addObstacle"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["refreshEnv"], SIGNAL(activated()), this, SLOT(RefreshEnv()));
  connect(m_actions["randEnvColors"], SIGNAL(activated()), this, SLOT(RandomizeEnvColors()));
  connect(m_actions["addObstacle"], SIGNAL(activated()), this, SLOT(AddObstacle()));
}

void
EnvironmentOptions::SetUpToolbar(){
  m_toolbar = new QToolBar(GetMainWin());
  m_toolbar->addAction(m_actions["randEnvColors"]);
}

void
EnvironmentOptions::Reset(){
  m_actions["refreshEnv"]->setEnabled(true);
  m_actions["randEnvColors"]->setEnabled(true);
  m_actions["addObstacle"]->setEnabled(true);
}

void
EnvironmentOptions::SetHelpTips(){
  m_actions["randEnvColors"]->setWhatsThis(tr("Click this button to"
    " randomize the colors of the environment objects."));
}


//Slots

void
EnvironmentOptions::RefreshEnv(){
  GetVizmo().RefreshEnv();
  GetMainWin()->GetObjectSelection()->reset();
  GetMainWin()->GetGLScene()->updateGL();
}

void
EnvironmentOptions::RandomizeEnvColors(){
  GetVizmo().envObjsRandomColor();
  GetMainWin()->GetGLScene()->updateGL();
}

void
EnvironmentOptions::AddObstacle(){
  //TODO: Add back in functionality for adding obstacles
}
