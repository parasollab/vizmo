#include "PathOptions.h"

#include <QAction>
#include <QToolBar>
#include <QPixmap>
#include <QTranslator>

#include "MainWin.h"
#include "SceneWin.h"
#include "CustomizePathDialog.h"
#include "QueryGUI.h"
#include "Models/Vizmo.h"

#include "Icons/Pen.xpm"
#include "Icons/Flag.xpm"

PathOptions::PathOptions(QWidget* _parent, VizmoMainWin* _mainWin)
  : OptionsBase(_parent, _mainWin) {
    CreateActions();
    SetUpCustomSubmenu();
    SetUpToolbar();
    SetHelpTips();
  }

void
PathOptions::CreateActions() {

  //1. Create actions and add them to map
  QAction* showHidePath = new QAction(QPixmap(pen), tr("Show/Hide Path"), this);
  m_actions["showHidePath"] = showHidePath;
  QAction* showHideSG = new QAction(QPixmap(flag), tr("Show/Hide Start/Goal"), this);
  m_actions["showHideSG"] = showHideSG;
  QAction* pathOptions = new QAction(tr("Path Display Options"), this);
  m_actions["pathOptions"] = pathOptions;
  m_pathOptionsInput = new CustomizePathDialog(this);          //pop-up window

  //2. Set other specifications as necessary
  m_actions["showHidePath"]->setEnabled(false);
  m_actions["showHidePath"]->setStatusTip(tr("Show or hide the path"));
  m_actions["showHideSG"]->setEnabled(false);
  m_actions["showHideSG"]->setStatusTip(tr("Show or hide the start and goal"));
  m_actions["pathOptions"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["showHidePath"], SIGNAL(activated()), this, SLOT(ShowHidePath()));
  connect(m_actions["showHideSG"], SIGNAL(activated()), this, SLOT(ShowHideStartGoal()));
  connect(m_actions["pathOptions"], SIGNAL(activated()), this, SLOT(PathDisplayOptions()));
}

void
PathOptions::SetUpCustomSubmenu(){

  m_submenu = new QMenu("Path", this);

  m_submenu->addAction(m_actions["showHidePath"]);
  m_submenu->addAction(m_actions["showHideSG"]);
  m_submenu->addAction(m_actions["pathOptions"]);
}

void
PathOptions::SetUpToolbar(){

  m_toolbar = new QToolBar(GetMainWin());

  m_toolbar->addAction(m_actions["showHidePath"]);
  m_toolbar->addAction(m_actions["showHideSG"]);
}

void
PathOptions::Reset(){

  if(m_actions["showHidePath"] != NULL){
    if(GetVizmo().GetPathSize()==0)
      m_actions["showHidePath"]->setEnabled(false);
    else{
      m_actions["showHidePath"]->setEnabled(true);
      m_actions["pathOptions"]->setEnabled(true);
    }
  }

  if(m_actions["showHideSG"] != NULL){
    if(GetVizmo().IsQueryLoaded())
      m_actions["showHideSG"]->setEnabled(true);
    else
      m_actions["showHideSG"]->setEnabled(false);
  }

  m_pathOptionsInput->RestoreDefault();
  //Statements below should NOT be moved into RestoreDefault()
  //RestoreDefault is for colors only
  m_pathOptionsInput->m_widthLineEdit->setText("1");
  m_pathOptionsInput->m_modLineEdit->setText("3");  //Display interval
}

void
PathOptions::SetHelpTips(){
  m_actions["showHidePath"]->setWhatsThis(tr("Click this button to visualize"
        " the <b>Path</b>.<br> You can also select the <b>Show/Hide Path</b> option"
        " from the <b>Path</b> menu."));

  m_actions["showHideSG"]->setWhatsThis(tr("Click this button to visualize the"
        " <b>Start and Goal</b> configurations.<br>"
        "You can also select the <b>Show/Hide Start/Goal</b> option "
        "from the <b>Path</b> menu."));
}

//Slots
void
PathOptions::ShowHidePath(){
  static bool show=false;
  show=!show;
  GetVizmo().ShowPathFrame(show);
  GetMainWin()->GetGLScene()->updateGL();
}

void
PathOptions::ShowHideStartGoal(){
  static bool show=false;
  show=!show;
  GetVizmo().ShowQueryFrame(show);
  GetMainWin()->GetGLScene()->updateGL();
}

void
PathOptions::PathDisplayOptions(){
  //Pop up the path customization window
  m_pathOptionsInput->show();
}

