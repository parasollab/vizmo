#include "OptionsBase.h"

//create a 3x3 tool tab from a vector of action names.
//use "_blank_" to leave an empty spot.
//use "_separator_" to put space between button groups.
void
OptionsBase::
CreateToolTab(vector<string> _buttonList) {
  //create a widget and layout for the tab
  m_toolTab = new QWidget(m_mainWindow);
  QGridLayout* tabLayout = new QGridLayout;

  //make a button for each action and add it to the layout
  //leave a row/column space between each button
  int row = 0;
  int col = 0;
  for(vector<string>::iterator vit = _buttonList.begin();
      vit != _buttonList.end(); vit++) {
    if((*vit != "_blank_") && (*vit != "_separator_")) {
      QToolButton* newButton = new QToolButton(m_toolTab);
      newButton->setDefaultAction(m_actions[*vit]);
      newButton->setToolButtonStyle(m_mainWindow->toolButtonStyle());
      newButton->setIconSize(m_mainWindow->iconSize());
      tabLayout->addWidget(newButton, row, col % 6);
    }
    if(*vit == "_separator_") {
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
    //move to next row after 3 buttons
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

