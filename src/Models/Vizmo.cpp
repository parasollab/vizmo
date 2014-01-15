#include "Vizmo.h"

#include <iostream>
using namespace std;

#include "CfgModel.h"
#include "DebugModel.h"
#include "EnvModel.h"
#include "Model.h"
#include "PathModel.h"
#include "QueryModel.h"
#include "RobotModel.h"
#include "Utilities/PickBox.h"

////////////////////////////////////////////////////////////////////////////////
//Define Vizmo singleton
////////////////////////////////////////////////////////////////////////////////
Vizmo vizmo;
Vizmo& GetVizmo(){return vizmo;}

////////////////////////////////////////////////////////////////////////////////
// Vizmo
////////////////////////////////////////////////////////////////////////////////
Vizmo::Vizmo() :
  m_envModel(NULL),
  m_robotModel(NULL),
  m_mapModel(NULL),
  m_queryModel(NULL),
  m_pathModel(NULL),
  m_debugModel(NULL),
  m_problem(NULL) {
  }

Vizmo::~Vizmo() {
  Clean();
}

bool
Vizmo::InitModels() {
  Clean();

  try {
    //Create environment first
    if(m_envFilename.empty())
      throw ParseException(WHERE, "Vizmo must load an environment file.");

    m_envModel = new EnvModel(m_envFilename);
    m_loadedModels.push_back(m_envModel);

    //create robot
    m_robotModel = new RobotModel(m_envModel);
    m_loadedModels.push_back(m_robotModel);
    cout << "Load Environment File : "<< m_envFilename << endl;

    //Create map
    if(!m_mapFilename.empty()) {
      m_mapModel = new MapModel<CfgModel, EdgeModel>(m_mapFilename);
      m_loadedModels.push_back(m_mapModel);
      cout << "Load Map File : " << m_mapFilename << endl;
    }

    //Create qry
    if(!m_queryFilename.empty()) {
      m_queryModel = new QueryModel(m_queryFilename);
      m_loadedModels.push_back(m_queryModel);
      cout << "Load Query File : " << m_queryFilename << endl;
    }

    //Create path
    if(!m_pathFilename.empty()) {
      m_pathModel = new PathModel(m_pathFilename);
      m_loadedModels.push_back(m_pathModel);
      cout << "Load Path File : " << m_pathFilename << endl;
    }

    //Create debug
    if(!m_debugFilename.empty()){
      m_debugModel = new DebugModel(m_debugFilename);
      m_loadedModels.push_back(m_debugModel);
      cout << "Load Debug File : " << m_debugFilename << endl;
    }
  }
  catch(PMPLException& _e) {
    cerr << _e.what() << endl;
    cerr << "Cleaning vizmo objects." << endl;
    Clean();
    return false;
  }
  catch(exception& _e) {
    cerr << "Error::" << _e.what() << "\nExiting." << endl;
    exit(1);
  }

  //Put robot in start cfg, if availiable
  PlaceRobot();

  InitPMPL();

  return true;
}

void
Vizmo::InitPMPL() {
  //initialize PMPL structures for collision detection
  m_problem = new VizmoProblem();
  m_problem->SetEnvironment(m_envModel->GetEnvironment());

  //add rapid collision detection validity
  CollisionDetectionMethod* cd = new Rapid();
  VizmoProblem::ValidityCheckerPointer vc(new CollisionDetectionValidity<VizmoTraits>(cd));
  m_problem->AddValidityChecker(vc, "rapid");

  //add uniform sampler
  VizmoProblem::SamplerPointer sp(new UniformRandomSampler<VizmoTraits>("rapid"));
  m_problem->AddSampler(sp, "uniform");

  //add distance metric
  VizmoProblem::DistanceMetricPointer dm(new EuclideanDistance<VizmoTraits>());
  m_problem->AddDistanceMetric(dm, "euclidean");

  //add straight line local planner
  VizmoProblem::LocalPlannerPointer lp(new StraightLine<VizmoTraits>("rapid", true));
  m_problem->AddLocalPlanner(lp, "sl");

  //add neighborhood finder
  VizmoProblem::NeighborhoodFinderPointer nfp(new BruteForceNF<VizmoTraits>("euclidean", false, 5));
  m_problem->AddNeighborhoodFinder(nfp, "BFNF");

  //add connector
  VizmoProblem::ConnectorPointer cp(new NeighborhoodConnector<VizmoTraits>("BFNF", "sl"));
  m_problem->AddConnector(cp, "Neighborhood Connector");

  //add num nodes metric for evaluator
  VizmoProblem::MetricPointer mp(new NumNodesMetric<VizmoTraits>());
  m_problem->AddMetric(mp, "NumNodes");

  //add evaluator: if a query file is loaded, use Query evaluator. Otherwise,
  //use nodes eval
  VizmoProblem::MapEvaluatorPointer pme(new PrintMapEvaluation<VizmoTraits>("debugmap"));
  m_problem->AddMapEvaluator(pme, "PrintMap");
  if(m_queryModel) {
    VizmoProblem::MapEvaluatorPointer mep(new Query<VizmoTraits>(m_queryFilename, vector<string>(1, "Neighborhood Connector")));
    mep->SetDebug(true);
    m_problem->AddMapEvaluator(mep, "Query");

    //setup debugging evaluator
    vector<string> evals;
    evals.push_back("PrintMap");
    evals.push_back("Query");
    VizmoProblem::MapEvaluatorPointer ce(new ComposeEvaluator<VizmoTraits>(ComposeEvaluator<VizmoTraits>::AND, evals));
    m_problem->AddMapEvaluator(ce, "DebugQuery");
  }
  else {
    VizmoProblem::MapEvaluatorPointer mep(new ConditionalEvaluator<VizmoTraits>(ConditionalEvaluator<VizmoTraits>::GT, "NumNodes", 100));
    m_problem->AddMapEvaluator(mep, "NodesEval");
  }

  //add region strategy
  VizmoProblem::MPStrategyPointer rs(new RegionStrategy<Vizmo>(this));
  m_problem->AddMPStrategy(rs, "regions");

  //set the MPProblem pointer and build CD structures
  m_problem->SetMPProblem();
  m_problem->BuildCDStructures();
}

void
Vizmo::Clean() {
  delete m_envModel;
  delete m_robotModel;
  delete m_mapModel;
  delete m_queryModel;
  delete m_pathModel;
  delete m_debugModel;
  delete m_problem;
  m_envModel = NULL;
  m_robotModel = NULL;
  m_mapModel = NULL;
  m_queryModel = NULL;
  m_pathModel = NULL;
  m_debugModel = NULL;
  m_problem = NULL;

  m_loadedModels.clear();
  m_selectedModels.clear();
}

//Display OpenGL Scene
void
Vizmo::Draw() {
  typedef vector<Model*>::iterator MIT;
  for(MIT mit = m_loadedModels.begin(); mit!=m_loadedModels.end(); ++mit)
    (*mit)->Draw();

  glColor3f(1,1,0); //Selections are yellow, so set the color once now
  for(MIT mit = m_selectedModels.begin(); mit != m_selectedModels.end(); ++mit)
    (*mit)->DrawSelect();
}

//Select Objects in OpenGL Scene
void
Vizmo::Select(const Box& _box) {
  // prepare for selection mode
  GLuint hitBuffer[1024];
  glSelectBuffer(1024, hitBuffer);
  glRenderMode(GL_SELECT);

  // get view port
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  // initialize stack
  glInitNames();

  // change view volume
  glMatrixMode(GL_PROJECTION);

  double pm[16]; //current projection matrix
  glGetDoublev(GL_PROJECTION_MATRIX, pm);

  glPushMatrix();
  glLoadIdentity();

  double x = (_box.m_left + _box.m_right)/2;
  double y = (_box.m_top + _box.m_bottom)/2;
  double w = fabs(_box.m_right - _box.m_left); if(w<5) w=5;
  double h = fabs(_box.m_top - _box.m_bottom); if(h<5) h=5;

  gluPickMatrix(x, y, w, h, viewport);
  glMultMatrixd(pm); //apply current proj matrix

  //draw
  glMatrixMode(GL_MODELVIEW);
  typedef vector<Model*>::iterator MIT;
  for(MIT mit = m_loadedModels.begin(); mit != m_loadedModels.end(); ++mit) {
    glPushName(mit-m_loadedModels.begin());
    (*mit)->Draw();
    glPopName();
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  GLuint hits = glRenderMode(GL_RENDER);

  // unselect everything first
  m_selectedModels.clear();

  if(hits > 0)
    SearchSelectedItems(hits, hitBuffer, (w*h) > 100);
}

//////////////////////////////////////////////////
// Collision Detection related functions
//////////////////////////////////////////////////

bool
Vizmo::CollisionCheck(CfgModel& _c) {
  if(m_envModel) {
    VizmoProblem::ValidityCheckerPointer vc = m_problem->GetValidityChecker("rapid");
    bool b = vc->IsValid(_c, "Vizmo");
    _c.SetValidity(b);
    return b;
  }
  cerr << "Warning::Collision checking when there is no environment. Returning false." << endl;
  return false;
}

bool
Vizmo::VisibilityCheck(CfgModel& _c1, CfgModel& _c2) {
  if(m_envModel) {
    Environment* env = m_problem->GetEnvironment();
    VizmoProblem::LocalPlannerPointer lp = m_problem->GetLocalPlanner("sl");
    LPOutput<VizmoTraits> lpout;
    return lp->IsConnected(_c1, _c2, &lpout, env->GetPositionRes(), env->GetOrientationRes());
  }
  cerr << "Warning::Visibility checking when there is no environment. Returning false." << endl;
  return false;
}

void
Vizmo::PlaceRobot() {
  if(m_robotModel){
    vector<double> cfg;
    if(m_queryModel)
      cfg = m_queryModel->GetQueryCfg(0).GetData();
      //cfg = m_queryModel->GetStartGoal(0);
    else if(m_pathModel)
      cfg = m_pathModel->GetConfiguration(0).GetData();
    else
      cfg = vector<double>(CfgModel::DOF());
    if(m_debugModel || (m_mapModel && !(m_pathModel || m_queryModel)))
      m_robotModel->SetRenderMode(INVISIBLE_MODE);

    if(!cfg.empty()) {
      //copy initial cfg. to RobotModel
      m_robotModel->Configure(cfg);
      m_robotModel->SetInitialCfg(cfg);
    }
  }
}

//Parse the Hit Buffer. Store selected obj into m_selectedModels.
//hit is the number of hit by this selection
//buffer is the hit buffer
//if all, all obj select will be put into m_selectedItems,
//otherwise only the closest will be selected.
void
Vizmo::SearchSelectedItems(int _hit, void* _buffer, bool _all) {
  //init local data
  GLuint* ptr = (GLuint*)_buffer;
  unsigned int* selName = NULL;

  //input error
  if(!ptr)
    return;

  double z1; //near z for hit object
  double closeDistance = 1e3;

  for(int i=0; i<_hit; i++) {
    unsigned int* curName=NULL;
    GLuint nameSize = *ptr; ptr++;
    z1 = ((double)*ptr)/0x7fffffff; ptr++; //near z
    ptr++; //far z, we don't use this info

    curName = new unsigned int[nameSize];
    if(!curName)
      return;

    for(unsigned int iN=0; iN<nameSize; ++iN){
      curName[iN] = *ptr;
      ptr++;
    }

    if(!_all) {//not all
      if( z1<closeDistance ) {
        closeDistance = z1;     // set current nearset to z1
        delete [] selName;      //free preallocated mem
        if((selName=new unsigned int[nameSize])==NULL) return;
        memcpy(selName,curName,sizeof(unsigned int)*nameSize);
      }
    }
    else{ //select all
      if(curName[0] <= m_loadedModels.size()) {
        Model* selectModel = m_loadedModels[curName[0]];
        selectModel->Select(&curName[1], m_selectedModels);
      }
    }

    delete [] curName;  //free preallocated mem
  }

  //only the closest
  if(!_all) {
    // analyze selected item
    if(selName && selName[0] <= m_loadedModels.size()) {
      Model* selectModel = m_loadedModels[selName[0]];
      selectModel->Select(&selName[1], m_selectedModels);
    }
  }
  delete [] selName;
}

void
Vizmo::Solve(const string& _strategy) {
  VizmoProblem::MPStrategyPointer mps = m_problem->GetMPStrategy(_strategy);
  mps->operator()();
}
