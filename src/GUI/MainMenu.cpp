#include "MainMenu.h"

#include <QMenuBar>

#include "MainWindow.h"
#include "OptionsBase.h"

#include "CaptureOptions.h"
#include "EnvironmentOptions.h"
#include "FileOptions.h"
#include "GLWidgetOptions.h"
#include "HelpOptions.h"
#include "PathOptions.h"
#include "PlanningOptions.h"
#include "QueryOptions.h"
#include "RoadmapOptions.h"
#include "ToolTabOptions.h"

#include <iostream>

using namespace std;


MainMenu::
MainMenu(MainWindow* _mainWindow) : QWidget(_mainWindow) {
  //create option classes
  m_fileOptions = new FileOptions(this);
  m_glWidgetOptions = new GLWidgetOptions(this);
  m_environmentOptions = new EnvironmentOptions(this);
  m_roadmapOptions = new RoadmapOptions(this);
  m_pathOptions = new PathOptions(this);
  m_planningOptions = new PlanningOptions(this);
  m_queryOptions = new QueryOptions(this);
  m_captureOptions = new CaptureOptions(this);
  m_toolTabOptions = new ToolTabOptions(this);
  m_help = new HelpOptions(this); //needs to be last as it depends on the
                                  //'whats this' in the other options classes

  //add submenus to the main menu bar
  m_menuBar = new QMenuBar(this);
  m_menuBar->addMenu(m_fileOptions->GetSubMenu());
  m_menuBar->addMenu(m_glWidgetOptions->GetSubMenu());
  m_menuBar->addMenu(m_environmentOptions->GetSubMenu());
  m_menuBar->addMenu(m_pathOptions->GetSubMenu());
  m_menuBar->addMenu(m_planningOptions->GetSubMenu());
  m_menuBar->addMenu(m_queryOptions->GetSubMenu());
  m_menuBar->addMenu(m_roadmapOptions->GetSubMenu());
  m_menuBar->addMenu(m_captureOptions->GetSubMenu());
  m_end = m_menuBar->addMenu(m_help->GetSubMenu());
}


void
MainMenu::
CallReset() {
  m_fileOptions->Reset();
  m_glWidgetOptions->Reset();
  m_environmentOptions->Reset();
  m_roadmapOptions->Reset();
  m_pathOptions->Reset();
  m_planningOptions->Reset();
  m_queryOptions->Reset();
  m_captureOptions->Reset();
  m_help->Reset();
}


void
MainMenu::
ConfigureToolTabMenu() {
  static_cast<ToolTabOptions*>(m_toolTabOptions)->Init();
}
