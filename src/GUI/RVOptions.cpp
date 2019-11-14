#include "RVOptions.h"

#include <QtGui>
#include <QAction>

#include "CustomizePathDialog.h"
#include "GLWidget.h"
#include "MainWindow.h"

#include "Models/RVModel.h"
#include "Models/Vizmo.h"

#include "Icons/Navigate.xpm"
#include "Icons/Next.xpm"
#include "Icons/Previous.xpm"


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
  // buttons ordered alphabetically by action keyword
  m_actions["displayHideRV"] = new QAction(QPixmap(navigate),
      tr("Show/Hide RV"), this);

  m_actions["nextRV"] = new QAction(QPixmap(next1),
      tr("Display next RV"), this);

  m_actions["prevRV"] = new QAction(QPixmap(previous),
      tr("Display previous RV"), this);

  //2. Set other specifications as necessary
  m_actions["displayHideRV"]->setCheckable(true);
  m_actions["displayHideRV"]->setEnabled(false);

  m_actions["nextRV"]->setEnabled(false);
  m_actions["prevRV"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["displayHideRV"], SIGNAL(triggered()),
      this, SLOT(ShowHideRV()));

  connect(m_actions["nextRV"], SIGNAL(triggered()),
      this, SLOT(NextRV()));

  connect(m_actions["prevRV"], SIGNAL(triggered()),
      this, SLOT(PrevRV()));
}


void
RVOptions::
SetHelpTips() {
  m_actions["displayHideRV"]->setStatusTip(tr("Show or hide the rv map"));
  m_actions["displayHideRV"]->setWhatsThis(tr("Click this button to visualize the"
        " <b>Reachable Volume</b> map.<br>"
        "You can also select the <b>Show/Hide RV</b> option "
        "from the <b>Path</b> menu."));

  m_actions["nextRV"]->setStatusTip(tr("Display the next RV"));
  m_actions["nextRV"]->setWhatsThis(tr("If there are multiple Reachable Volumes, scroll to the next one."));

  m_actions["prevRV"]->setStatusTip(tr("Display the previous RV"));
  m_actions["prevRV"]->setWhatsThis(tr("If there are multiple Reachable Volumes, scroll to the previous one."));
}


void
RVOptions::
Reset() {
  if(m_actions["displayHideRV"] != NULL) {
    m_actions["displayHideRV"]->setEnabled(GetVizmo().IsRVLoaded());
    m_actions["displayHideRV"]->setChecked(false);
  }

  if(m_actions["nextRV"] != NULL) {
    m_actions["nextRV"]->setEnabled(GetVizmo().IsRVLoaded());
    m_actions["nextRV"]->setChecked(false);
  }

  if(m_actions["prevRV"] != NULL) {
    m_actions["prevRV"]->setEnabled(GetVizmo().IsRVLoaded());
    m_actions["prevRV"]->setChecked(false);
  }

}

/*-------------------------- RV Functions ---------------------------------*/

void
RVOptions::
ShowHideRV() {
  GetVizmo().GetRV()->SetRenderMode(m_actions["displayHideRV"]->isChecked() ?
      SOLID_MODE : INVISIBLE_MODE);
}

void
RVOptions::
NextRV() {
  RVModel* rvModel = GetVizmo().GetRV();
  rvModel->SetRenderMode(INVISIBLE_MODE);
  rvModel->IncrementIndex();
  rvModel->SetRenderMode(SOLID_MODE);
}

void
RVOptions::
PrevRV() {
  RVModel* rvModel = GetVizmo().GetRV();
  rvModel->SetRenderMode(INVISIBLE_MODE);
  rvModel->DecrementIndex();
  rvModel->SetRenderMode(SOLID_MODE);
}
