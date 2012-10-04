#include <QAction>
#include <QToolButton>
#include <QPixmap>
#include <QTranslator>
#include <QStatusBar>
#include <QColorDialog> 
#include <QCursor>  

#include <iostream>
#include <sstream>  
#include <Point.h>
#include <GL/gliCamera.h>  
#include "SceneOptions.h"
#include "MainWin.h"
#include "SceneWin.h" 
#include "CameraPosDialog.h" 
#include "icon/ResetCamera.xpm"  
#include "icon/BgColor.xpm" 

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
  QAction* showGrid = new QAction(tr("Grid"), this); 
  m_actions["showGrid"] = showGrid; 
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
  m_actions["showGrid"]->setCheckable(true); 	
  m_actions["showGrid"]->setChecked(false);
  m_actions["showAxis"]->setCheckable(true);
  m_actions["showAxis"]->setChecked(true);
  m_actions["resetCamera"]->setEnabled(false);
  m_actions["resetCamera"]->setStatusTip(tr("Reset the camera position")); 
  m_actions["changeBGColor"]->setEnabled(false); 
  m_actions["changeBGColor"]->setStatusTip(tr("Change the color of the background")); 

  //3. Make connections 
  connect(m_actions["showGrid"], SIGNAL(activated()), this, SLOT(ShowGrid()));
  connect(m_actions["showAxis"], SIGNAL(activated()), this, SLOT(ShowAxis())); 
  connect(m_actions["resetCamera"], SIGNAL(activated()), this, SLOT(ResetCamera())); 
  connect(m_actions["setCameraPosition"], SIGNAL(activated()), this, SLOT(SetCameraPosition())); 
  connect(m_actions["changeBGColor"], SIGNAL(activated()), this, SLOT(ChangeBGColor())); 
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
SceneOptions::ShowGrid(){

  GetMainWin()->GetGLScene()->showGrid(); 
}

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

  Point3d p = gliGetCameraFactory().getCurrentCamera()->getCameraPos();
  //Unfortunately, points are defined backwards for x and y
  //We want "X=3" to mean that VIEWER has moved to X=3 
  if(p[0] != 0) //otherwise displays '-0' ! 
    p[0] = -1*p[0];
  if(p[1] != 0)
    p[1] = -1*p[1]; 

  double azim = gliGetCameraFactory().getCurrentCamera()->getCurrentAzim();  
  double elev = gliGetCameraFactory().getCurrentCamera()->getCurrentElev();

  ostringstream ossX, ossY, ossZ, ossAzim, ossElev;  
  ossX << p[0]; 
  ossY << p[1]; 
  ossZ << p[2]; 
  ossAzim << azim; 
  ossElev << elev; 

  QString QsX((ossX.str()).c_str()); 
  QString QsY((ossY.str()).c_str()); 
  QString QsZ((ossZ.str()).c_str()); 
  QString QsAzim((ossAzim.str()).c_str());
  QString QsElev((ossElev.str()).c_str()); 

  (m_cameraPosInput->m_xLineEdit)->setText(QsX); 
  (m_cameraPosInput->m_yLineEdit)->setText(QsY); 
  (m_cameraPosInput->m_zLineEdit)->setText(QsZ); 
  (m_cameraPosInput->m_azimLineEdit)->setText(QsAzim); 
  (m_cameraPosInput->m_elevLineEdit)->setText(QsElev); 

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
  cm.addAction(m_actions["changeBGcolor"]);
  cm.addAction(m_actions["resetCamera"]); 
  cm.addAction(m_actions["setCameraPosition"]);
  cm.addAction(m_actions["showGrid"]); 
  cm.addAction(m_actions["showAxis"]); 

  if(cm.exec(QCursor::pos()) != 0) 
    GetMainWin()->GetGLScene()->updateGL();   
} 















