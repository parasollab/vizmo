#include "EnvironmentOptions.h"

#include <QAction>
#include <QToolBar>
#include <QPixmap>
#include <QTranslator>

#include "MainWindow.h"
#include "GLWidget.h"
#include "ItemSelectionGUI.h"
#include "Models/Vizmo.h"

#include "Icons/RandEnv.xpm"

EnvironmentOptions::EnvironmentOptions(QWidget* _parent, MainWindow* _mainWindow)
  : OptionsBase(_parent, _mainWindow) {
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
  connect(m_actions["refreshEnv"], SIGNAL(triggered()), this, SLOT(RefreshEnv()));
  connect(m_actions["randEnvColors"], SIGNAL(triggered()), this, SLOT(RandomizeEnvColors()));
  connect(m_actions["addObstacle"], SIGNAL(triggered()), this, SLOT(AddObstacle()));
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
  m_mainWindow->GetObjectSelection()->reset();
  m_mainWindow->GetGLScene()->updateGL();
}

void
EnvironmentOptions::RandomizeEnvColors(){
  GetVizmo().RandomizeEnvColors();
  m_mainWindow->GetGLScene()->updateGL();
}

void
EnvironmentOptions::AddObstacle(){
  //TODO: Add back in functionality for adding obstacles
}
