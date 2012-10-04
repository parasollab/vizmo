#include <QAction>
#include <QToolBar>
#include <QPixmap>
#include <QTranslator>

#include "PathOptions.h"
#include "MainWin.h"
#include "SceneWin.h" 
#include "CustomizePathDialog.h"  
#include "QueryGUI.h" 
#include "EnvObj/PathLoader.h"
#include "EnvObj/QueryLoader.h"
#include "EnvObj/DebugModel.h"   
#include "vizmo2.h"  
#include "icon/Pen.xpm"  
#include "icon/Flag.xpm" 

PathOptions::PathOptions(QWidget* _parent, VizmoMainWin* _mainWin)
  :OptionsBase(_parent, _mainWin)
{
  CreateActions(); 
  SetUpCustomSubmenu(); 
  SetUpToolbar(); 
  SetHelpTips(); 
}

void
PathOptions::CreateActions(){

  //1. Create actions and add them to map
  QAction* showHidePath = new QAction(QPixmap(pen), tr("Show/Hide Path"), this); 
  m_actions["showHidePath"] = showHidePath; 
  QAction* showHideSG = new QAction(QPixmap(flag), tr("Show/Hide Start/Goal"), this);
  m_actions["showHideSG"] = showHideSG;  
  QAction* pathOptions = new QAction(tr("Path Display Options"), this);
  m_actions["pathOptions"] = pathOptions;
  m_pathOptionsInput = new CustomizePathDialog(this);          //pop-up window  
  //QAction* setQryStart = new QAction(tr("Start"), this);     TEMPORARY DISABLE
  //m_actions["setQryStart"] = setQryStart;                    ...segfaults 
  //QAction* setQryGoal = new QAction(tr("Goal"), this); (broken in orig. vizmo
  //too)
  //m_actions["setQryGoal"] = setQryGoal;   

  //2. Set other specifications as necessary
  m_actions["showHidePath"]->setEnabled(false);
  m_actions["showHidePath"]->setStatusTip(tr("Show or hide the path"));
  m_actions["showHideSG"]->setEnabled(false); 
  m_actions["showHideSG"]->setStatusTip(tr("Show or hide the start and goal")); 
  m_actions["pathOptions"]->setEnabled(false);
  //m_actions["setQryStart"]->setEnabled(false); 
  //m_actions["setQryGoal"]->setEnabled(false);  

  //3. Make connections 
  connect(m_actions["showHidePath"], SIGNAL(activated()), this, SLOT(ShowHidePath())); 
  connect(m_actions["showHideSG"], SIGNAL(activated()), this, SLOT(ShowHideStartGoal())); 
  connect(m_actions["pathOptions"], SIGNAL(activated()), this, SLOT(PathDisplayOptions())); 
  //connect(m_actions["setQryStart"], SIGNAL(activated()), this, SLOT(SetQueryStart()));
  //connect(m_actions["setQryGoal"], SIGNAL(activated()), this, SLOT(SetQueryGoal()));   
}

void
PathOptions::SetUpCustomSubmenu(){

  m_submenu = new QMenu("Path", this); 

  m_submenu->addAction(m_actions["showHidePath"]);
  m_submenu->addAction(m_actions["showHideSG"]);
  m_submenu->addAction(m_actions["pathOptions"]); 

  //m_setQuery = new QMenu("Set Query", this); 
  //m_setQuery->addAction(m_actions["setQryStart"]); 	
  //m_setQuery->addAction(m_actions["setQryGoal"]); 
  //m_submenu->addMenu(m_setQuery);

  //m_setQuery->setEnabled(false); 
}

void
PathOptions::SetUpToolbar(){

  m_toolbar = new QToolBar(GetMainWin()); 

  m_toolbar->addAction(m_actions["showHidePath"]); 
  m_toolbar->addAction(m_actions["showHideSG"]); 
}

void
PathOptions::Reset(){

  if(m_actions["showHidePath"] != NULL){
    if(GetVizmo().GetPathSize()==0)
      m_actions["showHidePath"]->setEnabled(false);
    else{
      m_actions["showHidePath"]->setEnabled(true);
      m_actions["pathOptions"]->setEnabled(true);
    }
  }

  if(m_actions["showHideSG"] != NULL){ 
    if(GetVizmo().IsQueryLoaded())    
      m_actions["showHideSG"]->setEnabled(true); 
    else
      m_actions["showHideSG"]->setEnabled(false); 
  }

  m_pathOptionsInput->RestoreDefault();
  //Statements below should NOT be moved into RestoreDefault()
  //RestoreDefault is for colors only
  m_pathOptionsInput->m_widthLineEdit->setText("1"); 
  m_pathOptionsInput->m_modLineEdit->setText("3");  //Display interval  

  //m_setQuery->setEnabled(true);   TEMPORARY DISABLE 
  //m_actions["setQryStart"]->setEnabled(true); 
  //m_actions["setQryGoal"]->setEnabled(true); 
}

void
PathOptions::SetHelpTips(){

  m_actions["showHidePath"]->setWhatsThis(tr("Click this button to visualize"
    " the <b>Path</b>.<br> You can also select the <b>Show/Hide Path</b> option"
    " from the <b>Path</b> menu."));

  m_actions["showHideSG"]->setWhatsThis(tr("Click this button to visualize the" 
    " <b>Start and Goal</b> configurations.<br>"
    "You can also select the <b>Show/Hide Start/Goal</b> option "
    "from the <b>Path</b> menu."));
}

//Slots

void
PathOptions::ShowHidePath(){

  static bool show=false;
  show=!show;
  GetVizmo().ShowPathFrame(show);
  GetMainWin()->GetGLScene()->updateGL();
}

void
PathOptions::ShowHideStartGoal(){

  static bool show=false;
  show=!show;
  GetVizmo().ShowQueryFrame(show);
  GetMainWin()->GetGLScene()->updateGL();
}

void
PathOptions::PathDisplayOptions(){

  //Pop up the path customization window
  m_pathOptionsInput->show(); 
}

/*void                           TEMPORARY DIASBLE
  PathOptions::SetQueryStart(){

  m_qrySGUI = new queryGUI(this);
  connect(m_qrySGUI, SIGNAL(callUpdate()), GetMainWin(), SLOT(updateScreen()));
  connect(GetMainWin()->GetGLScene(), SIGNAL(MRbyGLI()), m_qrySGUI, SLOT(updateQryCfg()));
  m_qrySGUI->m_SorG = 's';
///////////////////////////////////////////
//set original Query cfgs. to queryGUI
//so they can be reset...
////////////////////////////////////////
vector<double> cfg;

if(GetVizmo().IsQueryLoaded()){ 
PlumObject* m_qry;   
m_qry = GetVizmo().GetQry();
CQueryLoader* q = (CQueryLoader*)m_qry->getLoader();
cfg = q->GetStartGoal(0);
m_qrySGUI->setQuery(cfg);
} 
else if(GetVizmo().IsPathLoaded()){
PlumObject* m_path;
m_path = GetVizmo().GetPath();
CPathLoader* ploader=(CPathLoader*)m_path->getLoader();
cfg = ploader->GetConfiguration(0);
} 
else{
int dof;
PlumObject* m_Rob;
m_Rob = GetVizmo().GetRobot();
OBPRMView_Robot* r = (OBPRMView_Robot*)m_Rob->getModel();    
dof = r->returnDOF(); 
cfg = vector<double>(dof,0);
} 

m_qrySGUI->show();
}*/

/*void                             TEMPORARY DISABLE
  PathOptions::SetQueryGoal(){

  m_qryGGUI= new queryGUI(this);
  connect(m_qryGGUI, SIGNAL(callUpdate()), GetMainWin(), SLOT(updateScreen()));
  connect(GetMainWin()->GetGLScene(), SIGNAL(MRbyGLI()), m_qryGGUI, SLOT(updateQryCfg()));
  m_qrySGUI->m_SorG = 'g';
///////////////////////////////////////////
//set original Query cfgs. to queryGUI
//so they can be reset...
////////////////////////////////////////

vector<double> cfg;

if(GetVizmo().IsQueryLoaded()){
PlumObject* m_qry;
m_qry = GetVizmo().GetQry();
CQueryLoader * q=(CQueryLoader*)m_qry->getLoader();
cfg=q->GetStartGoal(1);
m_qrySGUI->setQuery(cfg);
} 

else if(GetVizmo().IsPathLoaded()){
PlumObject* m_path;
m_path = GetVizmo().GetPath();
CPathLoader* ploader=(CPathLoader*)m_path->getLoader();
int path_size = ploader->GetPathSize();
cfg = ploader->GetConfiguration(path_size-1);
} 

else{
int dof;
PlumObject* m_Rob;
m_Rob = GetVizmo().GetRobot();
OBPRMView_Robot* r = (OBPRMView_Robot*)m_Rob->getModel();    
dof = r->returnDOF(); 
cfg = vector<double>(dof,0);
} 

m_qryGGUI->show();
}
*/


