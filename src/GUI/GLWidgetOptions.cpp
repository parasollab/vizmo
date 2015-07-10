#include "GLWidgetOptions.h"

#include <iostream>
#include <sstream>

#include <QtGui>

#include "CameraPosDialog.h"
#include "GLWidget.h"
#include "MainWindow.h"

#include "Models/EnvModel.h"
#include "Models/MapModel.h"
#include "Models/ActiveMultiBodyModel.h"
#include "Models/Vizmo.h"

#include "Utilities/Camera.h"

#include "Icons/Axis.xpm"
#include "Icons/BgColor.xpm"
#include "Icons/FrameRate.xpm"
#include "Icons/FreeFloatingCamera.xpm"
#include "Icons/MakeInvisible.xpm"
#include "Icons/MakeSolid.xpm"
#include "Icons/MakeWired.xpm"
#include "Icons/Pallet.xpm"
#include "Icons/ResetCamera.xpm"
#include "Icons/ResetCameraUp.xpm"
#include "Icons/SaveCameraPosition.xpm"
#include "Icons/SetCameraPosition.xpm"
#include "Icons/ShowNormals.xpm"


GLWidgetOptions::
GLWidgetOptions(QWidget* _parent) : OptionsBase(_parent, "Display"),
    m_cameraPosDialog(NULL) {
  CreateActions();
  SetHelpTips();
  SetUpSubmenu();
  SetUpToolTab();
}

/*----------------------------- GUI Management -------------------------------*/

void
GLWidgetOptions::
CreateActions() {
  //1. Create actions and add them to the map
  m_actions["showAxis"] = new QAction(QPixmap(axisicon),
      tr("Axis"), this);
  m_actions["showFrameRate"] = new QAction(QPixmap(framerate),
      tr("Theoretical Frame Rate"), this);
  m_actions["resetCamera"] = new QAction(QPixmap(resetcamera),
      tr("Reset Camera"), this);
  m_actions["setCameraPosition"] = new QAction(QPixmap(setcameraposition),
      tr("Set camera position"), this);
  m_actions["saveCameraPosition"] = new QAction(QPixmap(savecameraposition),
      tr("Save camera position"), this);
  m_actions["loadCameraPosition"] = new QAction(QPixmap(savecameraposition),
      tr("Load camera position"), this);
  m_actions["toggleCameraFree"] = new QAction(QPixmap(freeFloatingCamera),
      tr("Toggle free-floating camera."), this);
  m_actions["resetCameraUp"] = new QAction(QPixmap(resetCameraUp),
      tr("Reset camera up direction."), this);
  m_actions["changeBGColor"] = new QAction(QPixmap(bgColor),
      tr("Change background color"), this);
  m_actions["makeSolid"] = new QAction(QPixmap(makeSolidIcon),
      tr("Make Solid"), this);
  m_actions["makeWired"] = new QAction(QPixmap(makeWiredIcon),
      tr("Make Wired"), this);
  m_actions["makeInvisible"] = new QAction(QPixmap(makeInvisibleIcon),
      tr("Make Invisible"), this);
  m_actions["changeObjectColor"] = new QAction(QPixmap(pallet),
      tr("Change Color"), this);
  m_actions["showObjectNormals"] = new QAction(QPixmap(showNormals),
      tr("Show Normals"), this);

  //2. Set other specifications as necessary
  m_actions["showAxis"]->setCheckable(true);
  m_actions["showAxis"]->setChecked(true);
  m_actions["showFrameRate"]->setCheckable(true);
  m_actions["resetCamera"]->setEnabled(false);
  m_actions["setCameraPosition"]->setEnabled(false);
  m_actions["changeBGColor"]->setEnabled(false);
  m_actions["saveCameraPosition"]->setEnabled(false);
  m_actions["loadCameraPosition"]->setEnabled(false);
  m_actions["toggleCameraFree"]->setEnabled(false);
  m_actions["toggleCameraFree"]->setCheckable(true);
  m_actions["resetCameraUp"]->setEnabled(false);
  m_actions["makeSolid"]->setShortcut(tr("CTRL+F"));
  m_actions["makeSolid"]->setEnabled(false);
  m_actions["makeWired"]->setShortcut(tr("CTRL+W"));
  m_actions["makeWired"]->setEnabled(false);
  m_actions["makeInvisible"]->setShortcut(tr("CTRL+N"));
  m_actions["makeInvisible"]->setEnabled(false);
  m_actions["changeObjectColor"]->setEnabled(false);
  m_actions["showObjectNormals"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["resetCamera"], SIGNAL(triggered()),
      this, SLOT(ResetCamera()));
  connect(m_actions["setCameraPosition"], SIGNAL(triggered()),
      this, SLOT(SetCameraPosition()));
  connect(m_actions["saveCameraPosition"], SIGNAL(triggered()),
      this, SLOT(SaveCameraPosition()));
  connect(m_actions["loadCameraPosition"], SIGNAL(triggered()),
      this, SLOT(LoadCameraPosition()));
  connect(m_actions["toggleCameraFree"], SIGNAL(triggered()),
      this, SLOT(ToggleCameraFree()));
  connect(m_actions["resetCameraUp"], SIGNAL(triggered()),
      this, SLOT(ResetCameraUp()));
  connect(m_actions["changeBGColor"], SIGNAL(triggered()),
      this, SLOT(ChangeBGColor()));
  connect(m_actions["makeSolid"], SIGNAL(triggered()), this, SLOT(MakeSolid()));
  connect(m_actions["makeWired"], SIGNAL(triggered()), this, SLOT(MakeWired()));
  connect(m_actions["makeInvisible"], SIGNAL(triggered()),
      this, SLOT(MakeInvisible()));
  connect(m_actions["changeObjectColor"], SIGNAL(triggered()),
      this, SLOT(ChangeObjectColor()));
  connect(m_actions["showObjectNormals"], SIGNAL(triggered()),
      this, SLOT(ShowObjectNormals()));
  connect(m_actions["showAxis"], SIGNAL(triggered()),
      GetMainWindow()->GetGLWidget(), SLOT(ShowAxis()));
  connect(m_actions["showFrameRate"], SIGNAL(triggered()),
      GetMainWindow()->GetGLWidget(), SLOT(ShowFrameRate()));
  connect(GetMainWindow()->GetGLWidget(), SIGNAL(clickByRMB()),
      this, SLOT(ShowGeneralContextMenu()));
  connect(GetMainWindow()->GetGLWidget(), SIGNAL(selectByRMB()),
      this, SLOT(ShowGeneralContextMenu()));
}


void
GLWidgetOptions::
SetHelpTips() {
  m_actions["changeBGColor"]->setStatusTip(tr("Change the background color"));
  m_actions["changeBGColor"]->setWhatsThis(tr("Click this button to"
        " change the <b>background</b> color in the scene."));

  m_actions["changeObjectColor"]->setStatusTip(tr("Change object color"));
  m_actions["changeObjectColor"]->setWhatsThis(tr("Click this button to change"
        " the color of a selected item."));

  m_actions["showObjectNormals"]->setStatusTip(tr("Show model normals"));
  m_actions["showObjectNormals"]->setWhatsThis(tr("Click this button to show"
        " the normals within a model of a selected item."));

  m_actions["resetCamera"]->setStatusTip(tr("Reset the camera position"));
  m_actions["resetCamera"]->setWhatsThis(tr("Click this button to"
        " restore the default camera position. You can also manually specify"
        " a camera position with the <b>Set Camera Position</b> menu option."));

  m_actions["makeSolid"]->setStatusTip(tr("Change to solid displacy mode"));
  m_actions["makeSolid"]->setWhatsThis(tr("Click this button to display a"
        " selected item in <b>Solid</b> mode."));

  m_actions["makeWired"]->setStatusTip(tr("Change to wired displacy mode"));
  m_actions["makeWired"]->setWhatsThis(tr("Click this button to display a"
        " selected item in <b>Wire</n> mode."));

  m_actions["makeInvisible"]->setStatusTip(tr("Change to invisible"));
  m_actions["makeInvisible"]->setWhatsThis(tr("Click this button to make a"
        " selected item invisible."));

  m_actions["saveCameraPosition"]->setWhatsThis(tr("Click this button Save the "
        " camera position."));

  m_actions["loadCameraPosition"]->setWhatsThis(tr("Click this button load the "
        " camera position."));

  m_actions["toggleCameraFree"]->setWhatsThis(tr("Switch between free-floating "
        "and restricted camera control."));

  m_actions["resetCameraUp"]->setWhatsThis(tr("Reset the camera up direction"));
}


void
GLWidgetOptions::
SetUpSubmenu() {
  m_modifySelected = new QMenu("Modify Selected Item", this);
  m_modifySelected->addAction(m_actions["makeSolid"]);
  m_modifySelected->addAction(m_actions["makeWired"]);
  m_modifySelected->addAction(m_actions["makeInvisible"]);
  m_modifySelected->setEnabled(false);
  m_submenu = new QMenu(m_label, this);
  m_submenu->addMenu(m_modifySelected);
  m_submenu->addAction(m_actions["changeObjectColor"]);
  m_submenu->addAction(m_actions["showObjectNormals"]);
  m_submenu->addAction(m_actions["resetCamera"]);
  m_submenu->addAction(m_actions["changeBGColor"]);
}


void
GLWidgetOptions::
SetUpToolTab() {
  vector<string> buttonList;
  buttonList.push_back("setCameraPosition");
  buttonList.push_back("saveCameraPosition");
  buttonList.push_back("loadCameraPosition");
  buttonList.push_back("resetCamera");
  buttonList.push_back("resetCameraUp");
  buttonList.push_back("toggleCameraFree");
  buttonList.push_back("_separator_");
  buttonList.push_back("makeSolid");
  buttonList.push_back("makeWired");
  buttonList.push_back("makeInvisible");
  buttonList.push_back("changeObjectColor");
  buttonList.push_back("changeBGColor");
  buttonList.push_back("_separator_");
  buttonList.push_back("showAxis");
  buttonList.push_back("showFrameRate");
  buttonList.push_back("showObjectNormals");
  CreateToolTab(buttonList);
}


void
GLWidgetOptions::
Reset() {
  m_actions["resetCamera"]->setEnabled(true);
  m_actions["setCameraPosition"]->setEnabled(true);
  m_actions["changeBGColor"]->setEnabled(true);
  m_actions["changeObjectColor"]->setEnabled(true);
  m_modifySelected->setEnabled(true);
  m_actions["makeSolid"]->setEnabled(true);
  m_actions["makeWired"]->setEnabled(true);
  m_actions["makeInvisible"]->setEnabled(true);
  m_actions["showObjectNormals"]->setEnabled(true);
  m_actions["saveCameraPosition"]->setEnabled(true);
  m_actions["loadCameraPosition"]->setEnabled(true);
  m_actions["resetCameraUp"]->setEnabled(true);
  m_actions["toggleCameraFree"]->setEnabled(true);
}

/*----------------------------- GL Functions ---------------------------------*/

void
GLWidgetOptions::
ResetCamera() {
  GetMainWindow()->GetGLWidget()->ResetCamera();
  GetMainWindow()->GetGLWidget()->updateGL();
}


void
GLWidgetOptions::
SetCameraPosition() {
  if(m_cameraPosDialog == NULL && GetVizmo().GetEnv() != NULL) {
    m_cameraPosDialog = new CameraPosDialog(GetMainWindow(),
        GetMainWindow()->GetGLWidget()->GetCurrentCamera());
    GetMainWindow()->ShowDialog(m_cameraPosDialog);
  }
}


void
GLWidgetOptions::
SaveCameraPosition() {
  if(m_cameraPosDialog == NULL && GetVizmo().GetEnv() != NULL) {
    m_cameraPosDialog = new CameraPosDialog(GetMainWindow(),
        GetMainWindow()->GetGLWidget()->GetCurrentCamera());
    m_cameraPosDialog->SaveCameraPosition();
  }
}


void
GLWidgetOptions::
LoadCameraPosition() {
  if(m_cameraPosDialog == NULL && GetVizmo().GetEnv() != NULL) {
    m_cameraPosDialog = new CameraPosDialog(GetMainWindow(),
        GetMainWindow()->GetGLWidget()->GetCurrentCamera());
    m_cameraPosDialog->LoadCameraPosition();
  }
}


void
GLWidgetOptions::
ResetCameraUp() {
  GetMainWindow()->GetGLWidget()->GetCurrentCamera()->ResetUp();
}


void
GLWidgetOptions::
ToggleCameraFree() {
  GetMainWindow()->GetGLWidget()->GetCurrentCamera()->ToggleFreeFloat();
}


void
GLWidgetOptions::
ChangeBGColor() {
  QColor color = QColorDialog::getColor(Qt::white, this);
  if (color.isValid()){
    GetMainWindow()->GetGLWidget()->SetClearColor(
        (double)(color.red()) / 255.0,
        (double)(color.green()) / 255.0,
        (double)(color.blue()) / 255.0);
    GetMainWindow()->GetGLWidget()->updateGL();
  }
}


void
GLWidgetOptions::
ShowGeneralContextMenu() {
  //create context menu
  QMenu cm(this);

  //get selected models
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  if(sel.empty()) {
    //show general display options when no models are selected
    cm.addAction(m_actions["showAxis"]);
    cm.addAction(m_actions["showFrameRate"]);
    cm.addAction(m_actions["changeBGColor"]);
    cm.addSeparator();
    cm.addAction(m_actions["resetCamera"]);
    cm.addAction(m_actions["loadCameraPosition"]);
    cm.addAction(m_actions["saveCameraPosition"]);
    cm.addAction(m_actions["setCameraPosition"]);
  }
  else {
    //determine whether cfgs or other models are selected
    bool cfgsSelected = false;
    bool otherSelected = false;
    for(auto i : sel) {
      if(i->Name().find("Node") != string::npos ||
          i->Name().find("Edge") != string::npos)
        cfgsSelected = true;
      else
        otherSelected = true;
    }
    if(!cfgsSelected && otherSelected) {
      cm.addAction(m_actions["makeSolid"]);
      cm.addAction(m_actions["makeWired"]);
      cm.addAction(m_actions["makeInvisible"]);
    }
  }

  //show menu
  cm.exec(QCursor::pos());
}


void
GLWidgetOptions::
MakeSolid() {
  for(auto& i : GetVizmo().GetSelectedModels())
    i->SetRenderMode(SOLID_MODE);
}


void
GLWidgetOptions::
MakeWired() {
  for(auto& i : GetVizmo().GetSelectedModels())
    i->SetRenderMode(WIRE_MODE);
}


void
GLWidgetOptions::
MakeInvisible() {
  for(auto& i : GetVizmo().GetSelectedModels())
    i->SetRenderMode(INVISIBLE_MODE);
}


void
GLWidgetOptions::
ChangeObjectColor() {
  QColor color = QColorDialog::getColor(Qt::white, this, "color dialog");
  double r, g, b;
  if(color.isValid()) {
    r = color.red() / 255.0;
    g = color.green() / 255.0;
    b = color.blue() / 255.0;
  }
  else
    return;

  for(auto& i : GetVizmo().GetSelectedModels())
    i->SetColor(Color4(r, g, b, 1));
}


void
GLWidgetOptions::
ShowObjectNormals() {
  for(auto& i : GetVizmo().GetSelectedModels())
    i->ToggleNormals();
}
