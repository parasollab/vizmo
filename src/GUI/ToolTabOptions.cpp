#include "ToolTabOptions.h"

#include "MainMenu.h"
#include "MainWindow.h"
#include "ToolTabWidget.h"

/*-------------------------- GUI Management ----------------------------------*/

void
ToolTabOptions::
CreateActions() {
  MainMenu* mainMenu = GetMainWindow()->m_mainMenu;
  ToolTabWidget* toolTabWidget = GetMainWindow()->m_toolTabWidget;

  //pull list of tabs from main window's tool tab widget
  map<string, pair<string, QWidget*> >* tabList = &(toolTabWidget->m_tabs);

  //construct an action for each tab
  for(map<string, pair<string, QWidget*> >::iterator mit = tabList->begin();
      mit != tabList->end(); mit++) {
    //mit->first will be the tab label and mit->second will be its tool tip
    m_actions[mit->first] = new QAction(tr((mit->first).c_str()), this);
    m_actions[mit->first]->setToolTip(tr((mit->second).first.c_str()));
    m_actions[mit->first]->setEnabled(true);
    m_actions[mit->first]->setCheckable(true);
    m_actions[mit->first]->setChecked(true);
    connect(m_actions[mit->first], SIGNAL(triggered()),
        toolTabWidget, SLOT(ToggleTab()));
  }

  //create the submenu
  SetUpSubmenu();
  mainMenu->m_menuBar->insertMenu(mainMenu->m_end,
      mainMenu->m_toolTabOptions->GetSubMenu());
}
