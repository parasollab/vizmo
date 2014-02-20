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
#include "Models/Vizmo.h"

#include "Icons/Pallet.xpm"
#include "Icons/MakeSolid.xpm"
#include "Icons/MakeWired.xpm"
#include "Icons/MakeInvisible.xpm"
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
#include "Icons/RobotMode.xpm"
#include "Icons/PointMode.xpm"
#include "Icons/DeleteSelected.xpm"

RoadmapOptions::RoadmapOptions(QWidget* _parent, MainWindow* _mainWindow)
  : OptionsBase(_parent, _mainWindow) {
    CreateActions();
    SetUpCustomSubmenu();
    SetUpToolbar();
    SetHelpTips();
  }

void
RoadmapOptions::CreateActions(){

  //1. Create actions/additional submenus and add them to the map
  QAction* showHideRoadmap = new QAction(QPixmap(navigate), tr("Show Roadmap"), this);
  m_actions["showHideRoadmap"] = showHideRoadmap;

  QAction* robotView = new QAction(QPixmap(robotmode), tr("Robot"), this);
  m_actions["robotView"] = robotView;
  QAction* pointView = new QAction(QPixmap(pointmode), tr("Point"), this);
  m_actions["pointView"] = pointView;

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
  m_nodeSizeDialog = new SliderDialog("Node Scaling",
      "Drag the slider to scale the nodes", 0, 2500, 1000, this);

  QAction* edgeThickness = new QAction(QPixmap(edgeThicknessIcon), tr("Change Edge Thickness"), this);
  m_actions["edgeThickness"] = edgeThickness;
  m_edgeThicknessDialog = new SliderDialog("Edge Thickness",
      "Drag the slider to change the edge thickness", 100, 1000, 100, this);

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
  QAction* changeObjectColor = new QAction(tr("Change Color"), this);
  m_actions["changeObjectColor"] = changeObjectColor;

  //2. Set other specifications as necessary
  m_actions["showHideRoadmap"]->setCheckable(true);
  m_actions["showHideRoadmap"]->setEnabled(false);
  m_actions["showHideRoadmap"]->setStatusTip("Display or hide the roadmap");
  m_actions["showHideRoadmap"]->setToolTip("Show/Hide Roadmap");
  m_actions["robotView"]->setEnabled(false);
  m_actions["robotView"]->setStatusTip(tr("Display nodes in robot mode"));
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

  connect(m_actions["robotView"], SIGNAL(triggered()), this, SLOT(ClickRobot())); //would be better as single function
  connect(m_actions["pointView"], SIGNAL(triggered()), this, SLOT(ClickPoint()));

  connect(m_actions["makeSolid"], SIGNAL(triggered()), this, SLOT(MakeSolid()));
  connect(m_actions["makeWired"], SIGNAL(triggered()), this, SLOT(MakeWired()));
  connect(m_actions["makeInvisible"], SIGNAL(triggered()), this, SLOT(MakeInvisible()));
  connect(m_actions["changeNodeColor"], SIGNAL(triggered()), this, SLOT(ChangeObjectColor()));

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
  connect(m_actions["changeObjectColor"], SIGNAL(triggered()), this, SLOT(ChangeObjectColor()));
  connect(m_mainWindow->GetGLScene(), SIGNAL(selectByRMB()), this, SLOT(ShowObjectContextMenu()));
}

void
RoadmapOptions::SetUpCustomSubmenu(){

  m_submenu = new QMenu("Roadmap", this);

  m_submenu->addAction(m_actions["showHideRoadmap"]);

  m_nodeShape = new QMenu("Change Node Shape", this);
  m_nodeShape->addAction(m_actions["robotView"]);
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
  m_nodeShape->setEnabled(false);
  m_modifySelected->setEnabled(false);
  m_modifyCCs->setEnabled(false);
}

void
RoadmapOptions::SetUpToolbar(){

  m_toolbar = new QToolBar(m_mainWindow);

  m_toolbar->addAction(m_actions["showHideRoadmap"]);
  m_toolbar->addAction(m_actions["robotView"]);
  m_toolbar->addAction(m_actions["pointView"]);
  m_toolbar->addSeparator();
  m_toolbar->addAction(m_actions["makeSolid"]);
  m_toolbar->addAction(m_actions["makeWired"]);
  m_toolbar->addAction(m_actions["makeInvisible"]);
  m_toolbar->addAction(m_actions["changeNodeColor"]);
  m_toolbar->addAction(m_actions["scaleNodes"]);
  m_toolbar->addAction(m_actions["edgeThickness"]);
  m_toolbar->addAction(m_actions["randomizeColors"]);
  m_toolbar->addAction(m_actions["ccsOneColor"]);
  m_toolbar->addSeparator();
  m_toolbar->addAction(m_actions["editNode"]);
  m_toolbar->addAction(m_actions["editEdge"]);
}

void
RoadmapOptions::Reset(){

  if(m_actions["showHideRoadmap"] != NULL){
    m_actions["showHideRoadmap"]->setEnabled(GetVizmo().IsRoadMapLoaded());
    m_actions["showHideRoadmap"]->setChecked(false);
  }

  //Enable m_nodeShape AND its items
  m_nodeShape->setEnabled(true);
  m_actions["robotView"]->setEnabled(true);
  m_actions["pointView"]->setEnabled(true);

  ClickPoint();

  m_modifySelected->setEnabled(true);
  m_actions["makeSolid"]->setEnabled(true);
  m_actions["makeWired"]->setEnabled(true);
  m_actions["makeInvisible"]->setEnabled(true);
  m_actions["changeNodeColor"]->setEnabled(true);

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
RoadmapOptions::SetHelpTips(){

  m_actions["showHideRoadmap"]->setWhatsThis(tr("Click this button"
        " to visualize the <b>Roadmap</b>. You can also select the"
        " <b>Show/Hide Roadmap</b> option from the <b>Roadmap</b> menu."));
  m_actions["robotView"]->setWhatsThis(tr("Click this button to visualize"
        " the nodes in <b>Robot</b> mode."));
  m_actions["pointView"]->setWhatsThis(tr("Click this button to visualize"
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
RoadmapOptions::ShowRoadmap(){
  GetVizmo().GetMap()->SetRenderMode(m_actions["showHideRoadmap"]->isChecked() ? SOLID_MODE : INVISIBLE_MODE);
  m_mainWindow->GetGLScene()->updateGL();
}

void
RoadmapOptions::ClickRobot() {
  CfgModel::SetShape(CfgModel::Robot);
  m_mainWindow->GetGLScene()->updateGL();
}

void
RoadmapOptions::ClickPoint() {
  CfgModel::SetShape(CfgModel::Point);
  m_mainWindow->GetGLScene()->updateGL();
}

void
RoadmapOptions::MakeSolid(){
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  for(MIT i = sel.begin(); i!= sel.end(); i++)
    (*i)->SetRenderMode(SOLID_MODE);
  m_mainWindow->GetGLScene()->updateGL();
}

void
RoadmapOptions::MakeWired(){
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  for(MIT i = sel.begin(); i!= sel.end(); i++)
    (*i)->SetRenderMode(WIRE_MODE);
  m_mainWindow->GetGLScene()->updateGL();
}

void
RoadmapOptions::MakeInvisible(){
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  for(MIT i = sel.begin(); i!= sel.end(); i++)
    (*i)->SetRenderMode(INVISIBLE_MODE);
  m_mainWindow->GetGLScene()->updateGL();
}

void
RoadmapOptions::ShowNodeSizeDialog(){

  //For now, resizing only enabled for point abstraction. For robot,
  //would require extensive local coordinate system aspects
  if(CfgModel::GetShape() != CfgModel::Robot)
    m_nodeSizeDialog->show();
  else
    QMessageBox::about(this, "Sorry!", "You can only resize the nodes in <b>Point</b> mode.");
}

void
RoadmapOptions::ScaleNodes(){
  double resize = m_nodeSizeDialog->GetSliderValue() / 1000;
  CfgModel::Scale(resize);
  m_mainWindow->GetGLScene()->updateGL();
}

void
RoadmapOptions::ShowEdgeThicknessDialog(){
  m_edgeThicknessDialog->show();
}

void
RoadmapOptions::ChangeEdgeThickness(){
  double resize = m_edgeThicknessDialog->GetSliderValue() / 100;
  EdgeModel::m_edgeThickness = resize;
  m_mainWindow->GetGLScene()->updateGL();
}

void
RoadmapOptions::ShowNodeEditDialog(){

  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  Map* map = GetVizmo().GetMap();
  Graph* graph = map->GetGraph();

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
    QMessageBox::about(this, "", "Please select a node to modify.");
    return;
  }

  CfgModel* nodePreview = new CfgModel(*actualNode);
  map->GetTempCfgs().push_back(nodePreview);

  vector<EdgeModel*>& tempEdges = map->GetTempEdges();
  vector<EID> eids; //Save for edge removals if needed later
  VID nodeID = nodePreview->GetIndex();
  VI vi = graph->find_vertex(nodeID);

  for(EI ei = vi->begin(); ei != vi->end(); ++ei){
    eids.push_back((*ei).descriptor());
    EdgeModel* edgePreview = new EdgeModel((*ei).property());
    edgePreview->Set(nodePreview, &(graph->find_vertex((*ei).target())->property()));
    tempEdges.push_back(edgePreview);
  }

  NodeEditDialog n(this, nodePreview, m_mainWindow->GetGLScene(), nodePreview->Name());
  n.SetCurrentEdges(&tempEdges);

  if(n.exec() == QDialog::Accepted){
    if(nodePreview->IsValid()){
      actualNode->SetCfg(nodePreview->GetDataCfg());

      for(size_t i = 0; i < tempEdges.size(); i++){
        if(tempEdges[i]->IsValid() == false){
          graph->delete_edge(eids[i]);
          graph->delete_edge(eids[i].target(), eids[i].source());
        }
      }
    }
    else
      QMessageBox::about(this, "", "Invalid configuration!");

  map->RefreshMap();
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  m_mainWindow->GetGLScene()->updateGL();
  }
  sel.clear();
  map->ClearTempItems();
}

void
RoadmapOptions::ShowEdgeEditDialog(){

  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  Map* map = GetVizmo().GetMap();
  vector<EdgeModel*>& tempEdges = map->GetTempEdges();

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
    QMessageBox::about(this, "", "Please select an edge to modify.");
    return;
  }

  EdgeModel* edgePreview = new EdgeModel(*actualEdge);
  edgePreview->Set(actualEdge->GetStartCfg(), actualEdge->GetEndCfg());
  tempEdges.push_back(edgePreview);

  EdgeEditDialog ed(this, edgePreview, m_mainWindow->GetGLScene());

  if(ed.exec() == QDialog::Accepted){
    if(edgePreview->IsValid())
      actualEdge->SetIntermediates(edgePreview->GetIntermediates());
    else
      //For now, user must start all over again in this case
      QMessageBox::about(this, "", "Invalid edge!");
    sel.clear();
    map->RefreshMap();
    m_mainWindow->GetModelSelectionWidget()->ResetLists();
    m_mainWindow->GetGLScene()->updateGL();
  }
  sel.clear();
  map->ClearTempItems();
}

void
RoadmapOptions::AddNode(){

  Map* map = GetVizmo().GetMap();
  Graph* graph = map->GetGraph();

  CfgModel* previewNode = new CfgModel();
  map->GetTempCfgs().push_back(previewNode);
  NodeEditDialog n(this, previewNode, m_mainWindow->GetGLScene(), "New Node");

  if(n.exec() == QDialog::Accepted){
    if(previewNode->IsValid()){
      CfgModel newNode = *previewNode;
      newNode.SetRenderMode(SOLID_MODE);
      graph->add_vertex(newNode);
      map->RefreshMap();
      m_mainWindow->GetModelSelectionWidget()->ResetLists();
      m_mainWindow->GetGLScene()->updateGL();
    }
    else
      QMessageBox::about(this, "", "Cannot add invalid node!");
  }
  map->ClearTempItems();
}

void
RoadmapOptions::AddStraightLineEdge(){
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
    QMessageBox::about(this, "", "Please select exactly two nodes.");
    return;
  }

  if(GetVizmo().VisibilityCheck(*selNodes[0], *selNodes[1])){
    VID v0 = selNodes[0]->GetIndex();
    VID v1 = selNodes[1]->GetIndex();
    graph->add_edge(v0, v1);
    graph->add_edge(v1, v0);

    map->RefreshMap();
    m_mainWindow->GetModelSelectionWidget()->ResetLists();
    m_mainWindow->GetGLScene()->updateGL();
  }
  else
    QMessageBox::about(this, "", "Cannot add invalid edge!");

  sel.clear();
  map->ClearTempItems();
}

void
RoadmapOptions::DeleteSelectedItems(){

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
    QMessageBox::about(this, "", "Please select a group of nodes and edges to remove.");

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
    m_mainWindow->GetGLScene()->updateGL();
    sel.clear();
  }
  map->ClearTempItems();
}

void
RoadmapOptions::MergeSelectedNodes(){

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
    QMessageBox::about(this, "", "Please select a group of nodes.");
      return;
  }

  *superPreview /= numSelected;

  //Mark selected vertices for removal and save ids for edge addition
  vector<VID> toDelete;
  vector<VID> toAdd;

  typedef vector<CfgModel*>::iterator NIT;
  for(NIT it = selNodes.begin(); it != selNodes.end(); it++){
    VID selectedID = (*it)->GetIndex();
    toDelete.push_back(selectedID);
    VI vi = graph->find_vertex(selectedID);
    for(EI ei = vi->begin(); ei != vi->end(); ++ei){
      //Avoid saving selected node ids..better way to do this?!
      bool newEdge = true;
      for(NIT it2 = selNodes.begin(); it2 != selNodes.end(); it2++){
        if(**it2 == graph->find_vertex((*ei).target())->property()){
          newEdge = false;
          break;
        }
      }
      if(newEdge)
        toAdd.push_back((*ei).target());
    }
  }

  map->GetTempCfgs().push_back(superPreview);

  typedef vector<VID>::iterator VIT;
  vector<EdgeModel*>& tempEdges = map->GetTempEdges();
  for(VIT it = toAdd.begin(); it != toAdd.end(); it++){
    EdgeModel* edgePreview = new EdgeModel();
    CfgModel& c = graph->find_vertex(*it)->property();
    edgePreview->Set(superPreview, &c);
    tempEdges.push_back(edgePreview);
  }

  m_mainWindow->GetGLScene()->updateGL();

  NodeEditDialog n(this, superPreview, m_mainWindow->GetGLScene(), "New Supervertex");
  n.SetCurrentEdges(&tempEdges);

  if(n.exec() == QDialog::Accepted){
    if(superPreview->IsValid()){
      CfgModel super = *superPreview;
      super.SetRenderMode(SOLID_MODE);
      Map::VID superID = graph->add_vertex(super);

      //Add the valid new edges
      for(size_t i = 0; i < tempEdges.size(); i++){
        if(tempEdges[i]->IsValid()){
          graph->add_edge(superID, toAdd[i]);
          graph->add_edge(toAdd[i], superID);
        }
      }
      //Remove selected vertices
      for(VIT it = toDelete.begin(); it != toDelete.end(); it++)
        graph->delete_vertex(*it);

      map->RefreshMap();
    }
    else
      QMessageBox::about(this, "", "Invalid merge!");
  }

  sel.clear();
  map->ClearTempItems();
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
  m_mainWindow->GetGLScene()->updateGL();
}

void
RoadmapOptions::RandomizeCCColors(){
  GetVizmo().GetMap()->RandomizeCCColors();
  m_mainWindow->GetGLScene()->updateGL();
}

void
RoadmapOptions::MakeCCsOneColor(){
  QColor color = QColorDialog::getColor(Qt::white, this, "color dialog");
  if(color.isValid()) {
    float r = color.red()/255.;
    float g = color.green()/255.;
    float b = color.blue()/255.;
    GetVizmo().GetMap()->SetColor(Color4(r, g, b, 1));
  }
}

void
RoadmapOptions::ShowObjectContextMenu(){

  QMenu cm(this);
  cm.addAction(m_actions["makeSolid"]);
  cm.addAction(m_actions["makeWired"]);

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
    cm.addAction(m_actions["makeInvisible"]);
    cm.addAction(m_actions["changeObjectColor"]);
    cm.addSeparator();
  }
  if(cm.exec(QCursor::pos())!= 0) //was -1 for q3 version (index based)
    m_mainWindow->GetGLScene()->updateGL();
}

void
RoadmapOptions::ChangeObjectColor(){

  QColor color = QColorDialog::getColor(Qt::white, this, "color dialog");
  double r, g, b;
  if (color.isValid()){
    r = color.red() / 255.0;
    g = color.green() / 255.0;
    b = color.blue() / 255.0;
  }
  else
    return;

  vector<Model*>& selectedModels = GetVizmo().GetSelectedModels();
  for(MIT mit = selectedModels.begin(); mit != selectedModels.end(); ++mit) {
    Model* model = *mit;
    if(model->Name() == "Robot"){
      model->SetColor(Color4(r, g, b, 1));
      ((RobotModel*)model)->BackUp();
    }
    else
      model->SetColor(Color4(r, g, b, 1));
  }
}
