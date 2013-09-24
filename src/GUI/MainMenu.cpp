#include "MainMenu.h"

#include "MainWindow.h"
#include "OptionsBase.h"
#include "FileOptions.h"
#include "GLWidgetOptions.h"
#include "RobotOptions.h"
#include "EnvironmentOptions.h"
#include "RoadmapOptions.h"
#include "PathOptions.h"
#include "CaptureOptions.h"
#include "HelpOptions.h"

#include <iostream>
#include <QMenuBar>

using namespace std;

MainMenu::MainMenu(MainWindow* _mainWindow) {

  m_fileOptions = new FileOptions(this, _mainWindow);
  m_glWidgetOptions = new GLWidgetOptions(this, _mainWindow);
  m_robotOptions = new RobotOptions(this, _mainWindow);
  m_environmentOptions = new EnvironmentOptions(this, _mainWindow);
  m_roadmapOptions = new RoadmapOptions(this, _mainWindow);
  m_pathOptions = new PathOptions(this, _mainWindow);
  m_captureOptions = new CaptureOptions(this, _mainWindow);
  m_help = new HelpOptions(this, _mainWindow);              //This one should always be last!

  SetUpMainMenu();
}

void
MainMenu::SetUpMainMenu(){

  m_menuBar = new QMenuBar(this);

  m_menuBar->addMenu(m_fileOptions->GetSubMenu());
  m_menuBar->addMenu(m_glWidgetOptions->GetSubMenu());
  m_menuBar->addMenu(m_robotOptions->GetSubMenu());
  m_menuBar->addMenu(m_environmentOptions->GetSubMenu());
  m_menuBar->addMenu(m_roadmapOptions->GetSubMenu());
  m_menuBar->addMenu(m_pathOptions->GetSubMenu());
  m_menuBar->addMenu(m_captureOptions->GetSubMenu());
  m_menuBar->addMenu(m_help->GetSubMenu());
}

void
MainMenu::CallReset(){

  m_fileOptions->Reset();
  m_glWidgetOptions->Reset();
  m_robotOptions->Reset();
  m_environmentOptions->Reset();
  m_roadmapOptions->Reset();
  m_pathOptions->Reset();
  m_captureOptions->Reset();
  m_help->Reset();
}
