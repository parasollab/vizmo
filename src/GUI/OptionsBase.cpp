#include "OptionsBase.h"

#include <QMenu>
#include <QToolBar>
#include <QGridLayout>
#include <QToolButton>

#include "MainWindow.h"


void
OptionsBase::
SetUpSubmenu() {
  /// Places all actions in the menu.
  m_submenu = new QMenu(m_label, this);
  for(map<string, QAction*>::iterator ait = m_actions.begin();
      ait != m_actions.end(); ait++)
    m_submenu->addAction(ait->second);
}


void
OptionsBase::
SetUpToolbar() {
  /// The default implementation uses all actions, but it can be overridden by
  /// derived classes to use only a subset of actions.
  m_toolbar = new QToolBar(GetMainWindow());
  for(map<string, QAction*>::iterator ait = m_actions.begin();
      ait != m_actions.end(); ait++)
    m_toolbar->addAction(ait->second);
}


void
OptionsBase::
SetUpToolTab() {
  /// The default implementation uses all actions, but it can be overridden by
  /// derived classes to use only a subset of actions.
  vector<string> buttonList;
  for(map<string, QAction*>::iterator ait = m_actions.begin();
      ait != m_actions.end(); ait++)
    buttonList.push_back(ait->first);
  CreateToolTab(buttonList);
}


void
OptionsBase::
CreateToolTab(const vector<string>& _buttonList) {
  /// Use "_blank_" to leave an empty spot and "_separator_" to put space
  /// between button groups.

  //create a widget and layout for the tab
  m_toolTab = new QWidget(GetMainWindow());
  QGridLayout* tabLayout = new QGridLayout;

  //make a button for each action and add it to the layout. the layout uses 5
  //columns: cols 0,2,4 hold buttons and cols 1,3 make horizontal space between
  //buttons. even rows are skipped to leave vertical space between button rows.
  int row = 0;
  int col = 0;
  for(vector<string>::const_iterator vit = _buttonList.begin();
      vit != _buttonList.end(); vit++) {
    if((*vit != "_blank_") && (*vit != "_separator_")) {
      QToolButton* newButton = new QToolButton(m_toolTab);
      newButton->setDefaultAction(m_actions[*vit]);
      newButton->setToolButtonStyle(GetMainWindow()->toolButtonStyle());
      newButton->setIconSize(GetMainWindow()->iconSize());
      tabLayout->addWidget(newButton, row, col % 6);
    }
    else if(*vit == "_separator_") {
      //finish out the current row
      while((col % 6) != 0) {
        col += 2;
        //move to the next row once finished
        if((col % 6) == 0)
          row += 2;
      }
      //skip two columns. the standard column advance will finish out the row.
      col += 4;
    }
    //advance to next column
    col += 2;
    //move to next row after 5 columns (3 buttons)
    if((col % 6) == 0)
      row += 2;
  }

  //set sizing and stretch factors
  for(int i = 0; i < tabLayout->rowCount(); i ++)
    tabLayout->setRowMinimumHeight(i, 6);
  for(int i = 0; i < tabLayout->columnCount(); i++)
    tabLayout->setColumnMinimumWidth(i, 1);
  tabLayout->setRowStretch(tabLayout->rowCount(), 1);
  tabLayout->setColumnStretch(5, 1);

  //apply layout to tool tab
  m_toolTab->setLayout(tabLayout);
}
