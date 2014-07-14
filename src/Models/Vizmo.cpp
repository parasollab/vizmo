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
#include "MotionPlanning/VizmoTraits.h"
#include "PHANToM/Manager.h"
#include "Utilities/PickBox.h"

////////////////////////////////////////////////////////////////////////////////
//Define Vizmo singleton
////////////////////////////////////////////////////////////////////////////////
Vizmo vizmo;
Vizmo& GetVizmo() {return vizmo;}

////////////////////////////////////////////////////////////////////////////////
//Define Motion Planning singleton - eventually move to MP directory
////////////////////////////////////////////////////////////////////////////////
VizmoProblem* vizmoProblem;
VizmoProblem*& GetVizmoProblem() {return vizmoProblem;}

////////////////////////////////////////////////////////////////////////////////
// Vizmo
////////////////////////////////////////////////////////////////////////////////
Vizmo::
Vizmo() :
  m_envModel(NULL),
  m_robotModel(NULL),
  m_manager(NULL),
  m_mapModel(NULL),
  m_queryModel(NULL),
  m_pathModel(NULL),
  m_debugModel(NULL) {
  }

Vizmo::
~Vizmo() {
  Clean();
}

bool
Vizmo::
InitModels() {
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

    //try to initialize PHANToM
    m_manager = new Haptics::Manager();

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
Vizmo::
InitPMPL() {
  //initialize PMPL structures for collision detection
  VizmoProblem*& problem = GetVizmoProblem();
  problem = new VizmoProblem();
  problem->SetEnvironment(m_envModel->GetEnvironment());

  //add PQP_SOLID collision detection validity
  CollisionDetectionMethod* cd = new PQPSolid();
  VizmoProblem::ValidityCheckerPointer vc(new CollisionDetectionValidity<VizmoTraits>(cd));
  problem->AddValidityChecker(vc, "PQP_SOLID");

  //add uniform sampler
  VizmoProblem::SamplerPointer sp(new UniformRandomSampler<VizmoTraits>("PQP_SOLID"));
  problem->AddSampler(sp, "uniform");

  //add distance metric
  VizmoProblem::DistanceMetricPointer dm(new EuclideanDistance<VizmoTraits>());
  problem->AddDistanceMetric(dm, "euclidean");

  //add straight line local planner
  VizmoProblem::LocalPlannerPointer lp(new StraightLine<VizmoTraits>("PQP_SOLID", true));
  problem->AddLocalPlanner(lp, "sl");

  //add neighborhood finder
  VizmoProblem::NeighborhoodFinderPointer nfp(new BruteForceNF<VizmoTraits>("euclidean", false, 10));
  problem->AddNeighborhoodFinder(nfp, "BFNF");

  //add connector
  VizmoProblem::ConnectorPointer cp(new NeighborhoodConnector<VizmoTraits>("BFNF", "sl"));
  problem->AddConnector(cp, "Neighborhood Connector");

  //add num nodes metric for evaluator
  VizmoProblem::MetricPointer mp(new NumNodesMetric<VizmoTraits>());
  problem->AddMetric(mp, "NumNodes");

  //add evaluator: if a query file is loaded, use Query evaluator. Otherwise,
  //use nodes eval
  VizmoProblem::MapEvaluatorPointer pme(new PrintMapEvaluation<VizmoTraits>("debugmap"));
  problem->AddMapEvaluator(pme, "PrintMap");

  //add NumNodes eval
  VizmoProblem::MapEvaluatorPointer mep(new ConditionalEvaluator<VizmoTraits>(ConditionalEvaluator<VizmoTraits>::GT, "NumNodes", 10000));
  problem->AddMapEvaluator(mep, "NodesEval");

  //set up query evaluators
  if(m_queryModel) {
    VizmoProblem::MapEvaluatorPointer mep(new Query<VizmoTraits>(m_queryFilename, vector<string>(1, "Neighborhood Connector")));
    problem->AddMapEvaluator(mep, "Query");

    //setup debugging evaluator
    vector<string> evals;
    evals.push_back("PrintMap");
    evals.push_back("Query");
    VizmoProblem::MapEvaluatorPointer ce(new ComposeEvaluator<VizmoTraits>(ComposeEvaluator<VizmoTraits>::AND, evals));
    problem->AddMapEvaluator(ce, "DebugQuery");

    //set up bounded query evaluator
    evals.clear();
    evals.push_back("NodesEval");
    evals.push_back("Query");
    VizmoProblem::MapEvaluatorPointer bqe(new ComposeEvaluator<VizmoTraits>(ComposeEvaluator<VizmoTraits>::OR, evals));
    problem->AddMapEvaluator(bqe, "BoundedQuery");
  }

  //add region strategy
  VizmoProblem::MPStrategyPointer rs(new RegionStrategy<VizmoTraits>());
  problem->AddMPStrategy(rs, "regions");


  //set the MPProblem pointer and build CD structures
  problem->SetMPProblem();
  problem->BuildCDStructures();
}

void
Vizmo::
Clean() {
  delete m_envModel;
  delete m_robotModel;
  delete m_manager;
  delete m_mapModel;
  delete m_queryModel;
  delete m_pathModel;
  delete m_debugModel;
  m_envModel = NULL;
  m_robotModel = NULL;
  m_manager = NULL;
  m_mapModel = NULL;
  m_queryModel = NULL;
  m_pathModel = NULL;
  m_debugModel = NULL;

  m_loadedModels.clear();
  m_selectedModels.clear();

  delete GetVizmoProblem();
  GetVizmoProblem() = NULL;
}

//Display OpenGL Scene
void
Vizmo::
Draw() {
  typedef vector<Model*>::iterator MIT;
  for(MIT mit = m_loadedModels.begin(); mit!=m_loadedModels.end(); ++mit)
    (*mit)->DrawRender();

  glColor3f(1,1,0); //Selections are yellow, so set the color once now
  for(MIT mit = m_selectedModels.begin(); mit != m_selectedModels.end(); ++mit)
    (*mit)->DrawSelected();
}

//Select Objects in OpenGL Scene
void
Vizmo::
Select(const Box& _box) {
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
    (*mit)->DrawSelect();
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
Vizmo::
CollisionCheck(CfgModel& _c) {
  if(m_envModel) {
    VizmoProblem::ValidityCheckerPointer vc = GetVizmoProblem()->GetValidityChecker("PQP_SOLID");
    bool b = vc->IsValid(_c, "Vizmo");
    _c.SetValidity(b);
    return b;
  }
  cerr << "Warning::Collision checking when there is no environment. Returning false." << endl;
  return false;
}

pair<bool, double>
Vizmo::
VisibilityCheck(CfgModel& _c1, CfgModel& _c2) {
  if(m_envModel) {
    Environment* env = GetVizmoProblem()->GetEnvironment();
    VizmoProblem::LocalPlannerPointer lp = GetVizmoProblem()->GetLocalPlanner("sl");
    LPOutput<VizmoTraits> lpout;
    if(lp->IsConnected(_c1, _c2, &lpout, env->GetPositionRes(), env->GetOrientationRes()))
      return make_pair(true, lpout.m_edge.first.GetWeight());
    else return make_pair(false, EdgeModel::MaxWeight().Weight());
  }
  cerr << "Warning::Visibility checking when there is no environment. Returning false" << endl;
  return make_pair(false, EdgeModel::MaxWeight().Weight());
}

void
Vizmo::
PlaceRobot() {
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
Vizmo::
SearchSelectedItems(int _hit, void* _buffer, bool _all) {
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
Vizmo::
StartClock(const string& _c) {
  m_timers[_c].first.start();
  //GetVizmoProblem()->GetStatClass()->StartClock(_c);
}

void
Vizmo::
StopClock(const string& _c) {
  if(m_timers.count(_c))
    m_timers[_c].second = m_timers[_c].first.elapsed()/1000.;
  else
    m_timers[_c].second = 0;
  //GetVizmoProblem()->GetStatClass()->StopClock(_c);
}

void
Vizmo::
PrintClock(const string& _c, ostream& _os) {
  _os << _c << ": " << m_timers[_c].second << " sec" << endl;
  //GetVizmoProblem()->GetStatClass()->PrintClock(_c, _os);
}

void
Vizmo::
SetPMPLMap() {
  if(GetVizmo().IsRoadMapLoaded()) {
    vector<Model*>::iterator mit = find(m_loadedModels.begin(), m_loadedModels.end(), m_mapModel);
    m_loadedModels.erase(mit);
    delete m_mapModel;
  }
  m_mapModel = new MapModel<CfgModel, EdgeModel>(GetVizmoProblem()->GetRoadmap()->GetGraph());
  m_loadedModels.push_back(m_mapModel);
}

void
Vizmo::
Solve(const string& _strategy) {
  SRand(m_seed);
  VizmoProblem::MPStrategyPointer mps = GetVizmoProblem()->GetMPStrategy(_strategy);

  ostringstream oss;
  oss << GetEnv()->GetModelDataDir() << "/RegionStrategy." << GetSeed() << ".vd";
  // Initialize Vizmo Debug
  VDInit(oss.str());

  // Add this so that regions made prior to solving are added
  AddInitialRegions();

  mps->operator()();

  // Close the debug file
  VDClose();

  GetVizmo().GetMap()->RefreshMap();
}
