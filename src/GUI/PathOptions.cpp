#include "PathOptions.h"

#include <QtGui>

#include "MainWindow.h"
#include "GLWidget.h"
#include "CustomizePathDialog.h"

#include "Models/PathModel.h"
#include "Models/Vizmo.h"

#include "Icons/Path.xpm"
#include "Icons/PathOptions.xpm"

PathOptions::
PathOptions(QWidget* _parent, MainWindow* _mainWindow)
    : OptionsBase(_parent, _mainWindow), m_customizePathDialog(NULL) {
  CreateActions();
  SetUpCustomSubmenu();
  //SetUpToolbar(); currently using tool tabs
  SetUpToolTab();
  SetHelpTips();
}

void
PathOptions::
CreateActions() {

  //1. Create actions and add them to map
  QAction* showHidePath = new QAction(QPixmap(path), tr("Show/Hide Path"), this);
  m_actions["showHidePath"] = showHidePath;
  QAction* pathOptions = new QAction(QPixmap(pathoptions), tr("Path Display Options"), this);
  m_actions["pathOptions"] = pathOptions;

  //2. Set other specifications as necessary
  m_actions["showHidePath"]->setCheckable(true);
  m_actions["showHidePath"]->setEnabled(false);
  m_actions["showHidePath"]->setStatusTip(tr("Show or hide the path"));
  m_actions["pathOptions"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["showHidePath"], SIGNAL(triggered()), this, SLOT(ShowHidePath()));
  connect(m_actions["pathOptions"], SIGNAL(triggered()), this, SLOT(PathDisplayOptions()));
}

void
PathOptions::
SetUpCustomSubmenu() {

  m_submenu = new QMenu("Path", this);

  m_submenu->addAction(m_actions["showHidePath"]);
  m_submenu->addAction(m_actions["pathOptions"]);
}

void
PathOptions::
SetUpToolbar() {

  m_toolbar = new QToolBar(m_mainWindow);

  m_toolbar->addAction(m_actions["showHidePath"]);
}

void
PathOptions::
SetUpToolTab() {
  vector<string> buttonList;
  buttonList.push_back("showHidePath");
  buttonList.push_back("pathOptions");
  CreateToolTab(buttonList);
}

void
PathOptions::
Reset() {

  if(m_actions["showHidePath"] != NULL){
    m_actions["showHidePath"]->setEnabled(GetVizmo().IsPathLoaded());
    m_actions["pathOptions"]->setEnabled(GetVizmo().IsPathLoaded());
    m_actions["showHidePath"]->setChecked(false);
  }

  if(m_customizePathDialog != NULL) {
    m_customizePathDialog->RestoreDefault();
    //Statements below should NOT be moved into RestoreDefault()
    //RestoreDefault is for colors only
    m_customizePathDialog->GetWidthLine()->setText("1");
    m_customizePathDialog->GetModLine()->setText("3");  //Display interval
  }
}

void
PathOptions::
SetHelpTips() {
  m_actions["showHidePath"]->setWhatsThis(tr("Click this button to visualize"
        " the <b>Path</b>.<br> You can also select the <b>Show/Hide Path</b> option"
        " from the <b>Path</b> menu."));
}

//Slots
void
PathOptions::
ShowHidePath() {
  GetVizmo().GetPath()->SetRenderMode(m_actions["showHidePath"]->isChecked() ? SOLID_MODE : INVISIBLE_MODE);
  m_mainWindow->GetGLWidget()->updateGL();
}

//Pop up the path customization window
void
PathOptions::
PathDisplayOptions() {
  if(m_customizePathDialog == NULL) {
    m_customizePathDialog = new CustomizePathDialog(m_mainWindow);
    m_mainWindow->ShowDialog(m_customizePathDialog);
  }
}

