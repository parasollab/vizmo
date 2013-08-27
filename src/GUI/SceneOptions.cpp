#include <QAction>
#include <QToolButton>
#include <QPixmap>
#include <QTranslator>
#include <QStatusBar>
#include <QColorDialog>
#include <QCursor>

#include <iostream>
#include <sstream>
#include "Utilities/GL/Camera.h"
#include "SceneOptions.h"
#include "MainWin.h"
#include "SceneWin.h"
#include "CameraPosDialog.h"
#include "Icons/ResetCamera.xpm"
#include "Icons/BgColor.xpm"

SceneOptions::SceneOptions(QWidget* _parent, VizmoMainWin* _mainWin)
  :OptionsBase(_parent, _mainWin)
{
  CreateActions();
  SetUpSubmenu("Scene");
  SetUpToolbar();
  SetHelpTips();
}

void
SceneOptions::CreateActions(){

  //1. Create actions and add them to the map
  QAction* showAxis = new QAction(tr("Axis"), this);
  m_actions["showAxis"] = showAxis;
  QAction* resetCamera = new QAction(QIcon(QPixmap(resetCameraIcon)), tr("Reset Camera"), this);
  m_actions["resetCamera"] = resetCamera;
  QAction* setCameraPosition = new QAction(tr("Set camera position"), this);
  m_cameraPosInput = new CameraPosDialog(this);
  m_actions["setCameraPosition"] = setCameraPosition;
  QAction* changeBGColor = new QAction(QIcon(QPixmap(bgColor)), tr("Change background color"), this);
  m_actions["changeBGColor"] = changeBGColor;

  //2. Set other specifications as necessary
  m_actions["showAxis"]->setCheckable(true);
  m_actions["showAxis"]->setChecked(true);
  m_actions["resetCamera"]->setEnabled(false);
  m_actions["resetCamera"]->setStatusTip(tr("Reset the camera position"));
  m_actions["changeBGColor"]->setEnabled(false);
  m_actions["changeBGColor"]->setStatusTip(tr("Change the color of the background"));

  //3. Make connections
  connect(m_actions["showAxis"], SIGNAL(triggered()), this, SLOT(ShowAxis()));
  connect(m_actions["resetCamera"], SIGNAL(triggered()), this, SLOT(ResetCamera()));
  connect(m_actions["setCameraPosition"], SIGNAL(triggered()), this, SLOT(SetCameraPosition()));
  connect(m_actions["changeBGColor"], SIGNAL(triggered()), this, SLOT(ChangeBGColor()));
  connect(GetMainWin()->GetGLScene(), SIGNAL(clickByRMB()), this, SLOT(ShowGeneralContextMenu()));
}

void
SceneOptions::SetUpToolbar(){

  m_toolbar = new QToolBar(GetMainWin());
  m_toolbar->addAction(m_actions["resetCamera"]);
  m_toolbar->addAction(m_actions["changeBGColor"]);
}

void
SceneOptions::Reset(){

  m_actions["resetCamera"]->setEnabled(true);
  m_actions["changeBGColor"]->setEnabled(true);
}

void
SceneOptions::SetHelpTips(){

  m_actions["changeBGColor"]->setWhatsThis(tr("Click this button to"
    " change the <b>background</b> color in the scene."));
  m_actions["resetCamera"]->setWhatsThis(tr("Click this button to"
    " restore the default camera position. You can also manually specify"
    " a camera position with the <b>Set Camera Position</b> menu option."));

}

//Slots

void
SceneOptions::ShowAxis(){

  GetMainWin()->GetGLScene()->showAxis();
}

void
SceneOptions::ResetCamera(){

  GetMainWin()->GetGLScene()->resetCamera();
  GetMainWin()->GetGLScene()->updateGL();
}

void
SceneOptions::SetCameraPosition(){

  Point3d p = GetCameraFactory().GetCurrentCamera()->GetCameraPos();
  //Unfortunately, points are defined backwards for x and y
  //We want "X=3" to mean that VIEWER has moved to X=3
  if(p[0] != 0) //otherwise displays '-0' !
    p[0] = -p[0];
  if(p[1] != 0)
    p[1] = -p[1];

  double azim = GetCameraFactory().GetCurrentCamera()->GetCameraAzim();
  double elev = GetCameraFactory().GetCurrentCamera()->GetCameraElev();

  m_cameraPosInput->SetCameraPos(p[0], p[1], p[2], azim, elev);

  m_cameraPosInput->show();
}

void
SceneOptions::ChangeBGColor(){

  QColor color = QColorDialog::getColor(Qt::white, this);
  if (color.isValid()){
    GetMainWin()->GetGLScene()->setClearColor(
        (double)(color.red()) / 255.0,
        (double)(color.green()) / 255.0,
        (double)(color.blue()) / 255.0);
    GetMainWin()->GetGLScene()->updateGL();
  }
}

void
SceneOptions::ShowGeneralContextMenu(){

  QMenu cm(this);
  cm.addAction(m_actions["changeBGColor"]);
  cm.addAction(m_actions["resetCamera"]);
  cm.addAction(m_actions["setCameraPosition"]);
  cm.addAction(m_actions["showAxis"]);

  if(cm.exec(QCursor::pos()) != 0)
    GetMainWin()->GetGLScene()->updateGL();
}
