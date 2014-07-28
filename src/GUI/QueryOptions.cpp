#include "QueryOptions.h"

#include <QtGui>

#include "MainWindow.h"
#include "GLWidget.h"
#include "CustomizePathDialog.h"
#include "QueryEditDialog.h"
#include "NodeEditDialog.h"

#include "Models/Vizmo.h"
#include "Models/QueryModel.h"

#include "Icons/Flag.xpm"
#include "Icons/EditQuery.xpm"

QueryOptions::
QueryOptions(QWidget* _parent, MainWindow* _mainWindow)
    : OptionsBase(_parent, _mainWindow), m_queryEditDialog(NULL) {
  CreateActions();
  SetUpCustomSubmenu();
  //SetUpToolbar(); currently using tool tabs
  SetUpToolTab();
  SetHelpTips();
}

void
QueryOptions::
CreateActions() {

  //1. Create actions and add them to map
  QAction* showHideQuery = new QAction(QPixmap(flag), tr("Show/Hide Query"), this);
  m_actions["showHideQuery"] = showHideQuery;
  QAction* editQuery = new QAction(QPixmap(editquery), tr("Edit Query"), this);
  m_actions["editQuery"] = editQuery;

  //2. Set other specifications as necessary
  m_actions["showHideQuery"]->setCheckable(true);
  m_actions["showHideQuery"]->setEnabled(false);
  m_actions["showHideQuery"]->setStatusTip(tr("Show or hide the query"));
  m_actions["editQuery"]->setEnabled(false);
  m_actions["editQuery"]->setStatusTip(tr("Edit the query"));

  //3. Make connections
  connect(m_actions["showHideQuery"], SIGNAL(triggered()), this, SLOT(ShowHideQuery()));
  connect(m_actions["editQuery"], SIGNAL(triggered()), this, SLOT(EditQuery()));
}

void
QueryOptions::
SetUpCustomSubmenu() {

  m_submenu = new QMenu("Query", this);

  m_submenu->addAction(m_actions["showHideQuery"]);
  m_submenu->addAction(m_actions["editQuery"]);
}

void
QueryOptions::
SetUpToolbar() {

  m_toolbar = new QToolBar(m_mainWindow);

  m_toolbar->addAction(m_actions["showHideQuery"]);
}

void
QueryOptions::
SetUpToolTab() {
  vector<string> buttonList;
  buttonList.push_back("showHideQuery");
  buttonList.push_back("editQuery");
  CreateToolTab(buttonList);
}

void
QueryOptions::
Reset() {
  if(m_actions["showHideQuery"] != NULL) {
    m_actions["showHideQuery"]->setEnabled(GetVizmo().IsQueryLoaded());
    m_actions["showHideQuery"]->setChecked(false);
    m_actions["editQuery"]->setEnabled(GetVizmo().IsQueryLoaded());
  }
}

void
QueryOptions::
SetHelpTips() {
  m_actions["showHideQuery"]->setWhatsThis(tr("Click this button to visualize the"
        " <b>Start and Goal</b> configurations.<br>"
        "You can also select the <b>Show/Hide Start/Goal</b> option "
        "from the <b>Path</b> menu."));
}

//Slots
void
QueryOptions::
ShowHideQuery() {
  GetVizmo().GetQry()->SetRenderMode(m_actions["showHideQuery"]->isChecked() ? SOLID_MODE : INVISIBLE_MODE);
  m_mainWindow->GetGLWidget()->updateGL();
}

void
QueryOptions::
EditQuery() {
  if(m_queryEditDialog == NULL) {
    m_actions["showHideQuery"]->setChecked(true);
    GetVizmo().GetQry()->SetRenderMode(SOLID_MODE);
    m_mainWindow->GetGLWidget()->updateGL();
    m_queryEditDialog = new QueryEditDialog(m_mainWindow, GetVizmo().GetQry());
    m_mainWindow->ShowDialog(m_queryEditDialog);
    m_mainWindow->GetGLWidget()->updateGL();
  }
}
