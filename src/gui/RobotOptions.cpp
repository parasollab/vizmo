#include <QAction>

#include "RobotOptions.h"
#include "MainWin.h"
#include "vizmo2.h" 
#include "AnimationGUI.h" 
#include "SceneWin.h"

RobotOptions::RobotOptions(QWidget* _parent, VizmoMainWin* _mainWin)
  :OptionsBase(_parent, _mainWin)
{
  CreateActions();
  SetUpSubmenu("Robot"); 
  SetUpToolbar(); 
  SetHelpTips(); 
}

void
RobotOptions::CreateActions(){

  //1. Create actions and add them to the map 
  QAction* resetRobotPos = new QAction(tr("Reset position"), this); 
  m_actions["resetRobotPos"] = resetRobotPos;
  QAction* showRobotCfg = new QAction(tr("Show configuration"), this);
  m_actions["showRobotCfg"] = showRobotCfg; 

  //2. Set other specifications as necessary 
  m_actions["resetRobotPos"]->setEnabled(false); 
  m_actions["showRobotCfg"]->setEnabled(false); 
  m_actions["showRobotCfg"]->setCheckable(true);
  m_actions["showRobotCfg"]->setChecked(false); //may not need... 

  //3. Make connections 
  connect(m_actions["resetRobotPos"], SIGNAL(activated()), this, SLOT(ResetRobotPosition())); 
  // connect(m_actions["showRobotCfg"], SIGNAL(activated()), this, SLOT(ShowRobotCfg())); 
}

void
RobotOptions::SetUpToolbar(){} //Nothing in this case... 

void
RobotOptions::Reset(){

  m_actions["resetRobotPos"]->setEnabled(true); 
  m_actions["showRobotCfg"]->setEnabled(false); 
  m_actions["showRobotCfg"]->setChecked(false); 
}

void
RobotOptions::SetHelpTips(){

}

//Slots 

void
RobotOptions::ResetRobotPosition(){

  GetVizmo().ResetRobot();
  GetMainWin()->GetAnimationGUI()->reset();
  GetMainWin()->GetAnimationDebugGUI()->reset();
  GetMainWin()->GetGLScene()->updateGL();
}

/*void RobotOptions::ShowRobotCfg(){

  if(m_actions["showRobotCfg"]->isChecked()){
  GetMainWin()->GetRoadmap()->robCfgOn = true;
  if(GetMainWin()->GetRoadmap()->l_cfg->isHidden())
  GetMainWin()-> GetRoadmap()->l_cfg->show();
  if(GetMainWin()->GetRoadmap()->l_robCfg->isHidden())
  GetMainWin()->GetRoadmap()->l_robCfg->show();
  }
  else{
  GetMainWin()->GetRoadmap()->robCfgOn = false;
  GetMainWin()->GetRoadmap()->l_cfg->hide();
  GetMainWin()->GetRoadmap()->l_robCfg->hide();
  }
  }
  */           







