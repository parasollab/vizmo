/***********************************************
 * Implementation for RoadmapOptions.
 * This class has submenus WITHIN m_submenu, so
 * SetUpCustomSubmenu() is used instead of the
 * SetUpSubmenu() provided in the base class.
 * Actions in these smaller submenus are
 * grouped together in the code.
 ***********************************************/

#include "RoadmapOptions.h"

#include <QtGui>

#include "SliderDialog.h"
#include "NodeEditDialog.h"
#include "EdgeEditDialog.h"
#include "OptionsBase.h"
#include "GLWidget.h"
#include "MainWindow.h"
#include "ModelSelectionWidget.h"
#include "Models/DebugModel.h"
#include "Models/RobotModel.h"
#include "Models/QueryModel.h"
#include "Models/Vizmo.h"
#include "Utilities/AlertUser.h"

#include "Icons/RColor.xpm"
#include "Icons/CCsOneColor.xpm"
#include "Icons/NodeSize.xpm"
#include "Icons/EdgeThickness.xpm"
#include "Icons/Navigate.xpm"
#include "Icons/EditNode.xpm"
#include "Icons/EditEdge.xpm"
#include "Icons/AddNode.xpm"
#include "Icons/AddEdge.xpm"
#include "Icons/MergeNodes.xpm"

#include "Icons/DeleteSelected.xpm"

RoadmapOptions::
RoadmapOptions(QWidget* _parent, MainWindow* _mainWindow)
  : OptionsBase(_parent, _mainWindow) {
    CreateActions();
    SetUpCustomSubmenu();
    //SetUpToolbar(); currently using tool tabs
    SetUpToolTab();
    SetHelpTips();
  }

void
RoadmapOptions::
CreateActions() {

  //1. Create actions/additional submenus and add them to the map
  QAction* showHideRoadmap = new QAction(QPixmap(navigate), tr("Show Roadmap"), this);
  m_actions["showHideRoadmap"] = showHideRoadmap;
  QAction* scaleNodes = new QAction(QPixmap(nodeSizeIcon), tr("Scale Nodes"), this);
  m_actions["scaleNodes"] = scaleNodes;
  m_nodeSizeDialog = new SliderDialog("Node Scaling",
      "Node Scale", 0, 2500, 1000, this);
  QAction* edgeThickness = new QAction(QPixmap(edgeThicknessIcon), tr("Change Edge Thickness"), this);
  m_actions["edgeThickness"] = edgeThickness;
  m_edgeThicknessDialog = new SliderDialog("Edge Thickness",
      "Edge Thickness", 100, 1000, 100, this);
  QAction* editNode = new QAction(QPixmap(editnode), tr("Edit Node"), this);
  m_actions["editNode"] = editNode;
  QAction* editEdge = new QAction(QPixmap(editedge), tr("Edit Edge"), this);
  m_actions["editEdge"] = editEdge;
  QAction* addNode = new QAction(QPixmap(addnode), tr("Add Node"), this);
  m_actions["addNode"] = addNode;
  QAction* addEdge = new QAction(QPixmap(addedge), tr("Add Straight Line Edge"), this);
  m_actions["addEdge"] = addEdge;
  QAction* deleteSelected = new QAction(QPixmap(deleteselected), tr("Delete Selected Items"), this);
  m_actions["deleteSelected"] = deleteSelected;
  QAction* mergeNodes = new QAction(QPixmap(mergenodes), tr("Merge Nodes"), this);
  m_actions["mergeNodes"] = mergeNodes;
  QAction* randomizeColors = new QAction(QPixmap(rcolor), tr("Randomize Colors"), this);
  m_actions["randomizeColors"] = randomizeColors;
  QAction* ccsOneColor = new QAction(QPixmap(ccsOneColorIcon), tr("Make All One Color"), this);
  m_actions["ccsOneColor"] = ccsOneColor;
  QAction* saveStart = new QAction(tr("Save Query Start"), this);
  m_actions["saveStart"] = saveStart;
  QAction* saveGoal = new QAction(tr("Save Query Goal"), this);
  m_actions["saveGoal"] = saveGoal;

  //2. Set other specifications as necessary
  m_actions["showHideRoadmap"]->setCheckable(true);
  m_actions["showHideRoadmap"]->setEnabled(false);
  m_actions["showHideRoadmap"]->setStatusTip("Display or hide the roadmap");
  m_actions["showHideRoadmap"]->setToolTip("Show/Hide Roadmap");

  m_actions["scaleNodes"]->setShortcut(tr("CTRL+S"));
  m_actions["scaleNodes"]->setEnabled(false);
  m_actions["scaleNodes"]->setStatusTip(tr("Resize the nodes"));

  m_actions["edgeThickness"]->setEnabled(false);
  m_actions["edgeThickness"]->setStatusTip(tr("Change edge thickness"));

  m_actions["editNode"]->setEnabled(false);
  m_actions["editNode"]->setStatusTip(tr("Modify a roadmap node"));
  m_actions["editEdge"]->setEnabled(false);
  m_actions["editEdge"]->setStatusTip(tr("Modify a roadmap edge"));
  m_actions["addNode"]->setEnabled(false);
  m_actions["addNode"]->setStatusTip(tr("Add a new node the the roadmap"));
  m_actions["addEdge"]->setEnabled(false);
  m_actions["addEdge"]->setStatusTip(tr("Add edge between selected pair of nodes"));
  m_actions["deleteSelected"]->setEnabled(false);
  m_actions["deleteSelected"]->setStatusTip(tr("Delete selected nodes and/or edges"));
  m_actions["mergeNodes"]->setEnabled(false);
  m_actions["mergeNodes"]->setStatusTip(tr("Merge selected nodes into a supervertex"));

  m_actions["randomizeColors"]->setShortcut(tr("CTRL+R"));
  m_actions["randomizeColors"]->setEnabled(false);
  m_actions["randomizeColors"]->setStatusTip(tr("Randomize CC colors"));
  m_actions["ccsOneColor"]->setEnabled(false);
  m_actions["ccsOneColor"]->setStatusTip(tr("Make all CCs one color"));

  //3. Make connections
  connect(m_actions["showHideRoadmap"], SIGNAL(triggered()), this, SLOT(ShowRoadmap()));
  connect(m_actions["scaleNodes"], SIGNAL(triggered()), this, SLOT(ShowNodeSizeDialog()));
  connect(m_nodeSizeDialog->GetSlider(), SIGNAL(valueChanged(int)), this, SLOT(ScaleNodes()));

  connect(m_actions["edgeThickness"], SIGNAL(triggered()), this, SLOT(ShowEdgeThicknessDialog()));
  connect(m_edgeThicknessDialog->GetSlider(), SIGNAL(valueChanged(int)), this, SLOT(ChangeEdgeThickness()));

  connect(m_actions["editNode"], SIGNAL(triggered()), this, SLOT(ShowNodeEditDialog()));
  connect(m_actions["editEdge"], SIGNAL(triggered()), this, SLOT(ShowEdgeEditDialog()));
  connect(m_actions["addNode"], SIGNAL(triggered()), this, SLOT(AddNode()));
  connect(m_actions["addEdge"], SIGNAL(triggered()), this, SLOT(AddStraightLineEdge()));
  connect(m_actions["deleteSelected"], SIGNAL(triggered()), this, SLOT(DeleteSelectedItems()));
  connect(m_actions["mergeNodes"], SIGNAL(triggered()), this, SLOT(MergeSelectedNodes()));

  connect(m_actions["randomizeColors"], SIGNAL(triggered()), this, SLOT(RandomizeCCColors()));
  connect(m_actions["ccsOneColor"], SIGNAL(triggered()), this, SLOT(MakeCCsOneColor()));
  connect(m_mainWindow->GetGLWidget(), SIGNAL(selectByRMB()), this, SLOT(ShowObjectContextMenu()));
}

void
RoadmapOptions::
SetUpCustomSubmenu() {

  m_submenu = new QMenu("Roadmap", this);

  m_submenu->addAction(m_actions["showHideRoadmap"]);
  m_submenu->addAction(m_actions["scaleNodes"]);
  m_submenu->addAction(m_actions["edgeThickness"]);
  m_submenu->addAction(m_actions["editNode"]);
  m_submenu->addAction(m_actions["editEdge"]);
  m_submenu->addAction(m_actions["addNode"]);
  m_submenu->addAction(m_actions["addEdge"]);
  m_submenu->addAction(m_actions["deleteSelected"]);
  m_submenu->addAction(m_actions["mergeNodes"]);

  m_modifyCCs = new QMenu("Modify CCs", this);
  m_modifyCCs->addAction(m_actions["randomizeColors"]);
  m_modifyCCs->addAction(m_actions["ccsOneColor"]);
  m_submenu->addMenu(m_modifyCCs);

  //Disable the extra submenus by default
  m_modifyCCs->setEnabled(false);
}

void
RoadmapOptions::
SetUpToolbar() {

  m_toolbar = new QToolBar(m_mainWindow);

  m_toolbar->addAction(m_actions["showHideRoadmap"]);
  m_toolbar->addAction(m_actions["scaleNodes"]);
  m_toolbar->addAction(m_actions["edgeThickness"]);
  m_toolbar->addAction(m_actions["randomizeColors"]);
  m_toolbar->addAction(m_actions["ccsOneColor"]);
  m_toolbar->addSeparator();
  m_toolbar->addAction(m_actions["editNode"]);
  m_toolbar->addAction(m_actions["editEdge"]);
}

void
RoadmapOptions::
SetUpToolTab() {
  vector<string> buttonList;
  buttonList.push_back("showHideRoadmap");

  buttonList.push_back("ccsOneColor");
  buttonList.push_back("randomizeColors");
  buttonList.push_back("scaleNodes");
  buttonList.push_back("edgeThickness");
  buttonList.push_back("_separator_");

  buttonList.push_back("editNode");
  buttonList.push_back("editEdge");

  CreateToolTab(buttonList);
}

void
RoadmapOptions::
Reset() {

  if(m_actions["showHideRoadmap"] != NULL){
    m_actions["showHideRoadmap"]->setEnabled(GetVizmo().IsRoadMapLoaded());
    m_actions["showHideRoadmap"]->setChecked(true);
    if(GetVizmo().IsRoadMapLoaded())
    {
      ShowRoadmap();
    }
  }
  m_actions["scaleNodes"]->setEnabled(true);
  m_actions["edgeThickness"]->setEnabled(true);
  m_actions["editNode"]->setEnabled(true);
  m_actions["editEdge"]->setEnabled(true);
  m_actions["addNode"]->setEnabled(true);
  m_actions["addEdge"]->setEnabled(true);
  m_actions["deleteSelected"]->setEnabled(true);
  m_actions["mergeNodes"]->setEnabled(true);
  m_modifyCCs->setEnabled(true);
  m_actions["randomizeColors"]->setEnabled(true);
  m_actions["ccsOneColor"]->setEnabled(true);

  m_nodeSizeDialog->Reset();
  m_edgeThicknessDialog->Reset();
}

void
RoadmapOptions::
SetHelpTips() {

  m_actions["showHideRoadmap"]->setWhatsThis(tr("Click this button"
        " to visualize the <b>Roadmap</b>. You can also select the"
        " <b>Show/Hide Roadmap</b> option from the <b>Roadmap</b> menu."));
  m_actions["scaleNodes"]->setWhatsThis(tr("Click this button to resize the"
        " roadmap nodes by a specified factor."));
  m_actions["edgeThickness"]->setWhatsThis(tr("Click this button to scale the"
        " thickness of the edges."));
  m_actions["editNode"]->setWhatsThis(tr("Click this button to open the node editing"
        " widget."));
  m_actions["editEdge"]->setWhatsThis(tr("Click this button to open the edge editing"
        " widget."));
  m_actions["addNode"]->setWhatsThis(tr("Click this button to add a new node to the"
        " roadmap."));
  m_actions["addEdge"]->setWhatsThis(tr("Click this button to add an edge between two"
        " selected nodes."));
  m_actions["deleteSelected"]->setWhatsThis(tr("Click this button to delete any number"
        " of selected nodes and/or edges"));
  m_actions["mergeNodes"]->setWhatsThis(tr("Click this button to merge selected nodes"
        " into a supervertex."));
  m_actions["randomizeColors"]->setWhatsThis(tr("Click this button to randomize"
        " the colors of the connected components."));
  m_actions["ccsOneColor"]->setWhatsThis(tr("Click this button to set all of the"
        " connected components to a single color. "));
}

//Slots

void
RoadmapOptions::
ShowRoadmap() {
  GetVizmo().GetMap()->SetRenderMode(m_actions["showHideRoadmap"]->isChecked() ? SOLID_MODE : INVISIBLE_MODE);
  m_mainWindow->GetGLWidget()->updateGL();
}

void
RoadmapOptions::
ShowNodeSizeDialog() {

  //For now, resizing only enabled for point abstraction. For robot,
  //would require extensive local coordinate system aspects
  if(CfgModel::GetShape() != CfgModel::Robot){
    m_mainWindow->ShowDialog(m_nodeSizeDialog);
  }
  else
    AlertUser("You can only resize the nodes in <b>Point</b> mode.");
}

void
RoadmapOptions::
ScaleNodes() {
  double resize = m_nodeSizeDialog->GetSliderValue() / 1000;
  CfgModel::Scale(resize);
  m_mainWindow->GetGLWidget()->updateGL();
}

void
RoadmapOptions::
ShowEdgeThicknessDialog() {
  m_mainWindow->ShowDialog(m_edgeThicknessDialog);
}

void
RoadmapOptions::
ChangeEdgeThickness() {
  double resize = m_edgeThicknessDialog->GetSliderValue() / 100;
  EdgeModel::m_edgeThickness = resize;
  m_mainWindow->GetGLWidget()->updateGL();
}

void
RoadmapOptions::
ShowNodeEditDialog() {
  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  //Filter out incident edges and just use one node if multiple selected
  bool nodeSelected = false;
  CfgModel* actualNode;
  for(MIT it = sel.begin(); it != sel.end(); it++){
    if((*it)->Name().substr(0, 4) == "Node"){
      actualNode = (CfgModel*)*it;
      nodeSelected = true;
      break;
    }
  }
  if(nodeSelected == false){
    AlertUser("Please select a node to modify.");
    return;
  }

  NodeEditDialog* ned = new NodeEditDialog(m_mainWindow, actualNode->Name(),
      actualNode);
  m_mainWindow->ShowDialog(ned);
}

void
RoadmapOptions::
ShowEdgeEditDialog() {
  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  bool edgeSelected = false;
  EdgeModel* actualEdge;
  for(MIT it = sel.begin(); it != sel.end(); it++){
    if((*it)->Name().substr(0, 4) == "Edge"){
      actualEdge = (EdgeModel*)*it;
      edgeSelected = true;
      break;
    }
  }
  if(edgeSelected == false){
    AlertUser("Please select an edge to modify.");
    return;
  }

  EdgeEditDialog* eed = new EdgeEditDialog(m_mainWindow, actualEdge);
  m_mainWindow->ShowDialog(eed);
}

void
RoadmapOptions::
AddNode() {
  NodeEditDialog* ned = new NodeEditDialog(m_mainWindow, "New Node");
  m_mainWindow->ShowDialog(ned);
}

void
RoadmapOptions::
AddStraightLineEdge() {
  //By default, just attempts straight line and does not pop up EdgeEditDialog

  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  vector<CfgModel*> selNodes;
  Map* map = GetVizmo().GetMap();
  Graph* graph = map->GetGraph();

  //Filter away selected edges, but still enforce two nodes
  for(MIT it = sel.begin(); it != sel.end(); it++)
    if((*it)->Name().substr(0, 4) == "Node")
      selNodes.push_back((CfgModel*)*it);

  if(selNodes.size() != 2){
    AlertUser("Please select exactly two nodes.");
    return;
  }

  pair<bool, double> visibility = GetVizmo().VisibilityCheck(*selNodes[0], *selNodes[1]);
  if(visibility.first){
    VID v0 = selNodes[0]->GetIndex();
    VID v1 = selNodes[1]->GetIndex();
    graph->add_edge(v0, v1);
    graph->add_edge(v1, v0);

    //Set edge weights in underlying graph
    VI viTemp, viTemp2;
    EI eiTemp, eiTemp2;
    graph->find_edge(EID(v0, v1), viTemp, eiTemp);
    graph->find_edge(EID(v1, v0), viTemp2, eiTemp2);
    (*eiTemp).property().SetWeight(visibility.second);
    (*eiTemp2).property().SetWeight(visibility.second);

    map->RefreshMap();
    m_mainWindow->GetModelSelectionWidget()->ResetLists();
    m_mainWindow->GetGLWidget()->updateGL();
  }
  else
    AlertUser("Cannot add invalid edge!");

  sel.clear();
  map->ClearTempItems();
}

void
RoadmapOptions::
DeleteSelectedItems() {

  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  Map* map = GetVizmo().GetMap();
  Graph* graph = map->GetGraph();

  bool selectionValid = false;
  vector<VID> nodesToDelete;
  vector<pair<VID, VID> > edgesToDelete;

  //Mark selected items for removal
  for(MIT it = sel.begin(); it != sel.end(); it++){
    string objName = (*it)->Name();
    if(objName.substr(0, 4) == "Node"){
      selectionValid = true;
      VID vid = ((CfgModel*)(*it))->GetIndex();
      nodesToDelete.push_back(vid);
    }
    else if(objName.substr(0, 4) == "Edge"){
      selectionValid = true;
      EdgeModel* e = (EdgeModel*)(*it);
      edgesToDelete.push_back(make_pair(e->GetStartCfg()->GetIndex(), e->GetEndCfg()->GetIndex()));
    }
  }
  if(selectionValid == false)
    AlertUser("Please select a group of nodes and edges to remove.");
  else{
    //Remove selected vertices
    typedef vector<VID>::iterator VIT;
    for(VIT it = nodesToDelete.begin(); it != nodesToDelete.end(); it++)
      graph->delete_vertex(*it);
    //Remove selected edges
    typedef vector<pair<VID, VID> >::iterator EIT;
    for(EIT it = edgesToDelete.begin(); it != edgesToDelete.end(); it++){
      graph->delete_edge(it->first, it->second);
      graph->delete_edge(it->second, it->first);
    }
    map->RefreshMap();
    m_mainWindow->GetModelSelectionWidget()->ResetLists();
    m_mainWindow->GetGLWidget()->updateGL();
    sel.clear();
  }
  map->ClearTempItems();
}

void
RoadmapOptions::
MergeSelectedNodes() {
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  vector<CfgModel*> selNodes;
  Map* map = GetVizmo().GetMap();
  Graph* graph = map->GetGraph();

  //Create and center the supervertex preview
  CfgModel* superPreview = new CfgModel();
  bool nodesSelected = false;
  int numSelected = 0;

  //Filter only nodes from mass selection
  for(MIT it = sel.begin(); it != sel.end(); it++){
    if((*it)->Name().substr(0, 4) == "Node"){
      CfgModel* cfg = (CfgModel*)*it;
      *superPreview += *cfg;
      selNodes.push_back(cfg);
      numSelected++;
      nodesSelected = true;
    }
  }
  if(nodesSelected == false){
    AlertUser("Please select a group of nodes.");
    return;
  }
  *superPreview /= numSelected;

  //Mark selected vertices for removal
  vector<VID> toDelete;
  vector<VID> toConnect;
  typedef vector<CfgModel*>::iterator NIT;
  for(NIT it = selNodes.begin(); it != selNodes.end(); it++){
    VID selectedID = (*it)->GetIndex();
    toDelete.push_back(selectedID);
    VI vi = graph->find_vertex(selectedID);
    std::map<VID, bool> superTargets;
    for(EI ei = vi->begin(); ei != vi->end(); ++ei){
      VID targetVID = (*ei).target();
      if(superTargets.count(targetVID) == 0) {
        superTargets[targetVID] = true;
        toConnect.push_back(targetVID);
      }
    }
  }

  m_mainWindow->GetGLWidget()->updateGL();

  NodeEditDialog* ned = new NodeEditDialog(m_mainWindow, "New Supervertex",
      superPreview, toConnect, toDelete);
  m_mainWindow->ShowDialog(ned);
}

void
RoadmapOptions::
RandomizeCCColors() {
  GetVizmo().GetMap()->RandomizeCCColors();
  m_mainWindow->GetGLWidget()->updateGL();
}

void
RoadmapOptions::
MakeCCsOneColor() {
  QColor color = QColorDialog::getColor(Qt::white, this, "color dialog");
  if(color.isValid()) {
    float r = color.red()/255.;
    float g = color.green()/255.;
    float b = color.blue()/255.;
    GetVizmo().GetMap()->SetColor(Color4(r, g, b, 1));
  }
}

void
RoadmapOptions::
ShowObjectContextMenu() {

  QMenu cm(this);

  //Create submenu to set start and goal configs.
  //create it just if RobotModel.has been selected
  string str;

  for(MIT ig = GetVizmo().GetSelectedModels().begin();
      ig!=GetVizmo().GetSelectedModels().end(); ig++)
    str = (*ig)->Name();

  if(str == "Robot"){
    QMenu* cfgs = new QMenu("Set Query", this);  //Special robot menu
    cfgs->setTearOffEnabled(true);
    cfgs->addAction(m_actions["saveStart"]);
    cfgs->addAction(m_actions["saveGoal"]);
    cm.addMenu(cfgs);
  }
  else {
    cm.addSeparator();
  }
  if(cm.exec(QCursor::pos())!= 0) //was -1 for q3 version (index based)
    m_mainWindow->GetGLWidget()->updateGL();
}
