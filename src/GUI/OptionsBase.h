/************************************************************
 * Abstract base class for functionalities such as file options,
 *roadmap options, etc. Derived classes have a map of associated
 *actions. Drop-down submenus for each derived class
 *are created with SetUpSubmenu(), while SetUpToolbar() can be
 *overloaded to create a toolbar (one or more buttons) in the main
 *window using some or all of the actions used in the submenu.
 *************************************************************/

#ifndef OPTIONS_BASE_H
#define OPTIONS_BASE_H

#include "MainWin.h"

#include <QAction> //may be able to trim this later...
#include <QMenu>
#include <QToolBar>
#include <QToolButton>

#include <string>
#include <map>
#include <vector>

class VizmoMainWin;
class MainMenu;

using namespace std;

class OptionsBase : public QWidget{

  Q_OBJECT

  public:
    OptionsBase(QWidget* _parent, VizmoMainWin* _mainWin) :
      QWidget(_parent), m_mainWin(_mainWin),
      m_submenu(NULL), m_toolbar(NULL) {}

    QToolBar* GetToolbar(){return m_toolbar;}

    VizmoMainWin* m_mainWin;
    map<string, QAction*> m_actions;
    QMenu* m_submenu;    //has all associated actions
    QToolBar* m_toolbar; //may have none, one, some, or all assoc. actions

    virtual void CreateActions()=0;
    virtual void SetUpToolbar()=0;
    virtual void Reset()=0;        //enable appropriate actions when main win is reset
    virtual void SetHelpTips()=0;    //for the "What's This?" utility

    VizmoMainWin* GetMainWin() {return m_mainWin;}

    void
    SetUpSubmenu(QString _title){

      m_submenu = new QMenu(_title, this);
      for(map<string, QAction*>::iterator ait = m_actions.begin(); ait != m_actions.end(); ait++)
        m_submenu->addAction(ait->second);
    }
};

#endif



