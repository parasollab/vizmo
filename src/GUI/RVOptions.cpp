#include "RVOptions.h"

#include <QtGui>
#include <QAction>

#include "CustomizePathDialog.h"
#include "GLWidget.h"
#include "MainWindow.h"

#include "Models/RVModel.h"
#include "Models/Vizmo.h"

#include "Icons/Flag.xpm"


RVOptions::
RVOptions(QWidget* _parent) : OptionsBase(_parent, "RV") {
  CreateActions();
  SetHelpTips();
  SetUpSubmenu();
  SetUpToolTab();
}

/*-------------------------- GUI Management ----------------------------------*/

void
RVOptions::
CreateActions() {
  //1. Create actions and add them to map
  m_actions["showHideRV"] = new QAction(QPixmap(flag),
      tr("Show/Hide RV"), this);

  m_actions["nextRV"] = new QAction(QPixmap(flag),
      tr("Display next RV"), this);

  //2. Set other specifications as necessary
  m_actions["showHideRV"]->setCheckable(true);
  m_actions["showHideRV"]->setEnabled(false);

  m_actions["nextRV"]->setCheckable(true);
  m_actions["nextRV"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["showHideRV"], SIGNAL(triggered()),
      this, SLOT(ShowHideRV()));

  connect(m_actions["nextRV"], SIGNAL(triggered()),
      this, SLOT(NextRV()));
}


void
RVOptions::
SetHelpTips() {
  m_actions["showHideRV"]->setStatusTip(tr("Show or hide the rv map"));
  m_actions["showHideRV"]->setWhatsThis(tr("Click this button to visualize the"
        " <b>Reachable Volume</b> map.<br>"
        "You can also select the <b>Show/Hide RV</b> option "
        "from the <b>Path</b> menu."));

  m_actions["nextRV"]->setStatusTip(tr("Display the next RV"));
  m_actions["nextRV"]->setWhatsThis(tr("If there are multiple Reachable Volumes, scroll to the next one."));
}


void
RVOptions::
Reset() {
  if(m_actions["showHideRV"] != NULL) {
    m_actions["showHideRV"]->setEnabled(GetVizmo().IsRVLoaded());
    m_actions["showHideRV"]->setChecked(false);
  }
}

/*-------------------------- RV Functions ---------------------------------*/

void
RVOptions::
ShowHideRV() {
  GetVizmo().GetRV()->SetRenderMode(m_actions["showHideRV"]->isChecked() ?
      SOLID_MODE : INVISIBLE_MODE);
}

void
RVOptions::
NextRV() {
  GetVizmo().GetRV()->SetRenderMode(m_actions["showHideRV"]->isChecked() ?
      SOLID_MODE : INVISIBLE_MODE);
}
