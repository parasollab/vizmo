#include "QueryOptions.h"

#include <QtGui>
#include <QAction>

#include "CustomizePathDialog.h"
#include "GLWidget.h"
#include "MainWindow.h"
#include "NodeEditDialog.h"
#include "QueryEditDialog.h"

#include "Models/QueryModel.h"
#include "Models/Vizmo.h"

#include "Icons/EditQuery.xpm"
#include "Icons/Flag.xpm"


QueryOptions::
QueryOptions(QWidget* _parent) : OptionsBase(_parent, "Query"),
    m_queryEditDialog(NULL) {
  CreateActions();
  SetHelpTips();
  SetUpSubmenu();
  SetUpToolTab();
}

/*-------------------------- GUI Management ----------------------------------*/

void
QueryOptions::
CreateActions() {
  //1. Create actions and add them to map
  m_actions["showHideQuery"] = new QAction(QPixmap(flag),
      tr("Show/Hide Query"), this);
  m_actions["editQuery"] = new QAction(QPixmap(editquery),
      tr("Edit Query"), this);

  //2. Set other specifications as necessary
  m_actions["showHideQuery"]->setCheckable(true);
  m_actions["showHideQuery"]->setEnabled(false);
  m_actions["editQuery"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["showHideQuery"], SIGNAL(triggered()),
      this, SLOT(ShowHideQuery()));
  connect(m_actions["editQuery"], SIGNAL(triggered()), this, SLOT(EditQuery()));
}


void
QueryOptions::
SetHelpTips() {
  m_actions["showHideQuery"]->setStatusTip(tr("Show or hide the query"));
  m_actions["showHideQuery"]->setWhatsThis(tr("Click this button to visualize the"
        " <b>Start and Goal</b> configurations.<br>"
        "You can also select the <b>Show/Hide Start/Goal</b> option "
        "from the <b>Path</b> menu."));

  m_actions["editQuery"]->setStatusTip(tr("Edit the query"));
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

/*-------------------------- Query Functions ---------------------------------*/

void
QueryOptions::
ShowHideQuery() {
  GetVizmo().GetQry()->SetRenderMode(m_actions["showHideQuery"]->isChecked() ?
      SOLID_MODE : INVISIBLE_MODE);
}


void
QueryOptions::
EditQuery() {
  if(m_queryEditDialog == NULL) {
    m_actions["showHideQuery"]->setChecked(true);
    GetVizmo().GetQry()->SetRenderMode(SOLID_MODE);
    m_queryEditDialog = new QueryEditDialog(GetMainWindow(), GetVizmo().GetQry());
    GetMainWindow()->ShowDialog(m_queryEditDialog);
  }
}
