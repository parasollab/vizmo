#include "PathOptions.h"

#include <QtGui>

#include "CustomizePathDialog.h"
#include "GLWidget.h"
#include "MainWindow.h"

#include "Models/PathModel.h"
#include "Models/Vizmo.h"

#include "Icons/Path.xpm"
#include "Icons/PathOptions.xpm"


PathOptions::
PathOptions(QWidget* _parent) : OptionsBase(_parent, "Path"),
    m_customizePathDialog(NULL) {
  CreateActions();
  SetHelpTips();
  SetUpSubmenu();
  SetUpToolTab();
}

/*----------------------------- GUI Management -------------------------------*/

void
PathOptions::
CreateActions() {
  //1. Create actions and add them to map
  m_actions["showHidePath"] = new QAction(QPixmap(path),
      tr("Show/Hide Path"), this);
  m_actions["pathOptions"] = new QAction(QPixmap(pathoptions),
      tr("Path Display Options"), this);

  //2. Set other specifications as necessary
  m_actions["showHidePath"]->setCheckable(true);
  m_actions["showHidePath"]->setEnabled(false);
  m_actions["pathOptions"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["showHidePath"], SIGNAL(triggered()),
      this, SLOT(ShowHidePath()));
  connect(m_actions["pathOptions"], SIGNAL(triggered()),
      this, SLOT(PathDisplayOptions()));
}


void
PathOptions::
SetHelpTips() {
  m_actions["showHidePath"]->setStatusTip(tr("Show or hide the path"));
  m_actions["showHidePath"]->setWhatsThis(tr("Click this button to visualize"
      " the <b>Path</b>.<br> You can also select the <b>Show/Hide Path</b>"
      " option from the <b>Path</b> menu."));
}


void
PathOptions::
Reset() {
  if(m_actions["showHidePath"] != NULL) {
    m_actions["showHidePath"]->setEnabled(GetVizmo().IsPathLoaded());
    m_actions["pathOptions"]->setEnabled(GetVizmo().IsPathLoaded());
    m_actions["showHidePath"]->setChecked(false);
  }
  if(m_customizePathDialog != NULL) {
    m_customizePathDialog->RestoreDefault();
    m_customizePathDialog->GetWidthLine()->setText("1");
    m_customizePathDialog->GetModLine()->setText("3");
  }
}

/*----------------------------- Path Functions -------------------------------*/

void
PathOptions::
ShowHidePath() {
  GetVizmo().GetPath()->SetRenderMode(m_actions["showHidePath"]->isChecked() ?
      SOLID_MODE : INVISIBLE_MODE);
  GetMainWindow()->GetGLWidget()->updateGL();
}


void
PathOptions::
PathDisplayOptions() {
  if(m_customizePathDialog == NULL) {
    m_customizePathDialog = new CustomizePathDialog(GetMainWindow());
    GetMainWindow()->ShowDialog(m_customizePathDialog);
  }
}
