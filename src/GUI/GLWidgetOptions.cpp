#include "GLWidgetOptions.h"

#include <iostream>
#include <sstream>

#include <QtGui>

#include "CameraPosDialog.h"
#include "GLWidget.h"
#include "MainWindow.h"
#include "Models/EnvModel.h"
#include "Models/RobotModel.h"
#include "Models/Vizmo.h"
#include "Utilities/Camera.h"

#include "Icons/Axis.xpm"
#include "Icons/BgColor.xpm"
#include "Icons/FrameRate.xpm"
#include "Icons/MakeInvisible.xpm"
#include "Icons/MakeSolid.xpm"
#include "Icons/MakeWired.xpm"
#include "Icons/Pallet.xpm"
#include "Icons/ResetCamera.xpm"
#include "Icons/SetCameraPosition.xpm"
#include "Icons/ShowNormals.xpm"


GLWidgetOptions::
GLWidgetOptions(QWidget* _parent, MainWindow* _mainWindow)
  : OptionsBase(_parent, _mainWindow), m_cameraPosDialog(NULL) {
    CreateActions();
    SetUpCustomSubmenu();
    //SetUpToolbar(); currently using tool tabs
    SetUpToolTab();
    SetHelpTips();
  }

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
  m_actions["changeBGColor"] = new QAction(QPixmap(bgColor),
      tr("Change background color"), this);

  QAction* makeSolid = new QAction(QPixmap(makeSolidIcon), tr("Make Solid"), this);
  m_actions["makeSolid"] = makeSolid;
  QAction* makeWired = new QAction(QPixmap(makeWiredIcon), tr("Make Wired"), this);
  m_actions["makeWired"] = makeWired;
  QAction* makeInvisible = new QAction(QPixmap(makeInvisibleIcon), tr("Make Invisible"), this);
  m_actions["makeInvisible"] = makeInvisible;

  m_actions["changeObjectColor"] = new QAction(QPixmap(pallet), tr("Change Color"), this);

  m_actions["showObjectNormals"] = new QAction(QPixmap(showNormals), tr("Show Normals"), this);

  //2. Set other specifications as necessary
  m_actions["showAxis"]->setCheckable(true);
  m_actions["showAxis"]->setChecked(true);
  m_actions["showFrameRate"]->setCheckable(true);
  m_actions["resetCamera"]->setEnabled(false);
  m_actions["resetCamera"]->setStatusTip(tr("Reset the camera position"));
  m_actions["changeBGColor"]->setEnabled(false);
  m_actions["changeBGColor"]->setStatusTip(tr("Change the color of the background"));

  m_actions["makeSolid"]->setShortcut(tr("CTRL+F"));
  m_actions["makeSolid"]->setEnabled(false);
  m_actions["makeSolid"]->setStatusTip(tr("Change to solid displacy mode"));

  m_actions["makeWired"]->setShortcut(tr("CTRL+W"));
  m_actions["makeWired"]->setEnabled(false);
  m_actions["makeWired"]->setStatusTip(tr("Change to wired displacy mode"));

  m_actions["makeInvisible"]->setShortcut(tr("CTRL+N"));
  m_actions["makeInvisible"]->setEnabled(false);
  m_actions["makeInvisible"]->setStatusTip(tr("Change to invisible"));

  m_actions["changeObjectColor"]->setEnabled(false);
  m_actions["changeObjectColor"]->setStatusTip(tr("Change object color"));
  m_actions["showObjectNormals"]->setEnabled(false);
  m_actions["showObjectNormals"]->setStatusTip(tr("Show model normals"));


  //3. Make connections
  connect(m_actions["showAxis"], SIGNAL(triggered()),
      m_mainWindow->GetGLScene(), SLOT(ShowAxis()));
  connect(m_actions["showFrameRate"], SIGNAL(triggered()),
      m_mainWindow->GetGLScene(), SLOT(ShowFrameRate()));
  connect(m_actions["resetCamera"], SIGNAL(triggered()),
      this, SLOT(ResetCamera()));
  connect(m_actions["setCameraPosition"], SIGNAL(triggered()),
      this, SLOT(SetCameraPosition()));
  connect(m_actions["changeBGColor"], SIGNAL(triggered()),
      this, SLOT(ChangeBGColor()));
  connect(m_mainWindow->GetGLScene(), SIGNAL(clickByRMB()), this, SLOT(ShowGeneralContextMenu()));
  connect(m_actions["makeSolid"], SIGNAL(triggered()), this, SLOT(MakeSolid()));
  connect(m_actions["makeWired"], SIGNAL(triggered()), this, SLOT(MakeWired()));
  connect(m_actions["makeInvisible"], SIGNAL(triggered()), this, SLOT(MakeInvisible()));
  connect(m_actions["changeObjectColor"], SIGNAL(triggered()), this, SLOT(ChangeObjectColor()));
  connect(m_actions["showObjectNormals"], SIGNAL(triggered()), this, SLOT(ShowObjectNormals()));
}

void
GLWidgetOptions::
SetUpCustomSubmenu() {
  m_submenu = new QMenu("Display", this);

  m_modifySelected = new QMenu("Modify Selected Item", this);
  m_modifySelected->addAction(m_actions["makeSolid"]);
  m_modifySelected->addAction(m_actions["makeWired"]);
  m_modifySelected->addAction(m_actions["makeInvisible"]);
  m_submenu->addMenu(m_modifySelected);

  m_submenu->addAction(m_actions["changeObjectColor"]);
  m_submenu->addAction(m_actions["showObjectNormals"]);

  m_submenu->addAction(m_actions["resetCamera"]);
  m_submenu->addAction(m_actions["changeBGColor"]);

  m_modifySelected->setEnabled(false);
}

void
GLWidgetOptions::
SetUpToolbar() {
  m_toolbar = new QToolBar(m_mainWindow);

  m_toolbar->addAction(m_actions["makeSolid"]);
  m_toolbar->addAction(m_actions["makeWired"]);
  m_toolbar->addAction(m_actions["makeInvisible"]);
  m_toolbar->addAction(m_actions["changeObjectColor"]);
  m_toolbar->addAction(m_actions["showObjectNormals"]);
  m_toolbar->addAction(m_actions["resetCamera"]);
  m_toolbar->addAction(m_actions["changeBGColor"]);
}

void
GLWidgetOptions::
SetUpToolTab() {
  vector<string> buttonList;
  buttonList.push_back("setCameraPosition");
  buttonList.push_back("showAxis");
  buttonList.push_back("showFrameRate");
  buttonList.push_back("makeSolid");
  buttonList.push_back("makeWired");
  buttonList.push_back("makeInvisible");
  buttonList.push_back("changeObjectColor");
  buttonList.push_back("changeBGColor");
  buttonList.push_back("resetCamera");
  buttonList.push_back("showObjectNormals");
  CreateToolTab(buttonList);
}

void
GLWidgetOptions::
Reset() {
  m_actions["resetCamera"]->setEnabled(true);
  m_actions["changeBGColor"]->setEnabled(true);
  m_actions["changeObjectColor"]->setEnabled(true);
  m_modifySelected->setEnabled(true);
  m_actions["makeSolid"]->setEnabled(true);
  m_actions["makeWired"]->setEnabled(true);
  m_actions["makeInvisible"]->setEnabled(true);
  m_actions["showObjectNormals"]->setEnabled(true);
}

void
GLWidgetOptions::
SetHelpTips() {
  m_actions["changeBGColor"]->setWhatsThis(tr("Click this button to"
        " change the <b>background</b> color in the scene."));
  m_actions["changeObjectColor"]->setWhatsThis(tr("Click this button to change"
        " the color of a selected item."));
  m_actions["showObjectNormals"]->setWhatsThis(tr("Click this button to show"
        " the normals within a model of a selected item."));
  m_actions["resetCamera"]->setWhatsThis(tr("Click this button to"
        " restore the default camera position. You can also manually specify"
        " a camera position with the <b>Set Camera Position</b> menu option."));

  m_actions["makeSolid"]->setWhatsThis(tr("Click this button to display a"
        " selected item in <b>Solid</b> mode."));
  m_actions["makeWired"]->setWhatsThis(tr("Click this button to display a"
        " selected item in <b>Wire</n> mode."));
  m_actions["makeInvisible"]->setWhatsThis(tr("Click this button to make a"
        " selected item invisible."));
}

//Slots

void
GLWidgetOptions::
ResetCamera() {
  m_mainWindow->GetGLScene()->ResetCamera();
  m_mainWindow->GetGLScene()->updateGL();
}

void
GLWidgetOptions::
SetCameraPosition() {
  if(m_cameraPosDialog == NULL && GetVizmo().GetEnv() != NULL) {
    m_cameraPosDialog = new CameraPosDialog(m_mainWindow,
        m_mainWindow->GetGLScene()->GetCurrentCamera());
    m_mainWindow->ShowDialog(m_cameraPosDialog);
  }
}

void
GLWidgetOptions::
ChangeBGColor() {
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
GLWidgetOptions::
ShowGeneralContextMenu() {
  QMenu cm(this);
  cm.addAction(m_actions["changeBGColor"]);
  cm.addAction(m_actions["resetCamera"]);
  cm.addAction(m_actions["setCameraPosition"]);
  cm.addAction(m_actions["showAxis"]);
  cm.addAction(m_actions["showFrameRate"]);

  if(cm.exec(QCursor::pos()) != 0)
    m_mainWindow->GetGLScene()->updateGL();
}

void
GLWidgetOptions::
MakeSolid() {
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  for(MIT i = sel.begin(); i!= sel.end(); i++)
    (*i)->SetRenderMode(SOLID_MODE);
  m_mainWindow->GetGLScene()->updateGL();
}

void
GLWidgetOptions::
MakeWired() {
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  for(MIT i = sel.begin(); i!= sel.end(); i++)
    (*i)->SetRenderMode(WIRE_MODE);
  m_mainWindow->GetGLScene()->updateGL();
}

void
GLWidgetOptions::
MakeInvisible() {
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  for(MIT i = sel.begin(); i!= sel.end(); i++)
    (*i)->SetRenderMode(INVISIBLE_MODE);
  m_mainWindow->GetGLScene()->updateGL();
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

  vector<Model*>& selectedModels = GetVizmo().GetSelectedModels();
  for(MIT mit = selectedModels.begin(); mit != selectedModels.end(); ++mit) {
    Model* model = *mit;
    if(model->Name() == "Robot") {
      model->SetColor(Color4(r, g, b, 1));
      ((RobotModel*)model)->BackUp();
    }
    else
      model->SetColor(Color4(r, g, b, 1));
  }
}

void
GLWidgetOptions::
ShowObjectNormals() {
  vector<Model*>& selectedModels = GetVizmo().GetSelectedModels();
  for(MIT mit = selectedModels.begin(); mit != selectedModels.end(); ++mit) {
    Model* model = *mit;
    model->ToggleNormals();
  }
}
