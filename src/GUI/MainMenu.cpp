#include "MainMenu.h"
#include "MainWin.h"
#include "OptionsBase.h"
#include "FileOptions.h"
#include "SceneOptions.h"
#include "RobotOptions.h"
#include "EnvironmentOptions.h"
#include "RoadmapOptions.h"
#include "PathOptions.h"
#include "CaptureOptions.h"
#include "HelpOptions.h"

#include <iostream>
#include <QMenuBar>

using namespace std;

MainMenu::MainMenu(VizmoMainWin* _parent) : QWidget(_parent) {
  m_fileOptions = new FileOptions(this, _parent);
  m_sceneOptions = new SceneOptions(this, _parent);
  m_robotOptions = new RobotOptions(this, _parent);
  m_environmentOptions = new EnvironmentOptions(this, _parent);
  m_roadmapOptions = new RoadmapOptions(this, _parent);
  m_pathOptions = new PathOptions(this, _parent);
  m_captureOptions = new CaptureOptions(this, _parent);
  m_help = new HelpOptions(this, _parent);              //This one should always be last!

  SetUpMainMenu();
}

void
MainMenu::SetUpMainMenu(){

  m_menubar = new QMenuBar(this);

  m_menubar->addMenu(m_fileOptions->m_submenu);
  m_menubar->addMenu(m_sceneOptions->m_submenu);
  m_menubar->addMenu(m_robotOptions->m_submenu);
  m_menubar->addMenu(m_environmentOptions->m_submenu);
  m_menubar->addMenu(m_roadmapOptions->m_submenu);
  m_menubar->addMenu(m_pathOptions->m_submenu);
  m_menubar->addMenu(m_captureOptions->m_submenu);
  m_menubar->addMenu(m_help->m_submenu);
}

void
MainMenu::Reset(){

  m_fileOptions->Reset();
  m_sceneOptions->Reset();
  m_robotOptions->Reset();
  m_environmentOptions->Reset();
  m_roadmapOptions->Reset();
  m_pathOptions->Reset();
  m_captureOptions->Reset();
  m_help->Reset();
}


