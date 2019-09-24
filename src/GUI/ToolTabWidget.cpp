#include "MainMenu.h"
#include "MainWindow.h"
#include "OptionsBase.h"
#include "ToolTabOptions.h"
#include "ToolTabWidget.h"

#include <QAction>

ToolTabWidget::
ToolTabWidget(MainWindow* _mainWindow) : QTabWidget() {
  //map tabs for each option class to their names
  m_tabs["Roadmap"] = make_pair("Roadmap Tools",
      _mainWindow->m_mainMenu->m_roadmapOptions->GetToolTab());
  m_tabs["Env"] = make_pair("Environment Tools",
      _mainWindow->m_mainMenu->m_environmentOptions->GetToolTab());
  m_tabs["Display"] = make_pair("GL Options",
      _mainWindow->m_mainMenu->m_glWidgetOptions->GetToolTab());
  m_tabs["Path"] = make_pair("Path Tools",
      _mainWindow->m_mainMenu->m_pathOptions->GetToolTab());
  m_tabs["Query"] = make_pair("Query Tools",
      _mainWindow->m_mainMenu->m_queryOptions->GetToolTab());
  m_tabs["Planning"] = make_pair("Planning Tools",
      _mainWindow->m_mainMenu->m_planningOptions->GetToolTab());

  //add tabs to tool tab widget
  for(map<string, pair<string, QWidget*> >::iterator mit = m_tabs.begin();
      mit != m_tabs.end(); mit++) {
    addTab((mit->second).second, mit->first.c_str());
    setTabToolTip(count() - 1, (mit->second).first.c_str());
  }

  //set size
  setFixedSize(160, 345);

  //set tabs to appear on right-hand side
  setTabPosition(QTabWidget::East);

  //set tabs to be movable
  setMovable(true);
}

void
ToolTabWidget::
ToggleTab() {
  //this slot must be connected to a QAction
  QAction* toggle = static_cast<QAction*>(sender());
  string name = toggle->text().toStdString();
  int index = indexOf(m_tabs[name].second);
  //if tab is not visible, add it to ToolTabWidget
  if(index == -1) {
    addTab(m_tabs[name].second, name.c_str());
    setTabToolTip(count() - 1, m_tabs[name].first.c_str());
  }
  //if tab is already visible, remove it from ToolTabWidget
  else
    removeTab(index);
}
