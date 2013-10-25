#include "GLWidgetOptions.h"

#include <iostream>
#include <sstream>

#include <QAction>
#include <QToolButton>
#include <QPixmap>
#include <QTranslator>
#include <QStatusBar>
#include <QColorDialog>
#include <QCursor>

#include "CameraPosDialog.h"
#include "GLWidget.h"
#include "MainWindow.h"
#include "Utilities/Camera.h"

#include "Icons/ResetCamera.xpm"
#include "Icons/BgColor.xpm"

GLWidgetOptions::GLWidgetOptions(QWidget* _parent, MainWindow* _mainWindow)
  : OptionsBase(_parent, _mainWindow) {
    CreateActions();
    SetUpSubmenu("Scene");
    SetUpToolbar();
    SetHelpTips();
  }

void
GLWidgetOptions::CreateActions(){

  //1. Create actions and add them to the map
  m_actions["showAxis"] = new QAction(tr("Axis"), this);
  m_actions["showFrameRate"] = new QAction(tr("Theoretical Frame Rate"), this);
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
  m_actions["showFrameRate"]->setCheckable(true);
  m_actions["resetCamera"]->setEnabled(false);
  m_actions["resetCamera"]->setStatusTip(tr("Reset the camera position"));
  m_actions["changeBGColor"]->setEnabled(false);
  m_actions["changeBGColor"]->setStatusTip(tr("Change the color of the background"));

  //3. Make connections
  connect(m_actions["showAxis"], SIGNAL(triggered()),
      m_mainWindow->GetGLScene(), SLOT(ShowAxis()));
  connect(m_actions["showFrameRate"], SIGNAL(triggered()),
      m_mainWindow->GetGLScene(), SLOT(ShowFrameRate()));
  connect(m_actions["resetCamera"], SIGNAL(triggered()), this, SLOT(ResetCamera()));
  connect(m_actions["setCameraPosition"], SIGNAL(triggered()), this, SLOT(SetCameraPosition()));
  connect(m_actions["changeBGColor"], SIGNAL(triggered()), this, SLOT(ChangeBGColor()));
  connect(m_mainWindow->GetGLScene(), SIGNAL(clickByRMB()), this, SLOT(ShowGeneralContextMenu()));
}

void
GLWidgetOptions::SetUpToolbar(){
  m_toolbar = new QToolBar(m_mainWindow);
  m_toolbar->addAction(m_actions["resetCamera"]);
  m_toolbar->addAction(m_actions["changeBGColor"]);
}

void
GLWidgetOptions::Reset(){
  m_actions["resetCamera"]->setEnabled(true);
  m_actions["changeBGColor"]->setEnabled(true);
}

void
GLWidgetOptions::SetHelpTips(){
  m_actions["changeBGColor"]->setWhatsThis(tr("Click this button to"
    " change the <b>background</b> color in the scene."));
  m_actions["resetCamera"]->setWhatsThis(tr("Click this button to"
    " restore the default camera position. You can also manually specify"
    " a camera position with the <b>Set Camera Position</b> menu option."));
}

//Slots

void
GLWidgetOptions::ResetCamera(){
  m_mainWindow->GetGLScene()->ResetCamera();
  m_mainWindow->GetGLScene()->updateGL();
}

void
GLWidgetOptions::SetCameraPosition(){
  m_cameraPosInput->SetCamera(m_mainWindow->GetGLScene()->GetCurrentCamera());
  m_cameraPosInput->show();
}

void
GLWidgetOptions::ChangeBGColor(){

  QColor color = QColorDialog::getColor(Qt::white, this);
  if (color.isValid()){
    m_mainWindow->GetGLScene()->SetClearColor(
        (double)(color.red()) / 255.0,
        (double)(color.green()) / 255.0,
        (double)(color.blue()) / 255.0);
    m_mainWindow->GetGLScene()->updateGL();
  }
}

void
GLWidgetOptions::ShowGeneralContextMenu(){

  QMenu cm(this);
  cm.addAction(m_actions["changeBGColor"]);
  cm.addAction(m_actions["resetCamera"]);
  cm.addAction(m_actions["setCameraPosition"]);
  cm.addAction(m_actions["showAxis"]);
  cm.addAction(m_actions["showFrameRate"]);

  if(cm.exec(QCursor::pos()) != 0)
    m_mainWindow->GetGLScene()->updateGL();
}
