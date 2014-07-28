#include "RobotOptions.h"

#include <QtGui>

#include "MainWindow.h"
#include "AnimationWidget.h"
#include "GLWidget.h"
#include "Models/Vizmo.h"

#include "Icons/ResetRobotPos.xpm"
#include "Icons/ShowRobotCfg.xpm"

RobotOptions::RobotOptions(QWidget* _parent, MainWindow* _mainWindow)
  : OptionsBase(_parent, _mainWindow) {
    CreateActions();
    SetUpSubmenu("Robot");
    SetUpToolbar();
    SetHelpTips();
  }

void
RobotOptions::CreateActions(){

  //1. Create actions and add them to the map
  QAction* resetRobotPos = new QAction(QPixmap(resetrobotpos), tr("Reset position"), this);
  m_actions["resetRobotPos"] = resetRobotPos;
  QAction* showRobotCfg = new QAction(QPixmap(showrobotcfg), tr("Show configuration"), this);
  m_actions["showRobotCfg"] = showRobotCfg;

  //2. Set other specifications as necessary
  m_actions["resetRobotPos"]->setEnabled(false);
  m_actions["showRobotCfg"]->setEnabled(false);
  m_actions["showRobotCfg"]->setCheckable(true);
  m_actions["showRobotCfg"]->setChecked(false); //may not need...

  //3. Make connections
  connect(m_actions["resetRobotPos"], SIGNAL(triggered()), this, SLOT(ResetRobotPosition()));
  // connect(m_actions["showRobotCfg"], SIGNAL(triggered()), this, SLOT(ShowRobotCfg()));
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
  m_mainWindow->GetAnimationWidget()->Reset();
  m_mainWindow->GetGLWidget()->updateGL();
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







