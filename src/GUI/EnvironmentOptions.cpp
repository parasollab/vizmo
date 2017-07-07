//Revised
#include <fstream>

#include "EnvironmentOptions.h"

#include "Utilities/ReebGraphConstruction.h"
#include "Models/GraphModel.h"
#include <vector>

#include "Environment/FixedBody.h"
#include "Environment/StaticMultiBody.h"

#include "ChangeBoundaryDialog.h"
#include "EditRobotDialog.h"
#include "FileListDialog.h"
#include "GLWidget.h"
#include "MainMenu.h"
#include "MainWindow.h"
#include "ModelSelectionWidget.h"
#include "EdgeEditDialog.h"
#include "NodeEditDialog.h"
#include "ObstaclePosDialog.h"

#include "Models/EnvModel.h"
#include "Models/EdgeModel.h"
#include "Models/QueryModel.h"
#include "Models/RegionBoxModel.h"
#include "Models/RegionBox2DModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/RegionSphere2DModel.h"
#include "Models/StaticMultiBodyModel.h"
#include "Models/Vizmo.h"


#include "Icons/AddObstacle.xpm"
#include "Icons/ChangeBoundary.xpm"
#include "Icons/DeleteObstacle.xpm"
#include "Icons/DuplicateObstacle.xpm"
#include "Icons/EditRobot.xpm"
#include "Icons/MoveObstacle.xpm"
#include "Icons/RandEnv.xpm"
#include "Icons/RefreshEnv.xpm"
//#include "Icons/Skeleton.xpm"

#include "Icons/PointMode.xpm"
#include "Icons/RobotMode.xpm"


EnvironmentOptions::
EnvironmentOptions(QWidget* _parent) : OptionsBase(_parent, "Environment"),
    m_changeBoundaryDialog(NULL), m_editRobotDialog(NULL) {
  CreateActions();
  SetHelpTips();
  SetUpSubmenu();
  SetUpToolTab();
}

/*------------------------- GUI Management -----------------------------------*/

void
EnvironmentOptions::
CreateActions() {
  //1. Create actions and add them to the map
  m_actions["refreshEnv"] = new QAction(QPixmap(refreshenv),
      tr("Refresh"), this);
  m_actions["randEnvColors"] = new QAction(QPixmap(randEnvIcon),
      tr("Randomize Environment Colors"), this);
  m_actions["addObstacle"] = new QAction(QPixmap(addobstacle),
      tr("Add Obstacle"), this);
  m_actions["deleteObstacle"] = new QAction(QPixmap(deleteobstacle),
      tr("Delete Obstacle(s)"), this);
  m_actions["moveObstacle"] = new QAction(QPixmap(moveobstacle),
      tr("Move Obstacle(s)"), this);
  m_actions["duplicateObstacle"] = new QAction(QPixmap(duplicateobstacle),
      tr("Duplicate Obstacle(s)"), this);
  m_actions["changeBoundary"] = new QAction(QPixmap(changeboundary),
      tr("Change Boundary"), this);
  m_actions["editRobot"] = new QAction(QPixmap(editrobot),
      tr("Edit the Robot"), this);
  m_actions["robotView"] = new QAction(QPixmap(robotmode), tr("Robot"), this);
  m_actions["pointView"] = new QAction(QPixmap(pointmode), tr("Point"), this);
  //revisons
  m_actions["addSkeleton"] = new QAction(QPixmap(editrobot),
      tr("Add a Skeleton"),this);
  m_actions["saveSkeleton"] = new QAction(QPixmap(addobstacle),
      tr("Save the Skeleton"),this);
  m_actions["deleteSelected"] = new QAction(QPixmap(changeboundary),
      tr("Toggle Skeleton Info"), this);
  m_actions["addVertex"] = new QAction(QPixmap(addobstacle),
      tr("Add a node to the skeleton"), this);
  m_actions["addEdge"] = new QAction(QPixmap(addobstacle),
      tr("Add a edge to the skeleton"), this);

  //2. Set other specifications as necessary
  m_actions["refreshEnv"]->setEnabled(false);
  m_actions["robotView"]->setEnabled(false);
  m_actions["pointView"]->setEnabled(false);
  m_actions["addObstacle"]->setEnabled(false);
  m_actions["deleteObstacle"]->setEnabled(false);
  m_actions["moveObstacle"]->setEnabled(false);
  m_actions["duplicateObstacle"]->setEnabled(false);
  m_actions["changeBoundary"]->setEnabled(false);
  m_actions["editRobot"]->setEnabled(false);
  m_actions["randEnvColors"]->setEnabled(false);
  //revisions
  m_actions["addSkeleton"]->setEnabled(false);
  m_actions["saveSkeleton"]->setEnabled(false);
  m_actions["deleteSelected"]->setEnabled(false);
  m_actions["addVertex"]->setEnabled(false);
  m_actions["addEdge"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["refreshEnv"], SIGNAL(triggered()),
      this, SLOT(RefreshEnv()));
  connect(m_actions["randEnvColors"], SIGNAL(triggered()),
      this, SLOT(RandomizeEnvColors()));
  connect(m_actions["robotView"], SIGNAL(triggered()),
      this, SLOT(ClickRobot()));
  connect(m_actions["pointView"], SIGNAL(triggered()),
      this, SLOT(ClickPoint()));
  connect(m_actions["addObstacle"], SIGNAL(triggered()),
      this, SLOT(AddObstacle()));
  connect(m_actions["deleteObstacle"], SIGNAL(triggered()),
      this, SLOT(DeleteObstacle()));
  connect(m_actions["moveObstacle"], SIGNAL(triggered()),
      this, SLOT(MoveObstacle()));
  connect(m_actions["duplicateObstacle"], SIGNAL(triggered()),
      this, SLOT(DuplicateObstacles()));
  connect(m_actions["changeBoundary"], SIGNAL(triggered()),
      this, SLOT(ChangeBoundaryForm()));
  connect(m_actions["editRobot"], SIGNAL(triggered()),
      this, SLOT(EditRobot()));
  //revisions
  connect(m_actions["addSkeleton"],SIGNAL(triggered()),
      this, SLOT(AddSkeleton()));
  connect(m_actions["saveSkeleton"],SIGNAL(triggered()),
      this, SLOT(SaveSkeleton()));
  connect(m_actions["deleteSelected"],SIGNAL(triggered()),
      this, SLOT(DeleteSelectedItems()));
  connect(m_actions["addVertex"],SIGNAL(triggered()),
      this, SLOT(AddVertex()));
  connect(m_actions["addEdge"],SIGNAL(triggered()),
      this, SLOT(AddStraightLineEdge()));

}


void
EnvironmentOptions::
SetHelpTips() {
  m_actions["robotView"]->setStatusTip(tr("Display nodes in robot mode"));
  m_actions["robotView"]->setWhatsThis(tr("Click this button to vizualize"
        " the nodes in <b>Robot</b> mode."));

  m_actions["pointView"]->setStatusTip(tr("Display nodes in point mode"));
  m_actions["pointView"]->setWhatsThis(tr("Click this button to vizualize"
        " the nodes in <b>Point</b> mode."));

  m_actions["randEnvColors"]->setStatusTip(tr("Randomize environment colors"));
  m_actions["randEnvColors"]->setWhatsThis(tr("Click this button to"
        " randomize the colors of the environment objects."));

  m_actions["addObstacle"]->setWhatsThis(tr("Add obstacle button"));
  m_actions["deleteObstacle"]->setWhatsThis(tr("Delete obstacle button"));
  m_actions["moveObstacle"]->setWhatsThis(tr("Move obstacle button"));
  m_actions["duplicateObstacle"]->setWhatsThis(tr("duplicate obstacle button"));
  m_actions["changeBoundary"]->setWhatsThis(tr("Change the boundary"));
  m_actions["editRobot"]->setWhatsThis(tr("Edit the robot"));
  //revisions
  m_actions["addSkeleton"]->setWhatsThis(tr("Add a skeleton"));
  m_actions["saveSkeleton"]->setWhatsThis(tr("Save your Skeleton"));
  m_actions["addVertex"]->setWhatsThis(tr("Add a node to the Skeleton"));
  m_actions["deleteSelected"]->setWhatsThis(tr("Delete Selected Items"));
  m_actions["addEdge"]->setWhatsThis(tr("Add Edge"));
}


void
EnvironmentOptions::
SetUpSubmenu() {
  m_submenu = new QMenu(m_label, this);

  m_submenu->addAction(m_actions["refreshEnv"]);

  m_nodeShape = new QMenu("Change Node Shape", this);
  m_nodeShape->addAction(m_actions["robotView"]);
  m_nodeShape->addAction(m_actions["pointView"]);
  m_submenu->addMenu(m_nodeShape);

  m_obstacleMenu = new QMenu("Obstacle Tools", this);
  m_obstacleMenu->addAction(m_actions["addObstacle"]);
  m_obstacleMenu->addAction(m_actions["deleteObstacle"]);
  m_obstacleMenu->addAction(m_actions["moveObstacle"]);
  m_obstacleMenu->addAction(m_actions["duplicateObstacle"]);
  m_submenu->addMenu(m_obstacleMenu);
  m_submenu->addAction(m_actions["changeBoundary"]);
  m_submenu->addAction(m_actions["editRobot"]);
  m_obstacleMenu->setEnabled(false);
  //revisions
  m_skeletonMenu = new QMenu("Skeleton Tools", this);
  m_skeletonMenu->addAction(m_actions["saveSkeleton"]);
  m_skeletonMenu->addAction(m_actions["addSkeleton"]);
  m_skeletonMenu->addAction(m_actions["addVertex"]);
  m_skeletonMenu->addAction(m_actions["deleteSelected"]);
  m_skeletonMenu->addAction(m_actions["addEdge"]);
  m_submenu->addMenu(m_skeletonMenu);
  m_skeletonMenu->setEnabled(false);

  m_nodeShape->setEnabled(false);
}


void
EnvironmentOptions::
SetUpToolTab() {
  vector<string> buttonList;

  buttonList.push_back("robotView");
  buttonList.push_back("pointView");
  buttonList.push_back("randEnvColors");
  buttonList.push_back("_separator_");

  buttonList.push_back("addObstacle");
  buttonList.push_back("duplicateObstacle");
  buttonList.push_back("deleteObstacle");
  buttonList.push_back("moveObstacle");
  buttonList.push_back("_separator_");

  buttonList.push_back("changeBoundary");
  buttonList.push_back("_separator_");

  buttonList.push_back("editRobot");
  buttonList.push_back("_separator_");
  //revisions
  buttonList.push_back("addSkeleton");
  buttonList.push_back("saveSkeleton");
  buttonList.push_back("addVertex");
  buttonList.push_back("deleteSelected");
  buttonList.push_back("addEdge");
  buttonList.push_back("_separator_");

  CreateToolTab(buttonList);
}


void
EnvironmentOptions::
Reset() {
  m_nodeShape->setEnabled(true);
  m_actions["robotView"]->setEnabled(true);
  m_actions["pointView"]->setEnabled(true);
  m_actions["randEnvColors"]->setEnabled(true);

  ClickRobot();

  m_actions["refreshEnv"]->setEnabled(true);
  m_actions["addObstacle"]->setEnabled(true);
  m_actions["deleteObstacle"]->setEnabled(true);
  m_actions["moveObstacle"]->setEnabled(true);
  m_actions["duplicateObstacle"]->setEnabled(true);
  m_actions["changeBoundary"]->setEnabled(true);
  m_actions["editRobot"]->setEnabled(true);
  //revisions
  m_actions["addSkeleton"]->setEnabled(true);
  m_actions["saveSkeleton"]->setEnabled(true);
  m_actions["addVertex"]->setEnabled(true);
  m_actions["deleteSelected"]->setEnabled(true);
  m_actions["addEdge"]->setEnabled(true);

  m_obstacleMenu->setEnabled(true);
}

/*----------------------- Environment Editing --------------------------------*/

//revisions
void
EnvironmentOptions::
AddSkeleton() {

  /// Handles all local data types, including:
  /// \arg \c .env
  /// \arg \c .map
  /// \arg \c .query
  /// \arg \c .path
  /// \arg \c .vd
  /// \arg \c .xml
  //  \arg \c .graph
  QString fn = QFileDialog::getOpenFileName(this,
      "Choose a skeleton to open", GetMainWindow()->GetLastDir(),
      "Files (*.env *.map *.query *.path *.vd *.xml *.graph)");
  typedef stapl::sequential::directed_preds_graph<
   stapl::MULTIEDGES, Point3d,vector<Point3d>> GraphType;
  if(!fn.isEmpty()){
    QFileInfo fi(fn);
    GetMainWindow()->statusBar()->showMessage("Loading:"+fi.absoluteFilePath());
    GetMainWindow()->AlertUser("Loaded Skeleton: "+fi.fileName().toStdString());

    //Read the file
    ifstream input(fi.absoluteFilePath().toStdString());
    if(!input.good())
      GetMainWindow()->AlertUser("File Not Good");
    GraphType _g;
    size_t _numVert, _numEdges;
    input >> _numVert >> _numEdges;//extract from file the number of v and e
    //Get points
    for(size_t i = 0; i < _numVert; i++){
      Point3d _p1;
      size_t index;
      input >> index >> _p1 ;
      _g.add_vertex(index, _p1);// THIS IS WHAT I NEED
    }
    //Get Edges
    for(size_t i = 0; i < _numEdges; i++){
      size_t _v1, _v2;
      input >> _v1 >> _v2;
      size_t _interVerts;
      vector<Point3d> _edge;
      input >> _interVerts;
      for(size_t j = 0; j < _interVerts; j++){
        Point3d _p;
        input >> _p;
        _edge.push_back(_p);
      }
      _g.add_edge(_v1,_v2,_edge);
    }
    //Make A Graph Model
    EnvModel* env = GetVizmo().GetEnv();
    env->AddGraphModel(_g);
    auto _gm = env->GetGraphModel();
    //Update Environment
    if(_gm)
      _gm->SetRenderMode(SOLID_MODE);
		RefreshEnv();
  }
  else
    GetMainWindow()->statusBar()->showMessage("Loading aborted");

}
//revisions
void
EnvironmentOptions::
SaveSkeleton() {
  //save skeleton in the workspace

  QString fn = QFileDialog::getSaveFileName(this, "Chose a file name for the Skeleton",
      GetMainWindow()->GetLastDir(), "Files  (*.graph)");

  if(!fn.isEmpty()){
     EnvModel* env = GetVizmo().GetEnv();
     QFileInfo fi(fn);
     ofstream output(fi.absoluteFilePath().toStdString());
     env->SaveGraphFile(output);
   }
   else
     GetMainWindow()->statusBar()->showMessage("Saving aborted", 2000);
}



//revisions
void
EnvironmentOptions::
AddStraightLineEdge() {
  //By default, just attempts straight line and does not pop up EdgeEditDialog
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  vector<CfgModel*> selNodes;
  EnvModel* env = GetVizmo().GetEnv();
  GraphModel::SkeletonGraphType*  _gm = env->GetGraphModel()->GetGraph();

//  Filter away selected edges, but still enforce two nodes
for(auto it = sel.begin(); it != sel.end(); it++)
    if((*it)->Name().substr(0, 4) == "Node")
      selNodes.push_back((CfgModel*)*it);

  if(selNodes.size() != 2) {
    GetMainWindow()->AlertUser("Please select exactly two nodes.");
    return;
  }

       size_t v0 = selNodes[0]->GetIndex();
    size_t v1 = selNodes[1]->GetIndex();
//    _gm->add_edge(v0, v1);
    vector<CfgModel> intermediates;
   intermediates.push_back((*_gm->find_vertex(v0)).property());
         intermediates.push_back((*_gm->find_vertex(v1)).property());

//SkeletonGraphType::vertex_iterator viTemp, viTemp2;
 //   _gm->add_edge(v0, v1);
 // _gm->AddEdge(v0,v1);
// SkeletonGraphType:: adj_edge_iterator eiTemp; //eiTemp2;

//*_gm->find_edge(SkeletonGraphType::edge_descriptor(v1,v2), viTemp, eiTemp);
         _gm->add_edge(v0, v1, EdgeModel("",1, intermediates));
    //Set edge weights in underlying graph
 // _gm->AddEdge(v0,v1);
    env->GetGraphModel()->Build();
    RefreshEnv();
    GetMainWindow()->GetModelSelectionWidget()->ResetLists();
  sel.clear();




}

void
EnvironmentOptions::
AddVertex()  {
  //Adds a vertex to the skeleton

    NodeEditDialog* ned = new NodeEditDialog(GetMainWindow(), "New Vertex");
    GetMainWindow()->ShowDialog(ned);
 }

void
EnvironmentOptions::
DeleteSelectedItems() {
  //Deletes selected items from the skeleton
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  EnvModel* env = GetVizmo().GetEnv();
  GraphModel::SkeletonGraphType* _gm = env-> GetGraphModel()->GetGraph();

  bool selectionValid = false;
  vector<GraphModel::SkeletonGraphType::vertex_descriptor> nodesToDelete;
  vector<pair<GraphModel::SkeletonGraphType::vertex_descriptor, GraphModel::SkeletonGraphType::vertex_descriptor> > edgesToDelete;

  //Mark selected items for removal
  for(auto it = sel.begin(); it != sel.end(); it++) {
    string objName = (*it)->Name();
    if(objName.substr(0, 4) == "Node") {
      selectionValid = true;
      auto vid = ((CfgModel*)(*it))->GetIndex();
      nodesToDelete.push_back(vid);
    }
    else if(objName.substr(0, 4) == "Edge") {
      selectionValid = true;
      EdgeModel* e = (EdgeModel*)(*it);
      edgesToDelete.push_back(make_pair(e->GetStartCfg()->GetIndex(),
            e->GetEndCfg()->GetIndex()));
    }
  }

  if(selectionValid == false)
    GetMainWindow()->AlertUser("Please select a group of nodes and edges to"
        " remove.");
  else {
    //Remove selected vertices
    typedef vector<size_t>::iterator VIT;
    for(auto it = nodesToDelete.begin(); it != nodesToDelete.end(); it++)
       _gm->delete_vertex(*it);
    //Remove selected edges
    typedef vector<pair<size_t, size_t> >::iterator EIT;
    for(auto it = edgesToDelete.begin(); it != edgesToDelete.end(); it++) {
       _gm->delete_edge(it->first, it->second);
    }
    env-> GetGraphModel()->Build();
    RefreshEnv();
    GetMainWindow()->GetModelSelectionWidget()->ResetLists();
    sel.clear();
    cout << "deleted" << endl;
  }

}

void
EnvironmentOptions::
AddObstacle() {
  //get the name of an obstacle file
  QString fn = QFileDialog::getOpenFileName(this, "Choose an obstacle to load",
      GetMainWindow()->GetLastDir(), "Files  (*.g *.obj)");

  if(!fn.isEmpty()) {
    //save last directory
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());

    //add the new obstacle to the environment and select it
    shared_ptr<StaticMultiBodyModel> m = GetVizmo().GetEnv()->AddObstacle(
        fi.absolutePath().toStdString(),
        fi.fileName().toStdString(),
        Transformation());
    RefreshEnv();

    //Select new obstacle
    GetVizmo().GetSelectedModels().clear();
    GetVizmo().GetSelectedModels().push_back(m.get());

    //open the obstacle position dialog for the new obstacle
    vector<StaticMultiBodyModel*> v(1, m.get());
    ObstaclePosDialog* opd = new ObstaclePosDialog(GetMainWindow(), v);
    GetMainWindow()->ShowDialog(opd);
  }
  else
    GetMainWindow()->statusBar()->showMessage("Loading aborted");
}


void
EnvironmentOptions::
DeleteObstacle() {
  vector<StaticMultiBodyModel*> toDel;
  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  //grab the bodies from the selected vector
  for(auto& s : sel)
    if(s->Name() == "StaticMultiBody")
      toDel.push_back(static_cast<StaticMultiBodyModel*>(s));

  //alert that only non-active multibodies can be selected
  if(toDel.empty() || toDel.size() != sel.size())
    GetMainWindow()->AlertUser(
        "Must select one or more static multibodies only.");

  //successful selection, delete obstacle(s)
  else {
    for(auto& model : toDel)
      GetVizmo().GetEnv()->DeleteObstacle(model);
    GetVizmo().GetSelectedModels().clear();
    RefreshEnv();
  }
}


void
EnvironmentOptions::
MoveObstacle() {
  vector<StaticMultiBodyModel*> toMove;
  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  //grab the bodies from the selected vector
  for(auto& s : sel)
    if(s->Name().find("MultiBody") != string::npos &&
        s->Name() != "ActiveMultiBody")
      toMove.push_back(static_cast<StaticMultiBodyModel*>(s));

  //alert that only non-active multibodies can be selected
  if(toMove.empty() || toMove.size() != sel.size())
    GetMainWindow()->AlertUser(
        "Must select one or more non-active multibodies only.");

  //successful selection, show ObstaclePosDialog
  else {
    ObstaclePosDialog* opd = new ObstaclePosDialog(GetMainWindow(), toMove);
    GetMainWindow()->ShowDialog(opd);
  }
}


void
EnvironmentOptions::
DuplicateObstacles() {
  vector<StaticMultiBodyModel*> toCopy;
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  for(auto& s : sel)
    if(s->Name() == "StaticMultiBody")
      toCopy.push_back(static_cast<StaticMultiBodyModel*>(s));

  //alert that only non-active multibodies can be selected
  if(toCopy.empty() || toCopy.size() != sel.size())
    GetMainWindow()->AlertUser(
        "Must select one or more non-active multibodies only.");

  //successful selection, copy and show ObstaclePosDialog
  else {
    vector<StaticMultiBodyModel*> copies;
    for(auto& o : toCopy) {
      auto body = o->GetStaticMultiBody()->GetFixedBody(0);
      cout << "Filename: " << body->GetFileName() << endl;
      shared_ptr<StaticMultiBodyModel> newo = GetVizmo().GetEnv()->AddObstacle(
          "", body->GetFileName(), body->GetWorldTransformation()
          );
      copies.push_back(newo.get());
    }
    sel.clear();
    copy(copies.begin(), copies.end(), back_inserter(sel));

    ObstaclePosDialog* opd = new ObstaclePosDialog(GetMainWindow(), copies);
    GetMainWindow()->ShowDialog(opd);
    RefreshEnv();
  }
}


void
EnvironmentOptions::
ChangeBoundaryForm() {
  if(m_changeBoundaryDialog == NULL) {
    vector<Model*>& sel = GetVizmo().GetSelectedModels();
    //alert that only the boundary should be selected
    if(sel.size() != 1 || !(sel[0]->Name() == "Bounding Box" ||
          sel[0]->Name() == "Bounding Sphere")) {
      GetMainWindow()->AlertUser("Must select only the boundary.");
    }
    //successful selection, show ChangeBoundaryDialog
    else {
      m_changeBoundaryDialog = new ChangeBoundaryDialog(GetMainWindow());
      GetMainWindow()->ShowDialog(m_changeBoundaryDialog);
    }
  }
}


void
EnvironmentOptions::
EditRobot() {
  if(m_editRobotDialog == NULL) {
    m_editRobotDialog = new EditRobotDialog(GetMainWindow());
    GetMainWindow()->ShowDialog(m_editRobotDialog);
  }
}

/*----------------------- Environment Display --------------------------------*/

void
EnvironmentOptions::
RefreshEnv() {
  GetVizmo().GetEnv()->SetRenderMode(SOLID_MODE);
  GetMainWindow()->GetModelSelectionWidget()->reset();
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
}

void
EnvironmentOptions::
ClickRobot() {
  CfgModel::SetShape(CfgModel::Robot);
  if(GetVizmo().IsQueryLoaded())
    GetVizmo().GetQry()->Build();
}


void
EnvironmentOptions::
ClickPoint() {
  CfgModel::SetShape(CfgModel::Point);
  if(GetVizmo().IsQueryLoaded())
    GetVizmo().GetQry()->Build();
}


void
EnvironmentOptions::
RandomizeEnvColors() {
  GetVizmo().GetEnv()->ChangeColor();
}
