//Revised
#include <fstream>
#include <map>
#include <algorithm>

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
#include "Models/EllipsoidModel.h"

#include "Icons/CollapseEdge.xpm"
#include "Icons/AddObstacle.xpm"
#include "Icons/ChangeBoundary.xpm"
#include "Icons/DeleteObstacle.xpm"
#include "Icons/DuplicateObstacle.xpm"
#include "Icons/EditRobot.xpm"
#include "Icons/MoveObstacle.xpm"
#include "Icons/RandEnv.xpm"
#include "Icons/RefreshEnv.xpm"
#include "Icons/AddSkeleton.xpm"
#include "Icons/SaveSkeleton.xpm"
#include "Icons/AddVertex.xpm"
#include "Icons/DeleteSelected.xpm"
#include "Icons/AddEdge.xpm"
#include "Icons/CCsOneColor.xpm"
#include "Icons/MergeEdge.xpm"

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
  m_actions["addSkeleton"] = new QAction(QPixmap(addskeleton),
      tr("Add a Skeleton"),this);
  m_actions["saveSkeleton"] = new QAction(QPixmap(saveskeleton),
      tr("Save the Skeleton"),this);
  m_actions["deleteSelected"] = new QAction(QPixmap(deleteselected),
      tr("Delete Selected"), this);
  m_actions["addVertex"] = new QAction(QPixmap(addvertex),
      tr("Add a node to the skeleton"), this);
  m_actions["addEdge"] = new QAction(QPixmap(addedge),
      tr("Add a edge to the skeleton"), this);
  m_actions["colors"] = new QAction(QPixmap(ccsOneColorIcon),
      tr("Change Color"),this);
  m_actions["mergeEdges"] = new QAction(QPixmap(mergeedge),
      tr("Merge Edges"),this);
  m_actions["collapseEdge"] = new QAction(QPixmap(collapseedge),
      tr("Collapse Edge"),this);
  m_actions["loadEllipse"] = new QAction(QPixmap(addskeleton),
      tr("loadEllipse"),this);

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
  m_actions["colors"]->setEnabled(false);
  m_actions["mergeEdges"]->setEnabled(false);
  m_actions["collapseEdge"]->setEnabled(false);
  m_actions["loadEllipse"]->setEnabled(false);

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
  connect(m_actions["colors"], SIGNAL(triggered()),
      this, SLOT(ChangeColor()));
  connect(m_actions["mergeEdges"], SIGNAL(triggered()),
      this, SLOT(MergeEdges()));
  connect(m_actions["collapseEdge"], SIGNAL(triggered()),
      this, SLOT(CollapseEdge()));
  connect(m_actions["loadEllipse"], SIGNAL(triggered()),
      this, SLOT(LoadEllipses()));

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
  m_actions["colors"]->setWhatsThis(tr("Set Color"));
  m_actions["mergeEdges"]->setWhatsThis(tr("Merge Edges"));
  m_actions["collapseEdge"]->setWhatsThis(tr("Collapse Edge"));
  m_actions["loadEllipse"]->setWhatsThis(tr("Load Ellipses"));
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
  m_submenu->addAction(m_actions["loadEllipse"]);
  m_obstacleMenu->setEnabled(false);
  //revisions
  m_skeletonMenu = new QMenu("Skeleton Tools", this);
  m_skeletonMenu->addAction(m_actions["saveSkeleton"]);
  m_skeletonMenu->addAction(m_actions["addSkeleton"]);
  m_skeletonMenu->addAction(m_actions["addVertex"]);
  m_skeletonMenu->addAction(m_actions["addEdge"]);
  m_skeletonMenu->addAction(m_actions["deleteSelected"]);
  m_skeletonMenu->addAction(m_actions["colors"]);
  m_skeletonMenu->addAction(m_actions["mergeEdges"]);
  m_skeletonMenu->addAction(m_actions["collapseEdge"]);
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
  buttonList.push_back("loadEllipse");
  buttonList.push_back("_separator_");
  //revisions
  buttonList.push_back("addSkeleton");
  buttonList.push_back("saveSkeleton");
  buttonList.push_back("addVertex");
  buttonList.push_back("deleteSelected");
  buttonList.push_back("addEdge");
  buttonList.push_back("colors");
  buttonList.push_back("mergeEdges");
  buttonList.push_back("collapseEdge");
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
  m_actions["colors"]->setEnabled(true);
  m_actions["mergeEdges"]->setEnabled(true);
  m_actions["collapseEdge"]->setEnabled(true);
  m_actions["loadEllipse"]->setEnabled(true);

  m_obstacleMenu->setEnabled(true);
}

/*----------------------- Skeleton Editing --------------------------------*/
void EnvironmentOptions::
AddAnnotations() {
  /* Instructions and Key
   *
   * What the code does:
   *  This code will read in an annotation file and color
   *  in the skeleton in vizmo. It must correspond with a
   *  skeleton file.
   *
   *  This currently only works for clearance and energy,
   *  since it needs to decide the intervals. The clearance
   *  interval is decided based on the max and min values
   *  whereas the energy intervals are hardcoded since the
   *  energy function is weird.
   *
   * What you should check:
   *  There are two variables in this function called energy
   *  and detailed. If energy is false it will interpret the
   *  annotation as a clearance annotation. If energy is true
   *  it will interpret it as energy. There is an extra detailed
   *  variable I used for energy when I wanted an annotation
   *  that had more colors, but this is really easy to add for
   *  clearance.
   *
   *  Note: Sorry for not automating this process! Currently
   *  it is very bulky and excessive. You do need to recompile
   *  after you change the two variables. I ended up just having
   *  multiple vizmo executables: energy & clearance.
   *
   * How to use it in vizmo:
   *  When you add a skeleton into vizmo, it will prompt you for
   *  an annotation file. If you don't want one just click cancel.
   *
   */

    // Here are the two variables you need to manually change
    bool energy = false;
    bool detailed = false;

    // Gets annotations and adds it visually to the skeleton
    QString fn = QFileDialog::getOpenFileName(this,
  "Choose an annotation to open", GetMainWindow()->GetLastDir(),
  "Files (*.map)");

    if(!fn.isEmpty()) {
          QFileInfo fi(fn);
          GetMainWindow()->statusBar()->showMessage("Loading:"+fi.absoluteFilePath());
          GetMainWindow()->AlertUser("Loaded Skeleton: "+fi.fileName().toStdString());

          // Read the file
          ifstream annotationsInput(fi.absoluteFilePath().toStdString());
          if(!annotationsInput.good())
                  GetMainWindow()->AlertUser("File Not Good");

          //Metrics to decide the interval
          double maxEl = -10.0, minEl = 10.0, averageEl = 0.0;
          size_t numEl = 0;


          // Extract from file the number of vertices and edges
          size_t numVert, numEdges;
          annotationsInput >> numVert >> numEdges;

          m_vertexMap.clear();
          m_edgeMap.clear();

          // Add values to m_vertexMap and m_edgeMap
          for(size_t i = 0; i < numVert; i++) {
                  size_t index, size;
                  double value; // e.g. energy

                  // Read the vertex descriptor and the co-ordinates
                  annotationsInput >> index >> size >> value;

                  // Add vertex to m_vertexMap
                  m_vertexMap.emplace(index, value);

                  // Metrics for intervals
                  maxEl = std::max(maxEl, value);
                  minEl = std::min(minEl, value);
                  averageEl += value;
                  ++numEl;
          }

          for(size_t i = 0; i < numEdges; i++) {
                    // Get index, source and target descriptor
                    size_t index, v1, v2;
                    annotationsInput >> index >> v1 >> v2;

                    // Get number of intermediates
                    size_t interVerts;
                    annotationsInput >> interVerts;
                    double tempMax = -10.0;
                    double tempMin = 10.0;
                    // Get the intermediates coordinates
                    for(size_t j = 0; j < interVerts; j++) {
                            size_t sz;
                            double value;
                            annotationsInput >> sz >> value;
                            if (energy)
                              tempMax = std::max(tempMax, value);
                            else
                              tempMin = std::min(tempMin, value);

                            // Metrics for intervals
                            maxEl = std::max(maxEl, value);
                            minEl = std::min(minEl, value);
                            averageEl += value;
                            ++numEl;
                    }
                    // Add edge to m_edgeMap
                    if (energy)
                       m_edgeMap.emplace(std::make_pair(v2, v1), tempMax);
                    else
                       m_edgeMap.emplace(std::make_pair(v2, v1), tempMin);
                    //std::cout << tempMax << std::endl;
            }

          auto graph = GetVizmo().GetEnv()->GetGraphModel()->GetGraph();

          std::cout << "max: " << maxEl << ", min: " << minEl << ", average: " << averageEl / numEl << std::endl;

          // Setting the colors

          if (!energy && !detailed) {
              for(auto v = graph->begin(); v != graph->end(); ++v) {
                  if (m_vertexMap.find(v->descriptor()) != m_vertexMap.end()) {
                      double value = m_vertexMap[v->descriptor()];

                      // This is the clearance annotation with 4 intervals

                      double interval = (maxEl - minEl)/3;

                      if (value > interval*2 + minEl)
                        v->property().SetColor(Color4(0, 1, 0));
                      /*else if (value > interval*3 + minEl)
                        v->property().SetColor(Color4(0, 1, 1));*/
                      else if (value > interval + minEl)
                        v->property().SetColor(Color4(1, 1, 0));
                      else
                        v->property().SetColor(Color4(1, 0, 0));
                  }
              }

              for(auto e = graph->edges_begin(); e != graph->edges_end(); ++e) {
                auto id = std::make_pair(e->source(), e->target());
                auto id2 = std::make_pair(e->target(), e->source());
                if (m_edgeMap.find(id = id) != m_edgeMap.end() || m_edgeMap.find(id = id2) != m_edgeMap.end()) {
                      double value = m_edgeMap[id];
                      double interval = (maxEl - minEl)/3;

                      if (value > interval*2 + minEl)
                        e->property().SetColor(Color4(0, 1, 0));
                      /*else if (value > interval*2 + minEl)
                        e->property().SetColor(Color4(0, 1, 1));*/
                      else if (value > interval + minEl)
                        e->property().SetColor(Color4(1, 1, 0));
                      else
                        e->property().SetColor(Color4(1, 0, 0));
                  }
              }
          }
          if (!energy && detailed) {
              for(auto v = graph->begin(); v != graph->end(); ++v) {
                  if (m_vertexMap.find(v->descriptor()) != m_vertexMap.end()) {
                      double value = m_vertexMap[v->descriptor()];

                      // This is the clearance annotation with 8 intervals

                      double interval = (maxEl - minEl)/8;

                      if (value > interval*7 + minEl)
                        v->property().SetColor(Color4(0.7, 0, 0.3));
                      else if (value > interval*6 + minEl)
                        v->property().SetColor(Color4(1, 0, 0));
                      else if (value > interval*5 + minEl)
                        v->property().SetColor(Color4(0.7, 0.3, 0));
                      else if (value > interval*4 + minEl)
                        v->property().SetColor(Color4(0.55, 0.45, 0));
                      else if (value > interval*3 + minEl)
                        v->property().SetColor(Color4(0, 0.7, 0.3));
                      else if (value > interval*2 + minEl)
                        v->property().SetColor(Color4(0, 0.3, 0.7));
                      else if (value > interval + minEl)
                        v->property().SetColor(Color4(0, 0.1, 0.9));
                      else
                        v->property().SetColor(Color4(0.3, 0, 0.7));

                  }
              }

              for(auto e = graph->edges_begin(); e != graph->edges_end(); ++e) {
                auto id = std::make_pair(e->source(), e->target());
                auto id2 = std::make_pair(e->target(), e->source());
                if (m_edgeMap.find(id = id) != m_edgeMap.end() || m_edgeMap.find(id = id2) != m_edgeMap.end()) {
                      double value = m_edgeMap[id];
                      double interval = (maxEl - minEl)/8;

                      if (value > interval*7 + minEl)
                        e->property().SetColor(Color4(0.7, 0, 0.3));
                      else if (value > interval*6 + minEl)
                        e->property().SetColor(Color4(1, 0, 0));
                      else if (value > interval*5 + minEl)
                        e->property().SetColor(Color4(0.7, 0.3, 0));
                      else if (value > interval*4 + minEl)
                        e->property().SetColor(Color4(0.55, 0.45, 0));
                      else if (value > interval*3 + minEl)
                        e->property().SetColor(Color4(0, 0.7, 0.3));
                      else if (value > interval*2 + minEl)
                        e->property().SetColor(Color4(0, 0.3, 0.7));
                      else if (value > interval + minEl)
                        e->property().SetColor(Color4(0, 0.1, 0.9));
                      else
                        e->property().SetColor(Color4(0.3, 0, 0.7));

                  }
              }
          }
          else if (energy && !detailed) {
              for(auto v = graph->begin(); v != graph->end(); ++v) {
                  if (m_vertexMap.find(v->descriptor()) != m_vertexMap.end()) {
                      double value = m_vertexMap[v->descriptor()];

                      // This is the energy annotation with 4 intervals

                      if (value < -1e-03)
                        v->property().SetColor(Color4(1, 0, 0));
                      else if (value < -1e-05)
                        v->property().SetColor(Color4(0.6, 0.4, 0));
                      else if (value < -1e-07)
                        v->property().SetColor(Color4(0, 0.7, 0.3));
                      else
                        v->property().SetColor(Color4(0, 0, 1));
                  }
              }

              for(auto e = graph->edges_begin(); e != graph->edges_end(); ++e) {
                auto id = std::make_pair(e->source(), e->target());
                auto id2 = std::make_pair(e->target(), e->source());
                if (m_edgeMap.find(id = id) != m_edgeMap.end() || m_edgeMap.find(id = id2) != m_edgeMap.end()) {
                      double value = m_edgeMap[id];

                      if (value < -1e-03)
                        e->property().SetColor(Color4(1, 0, 0));
                      else if (value < -1e-05)
                        e->property().SetColor(Color4(0.6, 0.4, 0));
                      else if (value < -1e-07)
                        e->property().SetColor(Color4(0, 0.7, 0.3));
                      else
                        e->property().SetColor(Color4(0, 0, 1));
                  }
              }
          }
          else if (energy && detailed) {
              for(auto v = graph->begin(); v != graph->end(); ++v) {
                  if (m_vertexMap.find(v->descriptor()) != m_vertexMap.end()) {
                      double value = m_vertexMap[v->descriptor()];

                      // This is the energy annotation with 8 intervals

                      if (value < -0.1)
                        v->property().SetColor(Color4(0.7, 0, 0.3));
                      else if (value < -1e-03)
                        v->property().SetColor(Color4(1, 0, 0));
                      else if (value < -1e-05)
                        v->property().SetColor(Color4(0.7, 0.3, 0));
                      else if (value < -1e-06)
                        v->property().SetColor(Color4(0.55, 0.45, 0));
                      else if (value < -1e-07)
                        v->property().SetColor(Color4(0, 0.7, 0.3));
                      else if (value < 0)
                        v->property().SetColor(Color4(0, 0.3, 0.7));
                      else if (value < 1000)
                        v->property().SetColor(Color4(0, 0.1, 0.9));
                      else
                        v->property().SetColor(Color4(0.3, 0, 0.7));

                  }
              }

              for(auto e = graph->edges_begin(); e != graph->edges_end(); ++e) {
                auto id = std::make_pair(e->source(), e->target());
                auto id2 = std::make_pair(e->target(), e->source());
                if (m_edgeMap.find(id = id) != m_edgeMap.end() || m_edgeMap.find(id = id2) != m_edgeMap.end()) {
                  double value = m_edgeMap[id];

                      if (value < -0.1)
                        e->property().SetColor(Color4(0.7, 0, 0.3));
                      else if (value < -1e-03)
                        e->property().SetColor(Color4(1, 0, 0));
                      else if (value < -1e-05)
                        e->property().SetColor(Color4(0.7, 0.3, 0));
                      else if (value < -1e-06)
                        e->property().SetColor(Color4(0.55, 0.45, 0));
                      else if (value < -1e-07)
                        e->property().SetColor(Color4(0, 0.7, 0.3));
                      else if (value < 0)
                        e->property().SetColor(Color4(0, 0.3, 0.7));
                      else if (value < 1000)
                        e->property().SetColor(Color4(0, 0.1, 0.9));
                      else
                        e->property().SetColor(Color4(0.3, 0, 0.7));
                  }
              }


          }

    }
    else
          GetMainWindow()->statusBar()->showMessage("Loading aborted");

}

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
  ///  \arg \c .graph
	QString fn = QFileDialog::getOpenFileName(this,
      "Choose a skeleton to open", GetMainWindow()->GetLastDir(),
      "Files (*.env *.map *.query *.path *.vd *.xml *.graph)");
	typedef stapl::sequential::directed_preds_graph<
   stapl::MULTIEDGES, Point3d,vector<Point3d>> GraphType;

	if(!fn.isEmpty()) {
		QFileInfo fi(fn);
		GetMainWindow()->statusBar()->showMessage("Loading:"+fi.absoluteFilePath());
		GetMainWindow()->AlertUser("Loaded Skeleton: "+fi.fileName().toStdString());

		// Read the file
		ifstream input(fi.absoluteFilePath().toStdString());
		if(!input.good())
			GetMainWindow()->AlertUser("File Not Good");
		GraphType g;
		size_t numVert, numEdges;
		// Extract from file the number of vertices and edges
		input >> numVert >> numEdges;

		// Get vertices
		for(size_t i = 0; i < numVert; i++)	{
			Point3d p;
			size_t index;
			// Read the vertex descriptor and the co-ordinates
			input >> index >> p;
			g.add_vertex(index, p);
		}

		// Get edges
		for(size_t i = 0; i < numEdges; i++){
			size_t v1, v2;
			// Get source and target descriptor
			input >> v1 >> v2;
			size_t interVerts;
			vector<Point3d> edge;
			// Get number of intermediates
			input >> interVerts;
			// Get the intermediates coordinates
			for(size_t j = 0; j < interVerts; j++){
				Point3d p;
				input >> p;
				edge.push_back(p);
			}
			// Add edge
			g.add_edge(v1,v2,edge);
		}

		// Make a graph model
		EnvModel* env = GetVizmo().GetEnv();
		env->AddGraphModel(g);
		auto gm = env->GetGraphModel();

		// Update environment
		if(gm)
			gm->SetRenderMode(SOLID_MODE);
		RefreshEnv();

                this->AddAnnotations();
	}
	else
		GetMainWindow()->statusBar()->showMessage("Loading aborted");
}

void
EnvironmentOptions::
SaveSkeleton() {
	//save skeleton in the workspace

	QString fn = QFileDialog::getSaveFileName(this, "Chose a file name for the Skeleton",
      GetMainWindow()->GetLastDir(), "Files  (*.graph)");

	if(!fn.isEmpty()) {
		EnvModel* env = GetVizmo().GetEnv();
		QFileInfo fi(fn);
		ofstream output(fi.absoluteFilePath().toStdString());
		env->SaveGraphFile(output);
	}
	else
		GetMainWindow()->statusBar()->showMessage("Saving aborted", 2000);
}

void
EnvironmentOptions::
AddVertex()	{
	//Adds a vertex to the skeleton
	NodeEditDialog* ned = new NodeEditDialog(GetMainWindow(), "New Vertex");
	GetMainWindow()->ShowDialog(ned);
}

void
EnvironmentOptions::
AddStraightLineEdge() {
  EnvModel* env = GetVizmo().GetEnv();
  GraphModel::SkeletonGraphType*  gm = env->GetGraphModel()->GetGraph();
  if(gm->get_num_vertices() >= 2){

  // By default, just attempts straight line and does not pop up EdgeEditDialog
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  vector<CfgModel*> selNodes;

  // Filter away selected edges, but still enforce two nodes
  for(auto it = sel.begin(); it != sel.end(); it++)
    if((*it)->Name().substr(0, 4) == "Node")
      selNodes.push_back((CfgModel*)*it);

    // Enforce the selction of exactly two nodes
    if(selNodes.size() != 2) {
      GetMainWindow()->AlertUser("Please select exactly two nodes.");
      return;
    }

    // Get the source and target descriptor
    size_t v0 = selNodes[0]->GetIndex();
    size_t v1 = selNodes[1]->GetIndex();

    // Create the intermediate list with source and target as initial list
    vector<CfgModel> intermediates;
    intermediates.push_back((*gm->find_vertex(v0)).property());
    intermediates.push_back((*gm->find_vertex(v1)).property());

    // Add the edge in the graph
    auto ed = gm->add_edge(v0, v1, EdgeModel("",1, intermediates, true));
    GraphModel::SkeletonGraphType::vertex_iterator vi;
    GraphModel::SkeletonGraphType::adj_edge_iterator ei;
    gm->find_edge(ed, vi, ei);
    ei->property().Set(ed.id(), &((*gm->find_vertex(v0)).property()),
				&((*gm->find_vertex(v1)).property()));

    // Refresh the environment
    env->GetGraphModel()->Refresh();
    RefreshEnv();
    sel.clear();

    QMessageBox msgBox;
    msgBox.setText("Do you want to add intermediates?");
    msgBox.setStandardButtons(QMessageBox::Yes| QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    int ret= msgBox.exec();
    //User replies yes
    if(ret==16384){
      //add intermediates
      EdgeModel* actualEdge= &ei->property();
      string name = "Skeleton";
      EdgeEditDialog* eed = new EdgeEditDialog(name, GetMainWindow(), actualEdge);
      GetMainWindow()->ShowDialog(eed);
    }
  }
  else
    GetMainWindow()->AlertUser("Please add 2 vertices first");
}


void
EnvironmentOptions::
AddSpecificEdge(EdgeModel* _e1, EdgeModel* _e2, int _i) {
  EnvModel* env = GetVizmo().GetEnv();
  GraphModel::SkeletonGraphType*  gm = env->GetGraphModel()->GetGraph();

  //Get intermediates
  auto i1 = _e1->GetIntermediates();
  auto i2 = _e2->GetIntermediates();

  // Get the source and target descriptor
  size_t vs0 ,vs1;

	// Create the intermediate list with source and target as initial list
  vector<CfgModel> intermediates;
  if(_i < 2) {
    vs0 = _e1->GetStartCfg()->GetIndex();
    intermediates.insert(intermediates.end(), i1.begin(), i1.end());
  }
  else	{
    vs0 = _e1->GetEndCfg()->GetIndex();
    intermediates.insert(intermediates.end(), i1.rbegin(), i1.rend());
  }
  if(_i % 2 == 0) {
    vs1 = _e2->GetEndCfg()->GetIndex();
    intermediates.insert(intermediates.end(), i2.begin(), i2.end());
  }
  else  {
    vs1 = _e2->GetStartCfg()->GetIndex();
    intermediates.insert(intermediates.end(), i2.rbegin(), i2.rend());
  }

  // Add the edge in the graph
  auto ed = gm->add_edge(vs0, vs1, EdgeModel("",1, intermediates, true));
  GraphModel::SkeletonGraphType::vertex_iterator vi;
  GraphModel::SkeletonGraphType::adj_edge_iterator ei;
  gm->find_edge(ed, vi, ei);
  ei->property().Set(ed.id(), &((*gm->find_vertex(vs0)).property()),
				&((*gm->find_vertex(vs1)).property()));
}

void
EnvironmentOptions::
CollapseEdge()  {
  //Get selected items from the skeleton
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  if(sel.size() != 0) {

    typedef GraphModel::SkeletonGraphType GT;

    EnvModel* env = GetVizmo().GetEnv();
    GT* _gm = env->GetGraphModel()->GetGraph();

    typedef GraphModel::SkeletonGraphType::edge_descriptor  ED;
    vector<ED> edgesToCollapse;//vector of edge descriptors

    //Select edges to collapse
    EdgeModel* e;
    for(auto it = sel.begin(); it != sel.end(); it++) {
      string objName = (*it)->Name();
      if(objName.substr(0, 4) == "Edge") {
        e = (EdgeModel*)(*it);//store the edge in e variable
        edgesToCollapse.push_back(ED(e->GetStartCfg()->GetIndex(),
        e->GetEndCfg()->GetIndex(),e->GetID()));//add the edge descriptor to the vector
      }
    }
    //one edge selected varified
    if( edgesToCollapse.size() == 1) {
      GT::vertex_iterator vi1;
      GT::adj_edge_iterator ei1;
      _gm->find_edge(edgesToCollapse[0], vi1, ei1);
      auto v0= ei1->source();
      auto v1= ei1->target();
      auto vit0=_gm->find_vertex(v0);//get all the edges connected to the selected edge
      auto vit1=_gm->find_vertex(v1);
      /*
      NodeEditDialog* ned = new NodeEditDialog(GetMainWindow(),
          "Collapse Vertex", e, vit1);
      GetMainWindow()->ShowDialog(ned);
      */
      //iterate through the edges connected to the source
      for(auto it = vit0->begin(); it != vit0->end(); it++) {
				// if the edge is not the edge to be collapsed
				if(it->descriptor() != edgesToCollapse[0])	{
					// Change the end vertex of the edges adjacent to source as target
					vector<CfgModel> intermediates = it->property().GetIntermediates();
					// Replace the current source from intermediates as the target vertex
					intermediates.erase(intermediates.begin());
					intermediates.insert(intermediates.begin(), vit1->property());
        	_gm->add_edge(v1, it->target(),  EdgeModel("",1, intermediates, true));
				}
			}
			// iterate through the edges where source is the target vertex
			for(auto pit = vit0->predecessors().begin(); pit != vit0->predecessors().end(); ++pit)	{
				GT::vertex_iterator vi;
      	GT::adj_edge_iterator it;
				_gm->find_edge(ED(*pit,v0), vi, it);
				// if the edge is not the edge to be collapsed
				if(it->descriptor() != edgesToCollapse[0])	{
					// Change the end vertex of the edges adjacent to source as target
					vector<CfgModel> intermediates = it->property().GetIntermediates();
					// Replace the current target from intermediates as the target vertex
					intermediates.pop_back();
					intermediates.push_back(vit1->property());
        	_gm->add_edge(it->source(), v1,  EdgeModel("",1, intermediates, true));
				}
			}
			// Delete the source vertex and hence its adjacent edges
			_gm->delete_vertex(v0);

    }
    else
      GetMainWindow()->AlertUser("Please select an Edge to collapse");


    env->GetGraphModel()->Refresh();
    RefreshEnv();
    sel.clear();
  }
}
void
EnvironmentOptions::
MergeEdges() {
  EnvModel* env = GetVizmo().GetEnv();
  GraphModel::SkeletonGraphType*  gm = env->GetGraphModel()->GetGraph();
  if(gm->get_num_edges() >= 2){
    typedef GraphModel::SkeletonGraphType GT;

    //Get selected items from the skeleton
    vector<Model*>& sel = GetVizmo().GetSelectedModels();
    EnvModel* env = GetVizmo().GetEnv();
    GT* _gm = env->GetGraphModel()->GetGraph();
    if(sel.size() != 0) {
      //bool selectionValid = false;
      typedef GraphModel::SkeletonGraphType::edge_descriptor  ED;
      vector<ED> edgesToMerge;//vector of edge descriptors

      //Select edges to merge
      for(auto it = sel.begin(); it != sel.end(); it++) {
        string objName = (*it)->Name();
        if(objName.substr(0, 4) == "Edge") {
          //selectionValid = true;//if its an edge it is valid selection
          EdgeModel* e = (EdgeModel*)(*it);//store the edge in e variable
          edgesToMerge.push_back(ED(e->GetStartCfg()->GetIndex(),
          e->GetEndCfg()->GetIndex(),e->GetID()));//add the edge descriptor to the vector
        }
      }
      //two edges selected varified
      if( edgesToMerge.size() == 2 ) {
        GT::vertex_iterator vi1, vi2;
        GT::adj_edge_iterator ei1, ei2;
        _gm->find_edge(edgesToMerge[0], vi1, ei1);
        _gm->find_edge(edgesToMerge[1], vi2, ei2);
        EdgeModel* e1= &(ei1->property());
        EdgeModel* e2= &(ei2->property());
        bool _connect=false;
        size_t _vertex;

        if(ei1->source()==ei2->target()){
          _connect = true;
          _vertex = ei1->source();
          AddSpecificEdge(e1, e2, 3);
        }
        else if(ei1->target()==ei2->target()){
          _connect=true;
          _vertex = ei1->target();
          AddSpecificEdge(e1, e2, 1);
        }
        else if(ei1->source()==ei2->source()){
          _connect=true;
          _vertex = ei1->source();
          AddSpecificEdge(e1, e2, 2);
        }
        else if(ei1->target()==ei2->source()){
          _connect=true;
          _vertex = ei1->target();
          AddSpecificEdge(e1, e2, 0);
        }
        else
          GetMainWindow()->AlertUser("Two edges need to be adjacent to be merged");

        for(auto it = edgesToMerge.begin(); _connect && it != edgesToMerge.end(); it++) {
      	  _gm->delete_edge(*it);
        }

        if(_gm->get_in_degree(_vertex) + _gm->get_out_degree(_vertex)== 0)
          _gm->delete_vertex(_vertex);
      }
      else
        GetMainWindow()->AlertUser("Please select two connected Edges to merge");


      env->GetGraphModel()->Refresh();
      RefreshEnv();
      sel.clear();
    }
    else
    GetMainWindow()->AlertUser("You need to add 2 edges before merging");
  }
  else
    GetMainWindow()->AlertUser("Please select 2 edges to merge");
}

void
EnvironmentOptions::
DeleteSelectedItems() {
  //Deletes selected items from the skeleton
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  if(sel.size() != 0) {
	  EnvModel* env = GetVizmo().GetEnv();
  	GraphModel::SkeletonGraphType* _gm = env->GetGraphModel()->GetGraph();
  	if(_gm) {
	  	bool selectionValid = false;
			typedef GraphModel::SkeletonGraphType::edge_descriptor  ED;
			typedef GraphModel::SkeletonGraphType::vertex_descriptor  VD;
  		vector<VD> nodesToDelete;
  		vector<ED> edgesToDelete;

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
      		edgesToDelete.push_back(ED(e->GetStartCfg()->GetIndex(),
            e->GetEndCfg()->GetIndex(),e->GetID()));
    		}
  		}

  		if(selectionValid == false)
    		GetMainWindow()->AlertUser("Please select a group of nodes and edges to"
        	" remove.");
  		else {
    		//Remove selected vertices
   			for(auto it = nodesToDelete.begin(); it != nodesToDelete.end(); it++)
		   		_gm->delete_vertex(*it);

    		//Remove selected edges
    		for(auto it = edgesToDelete.begin(); it != edgesToDelete.end(); it++) {
       		_gm->delete_edge(*it);
    		}

		env->GetGraphModel()->Refresh();
		RefreshEnv();
    sel.clear();
  }
  }
  else
    GetMainWindow()->AlertUser("No graph items to delete");

  }
  else
    GetMainWindow()->AlertUser("Please select an item to delete");

}

void
EnvironmentOptions::
ChangeColor() {
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  if(sel.size() != 0) {
    QColor color = QColorDialog::getColor(Qt::white,this,"color dialog");
    if(color.isValid()) {
      float r = color.red()/255.0;
      float g = color.green()/255.0;
      float b = color.blue()/255.0;
      for(auto it = sel.begin(); it != sel.end(); it++) {
        (*it)->SetColor(Color4(r,g,b,1));
      }
    }
    EnvModel* env = GetVizmo().GetEnv();
    GraphModel::SkeletonGraphType* _gm = env->GetGraphModel()->GetGraph();
    if(_gm)
      GetVizmo().GetEnv()->GetGraphModel()->Refresh();
  }
  else
    GetMainWindow()->AlertUser("Please select one or more items");
}

/*----------------------- Environment Editing --------------------------------*/

void
EnvironmentOptions::
LoadEllipses()  {
  QString fn = QFileDialog::getOpenFileName(this, "Choose a file to load Ellipses",
      GetMainWindow()->GetLastDir(), "Files (*.e)");

  if(!fn.isEmpty()) {
    //save last directory
    QFileInfo fi(fn);
    GetMainWindow()->statusBar()->showMessage("Loading:"+fi.absoluteFilePath());
    GetMainWindow()->AlertUser("Loaded file: "+fi.fileName().toStdString());

    //Read the file
    ifstream input(fi.absoluteFilePath().toStdString());
    if(!input.good())
        GetMainWindow()->AlertUser("File Not Good");

    int d, size;      //dimension for the file
    input >> d >> size;

    vector<EllipsoidModel*> ellipses;
    for (int i = 0; i < size; i++)  {
      char type, s = 's', e = 'e';              //the type is 's' for sphere and 'e' for ellipse
      input >> type;
      Point3d p;
      input >> p;
      vector<double> center;
      for(int j = 0; j < 3; j++)  {
        center.push_back(p[j]);
      }
      EllipsoidModel* elli;

      if (type == s) {
        double r;
        input >> r;
        vector<double> axes(d,r);
        elli = new EllipsoidModel(center, axes);
      }
      else if (type == e) {
        vector<double> axes;
        for(int j = 0; j < d; j++)  {
          double temp;
          input >> temp;
          axes.push_back(temp);
        }

				double alpha, beta, gamma; // Not using the euler angle >> operator as it reads in different order than <<
				input >> alpha >> beta >> gamma;
				EulerAngle eAngle(fmod(degToRad(alpha), TWOPI), fmod(degToRad(beta), TWOPI), fmod(degToRad(gamma), TWOPI));
        elli = new EllipsoidModel(center, axes, eAngle);
      }
      ellipses.push_back(elli);
    }
    GetVizmo().GetEnv()->SetEllipses(ellipses);
    RefreshEnv();
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


