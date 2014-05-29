#include "MainMenu.h"
#include "MainWindow.h"
#include "ToolTabOptions.h"
#include "ToolTabWidget.h"

ToolTabOptions::ToolTabOptions(QWidget* _parent, MainWindow* _mainWindow) :
    OptionsBase(_parent, _mainWindow) {}

void
ToolTabOptions::CreateActions() {
  //pull list of tabs from main window's tool tab widget
  map<string, pair<string, QWidget*> >* tabList = &(m_mainWindow->m_toolTabWidget->m_tabs);

  //construct an action for each tab
  for(map<string, pair<string, QWidget*> >::iterator mit = tabList->begin();
      mit != tabList->end(); mit++) {
    m_actions[mit->first] = new QAction(tr((mit->first).c_str()), this);
    m_actions[mit->first]->setToolTip(tr((mit->second).first.c_str()));
    m_actions[mit->first]->setEnabled(true);
    m_actions[mit->first]->setCheckable(true);
    m_actions[mit->first]->setChecked(true);
    connect(m_actions[mit->first], SIGNAL(triggered()), m_mainWindow->m_toolTabWidget,
        SLOT(ToggleTab()));
  }

  //create the submenu
  SetUpSubmenu("Tool Tabs");
  m_mainWindow->m_mainMenu->m_menuBar->insertMenu(
      m_mainWindow->m_mainMenu->m_end,
      m_mainWindow->m_mainMenu->m_toolTabOptions->GetSubMenu());
}

void
ToolTabOptions::SetHelpTips() {

}
