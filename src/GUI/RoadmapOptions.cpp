/***********************************************
 * Implementation for RoadmapOptions. 
 * This class has submenus WITHIN m_submenu, so 
 * SetUpCustomSubmenu() is used instead of the 
 * SetUpSubmenu() provided in the base class.
 * Actions in these smaller submenus are 
 * grouped together in the code. 
 ***********************************************/ 

#include <QAction>
#include <QPushButton> 
#include <QButtonGroup>  
#include <QMenu>  
#include <QToolBar>
#include <QPixmap>
#include <QTranslator>
#include <QStatusBar>   
#include <QColorDialog> 
#include <QInputDialog>
#include <QMessageBox> 

#include "RoadmapOptions.h"
#include "SizeSliderDialog.h" 
#include "OptionsBase.h" 
#include "SceneWin.h"
#include "MainWin.h"
#include "vizmo2.h"
#include "Icons/Pallet.xpm"
#include "Icons/MakeSolid.xpm"
#include "Icons/MakeWired.xpm"
#include "Icons/MakeInvisible.xpm"
#include "Icons/CCColor.xpm"
#include "Icons/RColor.xpm"
#include "Icons/CCsOneColor.xpm"
#include "Icons/NodeSize.xpm"
#include "Icons/EdgeThickness.xpm"
#include "Icons/Navigate.xpm" 

RoadmapOptions::RoadmapOptions(QWidget* _parent, VizmoMainWin* _mainWin)
  :OptionsBase(_parent, _mainWin)
{
  CreateActions(); 
  SetUpCustomSubmenu(); 
  SetUpToolbar();
  SetHelpTips(); 

  m_robotButton->click();
  m_edgeThickness = 1; 
  m_nodeSize = 1;
  
}

void RoadmapOptions::CreateActions(){

  //1. Create actions/additional submenus and add them to the map
  QAction* showHideRoadmap = new QAction(QPixmap(navigate), tr("Show Roadmap"), this); 
  m_actions["showHideRoadmap"] = showHideRoadmap; 

  m_nodeView = new QButtonGroup(this); 
  
  QAction* robotView = new QAction(tr("Robot"), this);
  m_actions["robotView"] = robotView;
  //Actually make a push button for Robot (looks better on toolbar) 
  m_robotButton = new QPushButton(tr("Robot"), this); 
  m_robotButton->setFixedWidth(48); 
  m_robotButton->setEnabled(false); 
  m_robotButton->setCheckable(true); 
  m_robotButton->setStatusTip("Display nodes in robot mode"); 
  m_nodeView->addButton(m_robotButton);  
  
  QAction* boxView = new QAction(tr("Box"), this); 
  m_actions["boxView"] = boxView;
  //Push button for Box
  m_boxButton = new QPushButton(tr("Box"), this);
  m_boxButton->setFixedWidth(48); 
  m_boxButton->setEnabled(false); 
  m_boxButton->setCheckable(true); 
  m_boxButton->setStatusTip("Display nodes in box mode"); 
  m_nodeView->addButton(m_boxButton); 
  
  QAction* pointView = new QAction(tr("Point"), this); 
  m_actions["pointView"] = pointView; 
  //Push button for Point
  m_pointButton = new QPushButton(tr("Point"), this); 
  m_pointButton->setFixedWidth(48); 
  m_pointButton->setEnabled(false); 
  m_pointButton->setCheckable(true); 
  m_pointButton->setStatusTip("Display nodes in point mode"); 
  m_nodeView->addButton(m_pointButton); 

  QAction* makeSolid = new QAction(QPixmap(makeSolidIcon), tr("Make Solid"), this);
  m_actions["makeSolid"] = makeSolid; 
  QAction* makeWired = new QAction(QPixmap(makeWiredIcon), tr("Make Wired"), this); 
  m_actions["makeWired"] = makeWired; 
  QAction* makeInvisible = new QAction(QPixmap(makeInvisibleIcon), tr("Make Invisible"), this); 
  m_actions["makeInvisible"] = makeInvisible; 
  QAction* changeNodeColor = new QAction(QPixmap(pallet), tr("Change Color"), this); 
  m_actions["changeNodeColor"] = changeNodeColor; 

  QAction* scaleNodes = new QAction(QPixmap(nodeSizeIcon), tr("Scale Nodes"), this);
  m_actions["scaleNodes"] = scaleNodes;
  //SizeSliderDialog needs a plain QWidget as a parent, but to actually access
  //this parent as RoadmapOptions within the dialog class, a RoadmapOptions
  //pointer is stored there (hence, we have SizeSliderDialog(QWidget* _parent,
  //RoadmapOptions* _accessParent) in the constructor). Alternatively, the
  //constructor can just take the first argument, and the parent can be casted
  //when needed in the dialog class. 
  m_nodeSizeDialog = new SizeSliderDialog("node", this, this); //String argument is the dialog mode 

  QAction* edgeThickness = new QAction(QPixmap(edgeThicknessIcon), tr("Change Edge Thickness"), this); 
  m_actions["edgeThickness"] = edgeThickness;
  m_edgeThicknessDialog = new SizeSliderDialog("edge", this, this); 

  QAction* colorSelected = new QAction(QPixmap(ccColor), tr("Change Color of Selected"), this);  
  m_actions["colorSelected"] = colorSelected; 
  QAction* randomizeColors = new QAction(QPixmap(rcolor), tr("Randomize Colors"), this); 
  m_actions["randomizeColors"] = randomizeColors;
  QAction* ccsOneColor = new QAction(QPixmap(ccsOneColorIcon), tr("Make All One Color"), this); 
  m_actions["ccsOneColor"] = ccsOneColor;
  QAction* saveStart = new QAction(tr("Save Query Start"), this); 
  m_actions["saveStart"] = saveStart; 
  QAction* saveGoal = new QAction(tr("Save Query Goal"), this); 
  m_actions["saveGoal"] = saveGoal; 
  QAction* changeObjectColor = new QAction(tr("Change Color"), this); 
  m_actions["changeObjectColor"] = changeObjectColor; 

  //2. Set other specifications as necessary
  m_actions["showHideRoadmap"]->setCheckable(true); 
  m_actions["showHideRoadmap"]->setEnabled(false);
  m_actions["showHideRoadmap"]->setStatusTip("Display or hide the roadmap");
  m_actions["showHideRoadmap"]->setToolTip("Show/Hide Roadmap");  
  //m_actions["robotView"]->setCheckable(true);  
  m_actions["robotView"]->setEnabled(false);
  m_actions["robotView"]->setStatusTip(tr("Display nodes in robot mode"));
  //m_actions["boxView"]->setCheckable(true);  
  m_actions["boxView"]->setEnabled(false);
  m_actions["boxView"]->setStatusTip(tr("Display nodes in box mode"));
  //m_actions["pointView"]->setCheckable(true);  
  m_actions["pointView"]->setEnabled(false);
  m_actions["pointView"]->setStatusTip(tr("Display nodes in point mode"));

  m_actions["makeSolid"]->setShortcut(tr("CTRL+N")); 
  m_actions["makeSolid"]->setEnabled(false);  
  m_actions["makeSolid"]->setStatusTip(tr("Change to solid display mode"));  
  m_actions["makeWired"]->setShortcut(tr("CTRL+N")); 
  m_actions["makeWired"]->setEnabled(false);  
  m_actions["makeWired"]->setStatusTip(tr("Change to wired display mode"));  
  m_actions["makeInvisible"]->setShortcut(tr("CTRL+N")); 
  m_actions["makeInvisible"]->setEnabled(false);  
  m_actions["makeInvisible"]->setStatusTip(tr("Change to invisible"));  
  m_actions["changeNodeColor"]->setShortcut(tr("CTRL+N")); 
  m_actions["changeNodeColor"]->setEnabled(false);  
  m_actions["changeNodeColor"]->setStatusTip(tr("Change node color"));  

  m_actions["scaleNodes"]->setShortcut(tr("CTRL+S")); 
  m_actions["scaleNodes"]->setEnabled(false); 
  m_actions["scaleNodes"]->setStatusTip(tr("Resize the nodes")); 

  m_actions["edgeThickness"]->setEnabled(false);
  m_actions["edgeThickness"]->setStatusTip(tr("Change edge thickness"));

  m_actions["colorSelected"]->setShortcut(tr("CTRL+C")); 
  m_actions["colorSelected"]->setEnabled(false); 	
  m_actions["colorSelected"]->setStatusTip(tr("Change color of selected CC")); 	
  m_actions["randomizeColors"]->setShortcut(tr("CTRL+R")); 
  m_actions["randomizeColors"]->setEnabled(false); 
  m_actions["randomizeColors"]->setStatusTip(tr("Randomize CC colors")); 
  m_actions["ccsOneColor"]->setEnabled(false);  
  m_actions["ccsOneColor"]->setStatusTip(tr("Make all CCs one color"));  

  //3. Make connections
  connect(m_actions["showHideRoadmap"], SIGNAL(activated()), this, SLOT(ShowRoadmap())); 

  connect(m_nodeView, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(ChangeNodeShape())); 
  connect(m_actions["robotView"], SIGNAL(activated()), this, SLOT(ClickRobot())); //would be better as single function  
  connect(m_actions["boxView"], SIGNAL(activated()), this, SLOT(ClickBox()));  
  connect(m_actions["pointView"], SIGNAL(activated()), this, SLOT(ClickPoint()));   

  connect(m_actions["makeSolid"], SIGNAL(activated()), this, SLOT(MakeSolid())); 
  connect(m_actions["makeWired"], SIGNAL(activated()), this, SLOT(MakeWired())); 
  connect(m_actions["makeInvisible"], SIGNAL(activated()), this, SLOT(MakeInvisible()));
  connect(m_actions["changeNodeColor"], SIGNAL(activated()), this, SLOT(ChangeObjectColor()));  

  connect(m_actions["scaleNodes"], SIGNAL(activated()), this, SLOT(ScaleNodes()));

  connect(m_actions["edgeThickness"], SIGNAL(activated()), this, SLOT(ChangeEdgeThickness()));  

  connect(m_actions["colorSelected"], SIGNAL(activated()), this, SLOT(ColorSelectedCC())); 
  connect(m_actions["randomizeColors"], SIGNAL(activated()), this, SLOT(RandomizeCCColors())); 
  connect(m_actions["ccsOneColor"], SIGNAL(activated()), this, SLOT(MakeCCsOneColor())); 
  connect(m_actions["saveStart"], SIGNAL(activated()), this, SLOT(SaveQueryStart())); 
  connect(m_actions["saveGoal"], SIGNAL(activated()), this, SLOT(SaveQueryGoal())); 
  connect(m_actions["changeObjectColor"], SIGNAL(activated()), this, SLOT(ChangeObjectColor())); 
  connect(GetMainWin()->GetGLScene(), SIGNAL(selectByRMB()), this, SLOT(ShowObjectContextMenu())); 

}

void
RoadmapOptions::SetUpCustomSubmenu(){

  m_submenu = new QMenu("Roadmap", this); 	

  m_submenu->addAction(m_actions["showHideRoadmap"]); 

  m_nodeShape = new QMenu("Change Node Shape", this); 
  m_nodeShape->addAction(m_actions["robotView"]);
  m_nodeShape->addAction(m_actions["boxView"]); 
  m_nodeShape->addAction(m_actions["pointView"]);
  m_submenu->addMenu(m_nodeShape); 

  m_modifySelected = new QMenu("Modify Selected Item", this); 
  m_modifySelected->addAction(m_actions["makeSolid"]);  
  m_modifySelected->addAction(m_actions["makeWired"]);  
  m_modifySelected->addAction(m_actions["makeInvisible"]);  
  m_modifySelected->addAction(m_actions["changeNodeColor"]);  
  m_submenu->addMenu(m_modifySelected);

  m_submenu->addAction(m_actions["scaleNodes"]); 

  m_submenu->addAction(m_actions["edgeThickness"]);  

  m_modifyCCs = new QMenu("Modify CCs", this); 
  m_modifyCCs->addAction(m_actions["colorSelected"]); 
  m_modifyCCs->addAction(m_actions["randomizeColors"]); 
  m_modifyCCs->addAction(m_actions["ccsOneColor"]); 
  m_submenu->addMenu(m_modifyCCs); 

  //Disable the extra submenus by default
  m_nodeShape->setEnabled(false); 
  m_modifySelected->setEnabled(false); 
  m_modifyCCs->setEnabled(false); 

}

void
RoadmapOptions::SetUpToolbar(){

  m_toolbar = new QToolBar(GetMainWin()); 

  m_toolbar->addAction(m_actions["showHideRoadmap"]); 
  m_toolbar->addWidget(m_robotButton);
  m_toolbar->addWidget(m_boxButton); 
  m_toolbar->addWidget(m_pointButton);
  m_toolbar->addSeparator();  
  m_toolbar->addAction(m_actions["makeSolid"]); 
  m_toolbar->addAction(m_actions["makeWired"]); 
  m_toolbar->addAction(m_actions["makeInvisible"]); 
  m_toolbar->addAction(m_actions["changeNodeColor"]); 
  m_toolbar->addAction(m_actions["scaleNodes"]); 
  m_toolbar->addAction(m_actions["edgeThickness"]); 
  m_toolbar->addAction(m_actions["colorSelected"]); 
  m_toolbar->addAction(m_actions["randomizeColors"]); 
  m_toolbar->addAction(m_actions["ccsOneColor"]); 
}

void
RoadmapOptions::Reset(){

  if(m_actions["showHideRoadmap"] != NULL){
    if(!GetVizmo().IsRoadMapLoaded()){
      m_actions["showHideRoadmap"]->setEnabled(false);
    }
    else{
      m_actions["showHideRoadmap"]->setEnabled(true);
    }
  }

  //Enable m_nodeShape AND its items 
  m_nodeShape->setEnabled(true);  
  m_actions["robotView"]->setEnabled(true);
  m_actions["boxView"]->setEnabled(true);
  m_actions["pointView"]->setEnabled(true);
  //Buttons must be enabled also 
  m_robotButton->setEnabled(true); 
  m_boxButton->setEnabled(true); 
  m_pointButton->setEnabled(true); 
  
  ClickPoint(); //Make sure a button is actually selected by default! 

  ChangeNodeShape(); 	

  m_modifySelected->setEnabled(true); 
  m_actions["makeSolid"]->setEnabled(true); 
  m_actions["makeWired"]->setEnabled(true); 
  m_actions["makeInvisible"]->setEnabled(true); 
  m_actions["changeNodeColor"]->setEnabled(true); 

  m_actions["scaleNodes"]->setEnabled(true);

  m_actions["edgeThickness"]->setEnabled(true);

  m_modifyCCs->setEnabled(true); 
  m_actions["colorSelected"]->setEnabled(true);
  m_actions["randomizeColors"]->setEnabled(true);
  m_actions["ccsOneColor"]->setEnabled(true);

  m_nodeSizeDialog->Reset();
  m_edgeThicknessDialog->Reset(); 
  
  if(GetMapModel() != NULL){
    GetMapModel()->GetNodeList().clear(); 
    GetMapModel()->SetSize(0.5);  
    GetMapModel()->GetNodesToConnect().clear(); 
    // GetMapModel()->GetCfgLabel()->clear(); temporarily removed/what is it for? 
    //GetMap()->l_cfg->clear();

    if(GetMapModel()->m_robCfgOn==false)
      //**MORE TEMPORARY(?) REMOVALS
      //  GetMapModel()->GetRobCfgLabel()->clear;  
      //GetMap()->l_robCfg->clear();

      //editAction->setChecked(false);
      //addNodeAction->setChecked(false);
      //addEdgeAction->setChecked(false);
      // GetMapModel()->GetMessageLabel()->clear(); 
      //GetMap()->GetLMessage()->clear();
      //GetMapModel()->GetIconLabel()->clear(); 
      //GetMap()->GetLIcon()->clear();
      GetMapModel()->SetMBEditModel(false);
    GetMapModel()->m_addNode=false;
    GetMapModel()->m_addEdge=false;
  }
}

void
RoadmapOptions::SetHelpTips(){

  m_actions["showHideRoadmap"]->setWhatsThis(tr("Click this button"
    " to visualize the <b>Roadmap</b>. You can also select the"
    " <b>Show/Hide Roadmap</b> option from the <b>Roadmap</b> menu."));
  m_robotButton->setWhatsThis(tr("Click this button to visualize"
    " the nodes in <b>Robot</b> mode."));
  m_boxButton->setWhatsThis(tr("Click this button to visualize"
    " the nodes in <b>Box</b> mode."));
  m_pointButton->setWhatsThis(tr("Click this button to visualize"
    " the nodes in <b>Point</b> mode."));
  m_actions["makeSolid"]->setWhatsThis(tr("Click this button to display a"
    " selected item in <b>Solid</b> mode."));
  m_actions["makeWired"]->setWhatsThis(tr("Click this button to display a"
    " selected item in <b>Wire</b> mode."));
  m_actions["makeInvisible"]->setWhatsThis(tr("Click this button to make a"
    " selected item invisible."));
  m_actions["changeNodeColor"]->setWhatsThis(tr("Click this button to change"
    " the color of a selected item."));
  m_actions["scaleNodes"]->setWhatsThis(tr("Click this button to resize the"
   " roadmap nodes by a specified factor."));
  m_actions["edgeThickness"]->setWhatsThis(tr("Click this button to scale the"
   " thickness of the edges."));
  m_actions["colorSelected"]->setWhatsThis(tr("Click this button to change the"
    " color of an entire connected component. Be sure to first select a CC from the" 
    " <b>Environment Objects</b> list.")); 
  m_actions["randomizeColors"]->setWhatsThis(tr("Click this button to randomize"
   " the colors of the connected components."));
  m_actions["ccsOneColor"]->setWhatsThis(tr("Click this button to set all of the"
   " connected components to a single color. ")); 
}

//Slots

void
RoadmapOptions::ShowRoadmap(){

  static bool show=false;
  show =!show;
  GetVizmo().ShowRoadMap(show);
 // Reset();
  GetVizmo().UpdateSelection();
  GetMainWin()->GetGLScene()->updateGL();
}

void
RoadmapOptions::ChangeNodeShape(){

  if(GetVizmo().GetMap() != NULL || GetVizmo().GetDebug() != NULL){
    if(m_nodeView->checkedButton() != 0){   
      string s = (string)(m_nodeView->checkedButton())->text().toAscii();  //prev checked action 
      GetVizmo().ChangeNodesShape(s); 
      GetMainWin()->GetGLScene()->updateGL();      
    }	
  }
}  

//Click functions are called when user selects drop-down menu option for Robot,
//Box, Point 
void
RoadmapOptions::ClickRobot(){

  m_robotButton->click();
} 

void
RoadmapOptions::ClickBox(){

  m_boxButton->click();

}

void
RoadmapOptions::ClickPoint(){

  m_pointButton->click(); 
}  

void
RoadmapOptions::MakeSolid(){

  string s;
  int k = 0; 
  vector<gliObj>& sel = GetVizmo().GetSelectedItem();
  typedef vector<gliObj>::iterator SI;
  for(SI i = sel.begin(); i!= sel.end(); i++){
    CGLModel* gl = (CGLModel*)(*i);
    gl->SetRenderMode(SOLID_MODE);
    s = gl->GetName();
    k++;
  }

  GetVizmo().UpdateSelection();
  GetMainWin()->GetGLScene()->updateGL(); 
}

void
RoadmapOptions::MakeWired(){

  string s;
  vector<gliObj>& sel = GetVizmo().GetSelectedItem();
  typedef vector<gliObj>::iterator SI;
  for(SI i = sel.begin(); i!= sel.end(); i++){
    CGLModel *gl = (CGLModel*)(*i);
    gl->SetRenderMode(WIRE_MODE);
  }
  GetVizmo().UpdateSelection();
  GetMainWin()->GetGLScene()->updateGL(); 
}

void 
RoadmapOptions::MakeInvisible(){

  string s;
  vector<gliObj>& sel = GetVizmo().GetSelectedItem();
  typedef vector<gliObj>::iterator SI;
  for(SI i = sel.begin(); i!= sel.end(); i++){
    CGLModel* gl = (CGLModel*)(*i);
    gl->SetRenderMode(INVISIBLE_MODE);
  }
  GetVizmo().UpdateSelection();
  GetMainWin()->GetGLScene()->updateGL(); 
}

void RoadmapOptions::ScaleNodes(){

 //For now, resizing only enabled for point and box abstractions. For robot,
 //would require extensive local coordinate system aspects  
 if(GetNodeShape() != "Robot")  
  m_nodeSizeDialog->show();
 else
  //Not a true "about" box, but does exactly what is needed. 
  QMessageBox::about(this, "Sorry!", "You can only resize the nodes in <b>Point</b> or <b>Box</b> mode.");  
}

void
RoadmapOptions::ChangeEdgeThickness(){

  m_edgeThicknessDialog->show(); 
}

void 
RoadmapOptions::ColorSelectedCC(){

  double R, G, B;
  R=G=B=1;

  //Check first if there is a CC selected
  vector<gliObj>& sel = GetVizmo().GetSelectedItem();
  typedef vector<gliObj>::iterator SI;
  string m_sO;
  for(SI i = sel.begin(); i!= sel.end(); i++){
    CGLModel* gl = (CGLModel*)(*i);
    m_sO = gl->GetName();
  }

  string m_s = "NULL";
  size_t position = 0;

  position = m_sO.find("CC",0);

  if(position != string::npos){ //Label "CC" has been found   
    QColor color = QColorDialog::getColor(Qt::white, this, "color dialog");
    if (color.isValid()){
      R = (double)(color.red()) / 255.0;
      G = (double)(color.green()) / 255.0;
      B = (double)(color.blue()) / 255.0;
    }

  string s;
  if(m_nodeView->checkedButton() != 0)
    s = (string)(m_nodeView->checkedButton())->text().toAscii();
  GetVizmo().ChangeCCColor(R, G, B, s);
  GetMainWin()->GetGLScene()->updateGL(); 
  } 
  else
    QMessageBox::about(this, "", "Please select a connected component from the <b>Environment Objects</b> menu.");   
}

void 
RoadmapOptions::RandomizeCCColors(){

  GetVizmo().ChangeNodesRandomColor(); 
  GetMainWin()->GetGLScene()->updateGL(); 
}

void 
RoadmapOptions::MakeCCsOneColor(){

  double R, G, B;
  R = G = B = 1;
  string s = "all";
  GetVizmo().oneColor = true;
  QColor color = QColorDialog::getColor(Qt::white, this, "color dialog");

  if(color.isValid()){
    R = (double)(color.red()) / 255.0;
    G = (double)(color.green()) / 255.0;
    B = (double)(color.blue()) / 255.0;
  }
  //also need to check that an action is indeed selected?  
  string shape = (string)(m_nodeView->checkedButton())->text().toAscii();
  GetVizmo().ChangeCCColor(R, G, B, shape);
}   

void
RoadmapOptions::ShowObjectContextMenu(){

  QMenu cm(this);
  cm.addAction(m_actions["makeSolid"]);  
  cm.addAction(m_actions["makeWired"]);  

  //Create submenu to set start and goal configs.
  //create it just if Robot has been selected
  string str;
  typedef vector<gliObj>::iterator GIT;

  for(GIT ig = GetVizmo().GetSelectedItem().begin(); 
      ig!=GetVizmo().GetSelectedItem().end(); ig++){
    CGLModel* gl=(CGLModel*)(*ig);
    vector<string> info=gl->GetInfo();
    str = info.front();
  }

  if(str == "Robot"){
    QMenu* cfgs = new QMenu("Set Query", this);  //Special robot menu 
    cfgs->setTearOffEnabled(true); 
    cfgs->addAction(m_actions["saveStart"]); 
    cfgs->addAction(m_actions["saveGoal"]);
    cm.addMenu(cfgs);  
  }
  else
    cm.addAction(m_actions["makeInvisible"]); 
    cm.addAction(m_actions["changeObjectColor"]);   
    cm.addSeparator();
    /*LEAVE HERE*/  //cm.insertItem("Edit...", this,SLOT(objectEdit()));

  if(cm.exec(QCursor::pos())!= 0) //was -1 for q3 version (index based) 
    GetMainWin()->GetGLScene()->updateGL();    
} 

void
RoadmapOptions::SaveQueryStart(){

  GetVizmo().SaveQryCfg('s'); 
}

void
RoadmapOptions::SaveQueryGoal(){

  GetVizmo().SaveQryCfg('g'); 
}

void
RoadmapOptions::ChangeObjectColor(){

  QColor color = QColorDialog::getColor(Qt::white, this, "color dialog");
  if (color.isValid()){
    GetVizmo().mR = (double)(color.red()) / 255.0;
    GetVizmo().mG = (double)(color.green()) / 255.0;
    GetVizmo().mB = (double)(color.blue()) / 255.0;
  }
  GetVizmo().ChangeAppearance(3);
}

/*void                                ANOTHER TEMPORARY DISABLING 
  RoadmapOptions::UpdateNodeCfg(){

  if(m_nodeGUI != NULL && m_nodeGUI->isVisible() && m_nodeGUI->filledFirstTime==false){
  if(!GetMapModel()->GetNodeList().empty()){
  CGLModel* n = GetMapModel()->GetNodeList().front(); 
  CCfg* cfg = (CCfg*)n;   
  vector<double> VNodeCfg;
  VNodeCfg.clear();

  VNodeCfg =  m_nodeGUI->getNodeCfg();

  cfg->SetCfg(VNodeCfg);
  cfg->GetCC()->ReBuildAll();
//emit callUpdate();
UpdateNodeCfg();  //well, then... 
}
}
}
*/






