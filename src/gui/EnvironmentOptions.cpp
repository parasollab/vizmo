#include <QAction>
#include <QToolBar>
#include <QPixmap>
#include <QTranslator>

#include "EnvironmentOptions.h"
#include "MainWin.h"
#include "SceneWin.h"
#include "ItemSelectionGUI.h" 
#include "AddObjDialog.h" 
#include "vizmo2.h" 
#include "icon/RandEnv.xpm"

  EnvironmentOptions::EnvironmentOptions(QWidget* _parent, VizmoMainWin* _mainWin)
:OptionsBase(_parent, _mainWin)
{
  CreateActions();
  SetUpSubmenu("Environment"); 
  SetUpToolbar();
  SetHelpTips(); 
}

void
EnvironmentOptions::CreateActions(){  

  //1. Create actions and add them to the map  
  QAction* refreshEnv = new QAction(tr("Refresh"), this); 
  m_actions["refreshEnv"] = refreshEnv;
  QAction* randEnvColors = new QAction(QPixmap(randEnvIcon), tr("Randomize Environment Colors"), this); 
  m_actions["randEnvColors"] = randEnvColors;  
  QAction* showBBox = new QAction(tr("&Bounding Box"), this); 
  m_actions["showBBox"] = showBBox; //may need to check on that shortcut  
  QAction* addObstacle = new QAction(tr("Add Obstacle"), this); 
  m_actions["addObstacle"] = addObstacle; 

  //2. Set other specifications as necessary
  m_actions["refreshEnv"]->setEnabled(false); 
  m_actions["randEnvColors"]->setEnabled(false); 	
  m_actions["randEnvColors"]->setStatusTip(tr("Randomize environment colors")); 	
  m_actions["showBBox"]->setEnabled(false); 
  m_actions["addObstacle"]->setEnabled(false); 

  //3. Make connections 
  connect(m_actions["refreshEnv"], SIGNAL(activated()), this, SLOT(RefreshEnv())); 
  connect(m_actions["randEnvColors"], SIGNAL(activated()), this, SLOT(RandomizeEnvColors())); 
  connect(m_actions["showBBox"], SIGNAL(activated()), this, SLOT(ShowBBox())); 
  connect(m_actions["addObstacle"], SIGNAL(activated()), this, SLOT(AddObstacle())); 
} 

void
EnvironmentOptions::SetUpToolbar(){
  
  m_toolbar = new QToolBar(GetMainWin()); 
  m_toolbar->addAction(m_actions["randEnvColors"]); 
}

void
EnvironmentOptions::Reset(){

  m_actions["refreshEnv"]->setEnabled(true); 	
  m_actions["randEnvColors"]->setEnabled(true);
  m_actions["showBBox"]->setEnabled(true);  
  m_actions["addObstacle"]->setEnabled(true); 	
}

void
EnvironmentOptions::SetHelpTips(){

  m_actions["randEnvColors"]->setWhatsThis(tr("Click this button to"
    " randomize the colors of the environment objects.")); 
}


//Slots   

void 
EnvironmentOptions::RefreshEnv(){

  GetVizmo().RefreshEnv();
  GetMainWin()->GetObjectSelection()->reset();
  GetMainWin()->GetGLScene()->updateGL();
}

void
EnvironmentOptions::RandomizeEnvColors(){

  GetVizmo().envObjsRandomColor(); 
  GetMainWin()->GetGLScene()->updateGL(); 
}

void
EnvironmentOptions::ShowBBox(){

  static bool show = true;  // The box is shown when the file is loaded
  show =!show;
  GetVizmo().ShowBBox(show);
  GetMainWin()->GetGLScene()->updateGL();
}

void
EnvironmentOptions::AddObstacle(){

  AddObjDial* addObj = new AddObjDial(this, "Add Object");
  bool create = addObj->create();
  if(create){  
    int r;
    r =   addObj->exec();
    //if(!r) return;
    GetMainWin()->GetObjectSelection()->reset();
  }

  else
    return;	
}



























