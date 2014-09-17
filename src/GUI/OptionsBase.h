/************************************************************
 * Abstract base class for functionalities such as file options,
 *roadmap options, etc. Derived classes have a map of associated
 *actions. Drop-down submenus for each derived class
 *are created with SetUpSubmenu(), while SetUpToolbar() can be
 *overloaded to create a toolbar (one or more buttons) in the main
 *window using some or all of the actions used in the submenu.
 *************************************************************/

#ifndef OPTIONS_BASE_H_
#define OPTIONS_BASE_H_

#include <QtGui>
#include "MainWindow.h"

class MainMenu;

using namespace std;

class OptionsBase : public QWidget {

  Q_OBJECT

  public:
    OptionsBase(QWidget* _parent, MainWindow* _mainWindow) :
      QWidget(_parent), m_mainWindow(_mainWindow),
      m_submenu(NULL), m_toolbar(NULL), m_toolTab(NULL) {}

    virtual void Reset()=0;        //enable appropriate actions when main win is reset
    QMenu* GetSubMenu() {return m_submenu;}
    QToolBar* GetToolbar() {return m_toolbar;}
    QWidget* GetToolTab() {return m_toolTab;}
    void ResetRegionTimer() {}

  protected:
    MainWindow* m_mainWindow;
    QMenu* m_submenu;    //has all associated actions
    map<string, QAction*> m_actions;
    QToolBar* m_toolbar; //may have none, one, some, or all assoc. actions
    QWidget* m_toolTab;  //tab format for actions in m_submenu
    virtual void CreateActions()=0;
    virtual void SetUpToolbar()=0;
    virtual void SetHelpTips()=0;    //for the "What's This?" utility
    void SetUpSubmenu(QString _title) {
      m_submenu = new QMenu(_title, this);
      for(map<string, QAction*>::iterator ait = m_actions.begin(); ait != m_actions.end(); ait++)
        m_submenu->addAction(ait->second);
    }
    void CreateToolTab(vector<string> _buttonList);
};

#endif
