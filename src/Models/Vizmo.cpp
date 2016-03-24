#include "Vizmo.h"

#include <iostream>
using namespace std;

#include "AvatarModel.h"
#include "CfgModel.h"
#include "DebugModel.h"
#include "EnvModel.h"
#include "Model.h"
#include "PathModel.h"
#include "QueryModel.h"
#include "ActiveMultiBodyModel.h"

#include "GUI/MainWindow.h"

#ifdef PMPCfg
#include "MotionPlanning/VizmoTraits.h"
#elif defined(PMPState)
#include "MotionPlanning/VizmoStateTraits.h"
#endif

#include "PHANToM/Manager.h"
#include "SpaceMouse/SpaceMouseManager.h"

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
  m_phantomManager(NULL),
  m_spaceMouseManager(NULL),
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
    if(!m_xmlFilename.empty()) {
      InitPMPL(m_xmlFilename);
      m_envModel = new EnvModel(GetVizmoProblem()->GetEnvironment());
      m_queryFilename = MPProblemBase::GetPath(m_queryFilename);
    }
    else {
      //Create environment first
      if(m_envFilename.empty())
        throw ParseException(WHERE, "Vizmo must load an environment file.");

      m_envModel = new EnvModel(m_envFilename);
    }
    m_loadedModels.push_back(m_envModel);


    //try to initialize PHANToM
    m_phantomManager = new Haptics::Manager();

    //try to initialize space mouse
    m_spaceMouseManager = new SpaceMouseManager();
    m_spaceMouseManager->Enable();
    m_spaceMouseManager->EnableCamera();

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
  PlaceRobots();

  if(m_xmlFilename.empty())
    InitPMPL();

  return true;
}

void
Vizmo::
InitPMPL(string _xmlFilename) {
  VizmoProblem*& problem = GetVizmoProblem();
  problem = new VizmoProblem(_xmlFilename);
}

void
Vizmo::
InitPMPL() {
  VizmoProblem*& problem = GetVizmoProblem();
  problem = new VizmoProblem();
  problem->SetEnvironment(m_envModel->GetEnvironment());

  //add PQP_SOLID collision detection validity
  CollisionDetectionMethod* cd = new PQPSolid();
  VizmoProblem::ValidityCheckerPointer vc(
      new CollisionDetectionValidity<VizmoTraits>(cd));
  problem->AddValidityChecker(vc, "PQP_SOLID");

  //add uniform sampler
  VizmoProblem::SamplerPointer sp(
      new UniformRandomSampler<VizmoTraits>("PQP_SOLID"));
  problem->AddSampler(sp, "Uniform");

  //add distance metric
  VizmoProblem::DistanceMetricPointer dm(new EuclideanDistance<VizmoTraits>());
  problem->AddDistanceMetric(dm, "euclidean");

  //add straight line local planner
  VizmoProblem::LocalPlannerPointer lp(
      new StraightLine<VizmoTraits>("PQP_SOLID", true));
  problem->AddLocalPlanner(lp, "sl");

  //add neighborhood finder
  VizmoProblem::NeighborhoodFinderPointer nfp(
      new BruteForceNF<VizmoTraits>("euclidean", false, 10));
  problem->AddNeighborhoodFinder(nfp, "BFNF");

  //add connector
  VizmoProblem::ConnectorPointer cp(
      new NeighborhoodConnector<VizmoTraits>("BFNF", "sl"));
  problem->AddConnector(cp, "kClosest");

  //add num nodes metric for evaluator
  VizmoProblem::MetricPointer mp(new NumNodesMetric<VizmoTraits>());
  problem->AddMetric(mp, "NumNodes");

  //add evaluator: if a query file is loaded, use Query evaluator. Otherwise,
  //use nodes eval
  VizmoProblem::MapEvaluatorPointer pme(
      new PrintMapEvaluation<VizmoTraits>("debugmap"));
  problem->AddMapEvaluator(pme, "PrintMap");

  //add NumNodes eval
  VizmoProblem::MapEvaluatorPointer mep(
      new ConditionalEvaluator<VizmoTraits>(
        ConditionalEvaluator<VizmoTraits>::GT, "NumNodes", 7500));
  problem->AddMapEvaluator(mep, "NodesEval");

  //set up query evaluators
  if(m_queryModel) {
    //setup standard query evaluator
    /*Query<VizmoTraits>* query = new Query<VizmoTraits>(m_queryFilename,
        vector<string>(1, "kClosest"));
    VizmoProblem::MapEvaluatorPointer mep(query);
    problem->AddMapEvaluator(mep, "Query");*/

    //setup debugging evaluator
    vector<string> evals;
    evals.push_back("PrintMap");
    evals.push_back("Query");
    VizmoProblem::MapEvaluatorPointer ce(
        new ComposeEvaluator<VizmoTraits>(
          ComposeEvaluator<VizmoTraits>::AND, evals));
    problem->AddMapEvaluator(ce, "DebugQuery");

    //set up bounded query evaluator
    evals.clear();
    evals.push_back("NodesEval");
    evals.push_back("Query");
    VizmoProblem::MapEvaluatorPointer bqe(new
        ComposeEvaluator<VizmoTraits>(ComposeEvaluator<VizmoTraits>::OR, evals));
    problem->AddMapEvaluator(bqe, "BoundedQuery");

    //add basic extender for I-RRT
    /*VizmoProblem::ExtenderPointer bero(new BasicExtender<VizmoTraits>(
          "euclidean", "PQP_SOLID", 10., true));
    problem->AddExtender(bero, "BERO");*/

    //add basic extender for avoid region checking
    /*VizmoProblem::ExtenderPointer arbero(new BasicExtender<VizmoTraits>(
          "euclidean", "RegionValidity", 10., true));
    problem->AddExtender(arbero, "RegionBERO");*/

    //add I-RRT strategy
    /*VizmoProblem::MPStrategyPointer irrt(
        new IRRTStrategy<VizmoTraits>(query->GetQuery().front(),
          query->GetQuery().back()));
    problem->AddMPStrategy(irrt, "IRRT");

    VizmoProblem::MPStrategyPointer rr(
        new RegionRRT<VizmoTraits>(query->GetQuery().front(),
          query->GetQuery().back()));
    problem->AddMPStrategy(rr, "RegionRRT");*/

  }
  /*
  //add region strategy
  VizmoProblem::MPStrategyPointer rs(new RegionStrategy<VizmoTraits>());
  problem->AddMPStrategy(rs, "RegionStrategy");

  //add path strategy
  VizmoProblem::MPStrategyPointer ps(new PathStrategy<VizmoTraits>());
  problem->AddMPStrategy(ps, "PathStrategy");

  //add spark region strategy
  VizmoProblem::MPStrategyPointer sr(new SparkPRM<VizmoTraits, SparkRegion>());
  problem->AddMPStrategy(sr, "SparkRegion");

  //add Cfg oracle
  VizmoProblem::MPStrategyPointer co(new CfgOracle<VizmoTraits>());
  problem->AddMPStrategy(co, "CfgOracle");

  //add region oracle
  VizmoProblem::MPStrategyPointer ro(new RegionOracle<VizmoTraits>());
  problem->AddMPStrategy(ro, "RegionOracle");

  //add path oracle
  VizmoProblem::MPStrategyPointer po(new PathOracle<VizmoTraits>());
  problem->AddMPStrategy(po, "PathOracle");
  */
  //avoid-region validity checker
  VizmoProblem::ValidityCheckerPointer arv(
      new AvoidRegionValidity<VizmoTraits>());
  problem->AddValidityChecker(arv, "AvoidRegionValidity");

  //avoid-region (only) local planner
  VizmoProblem::LocalPlannerPointer arsl(
      new StraightLine<VizmoTraits>("AvoidRegionValidity", true));
  problem->AddLocalPlanner(arsl, "AvoidRegionSL");

  //avoid-region + PQP_SOLID validity checker
  vector<string> vcList;
  vcList.push_back("PQP_SOLID");
  vcList.push_back("AvoidRegionValidity");
  VizmoProblem::ValidityCheckerPointer rv(new ComposeValidity<VizmoTraits>(
        ComposeValidity<VizmoTraits>::AND, vcList));
  problem->AddValidityChecker(rv, "RegionValidity");

  //avoid-region + PQP_SOLID local planner
  VizmoProblem::LocalPlannerPointer rsl(
      new StraightLine<VizmoTraits>("RegionValidity", true));
  problem->AddLocalPlanner(rsl, "RegionSL");

  //region uniform random sampler
  VizmoProblem::SamplerPointer rus(
      new UniformRandomSampler<VizmoTraits>("RegionValidity"));
  problem->AddSampler(rus, "RegionUniformSampler");

  //region obstacle-based sampler
  /*VizmoProblem::SamplerPointer robs(
      new ObstacleBasedSampler<VizmoTraits>("RegionValidity", "euclidean"));
  problem->AddSampler(robs, "RegionObstacleSampler");
  */

  //region neighborhood connector
  VizmoProblem::ConnectorPointer rc(
      new NeighborhoodConnector<VizmoTraits>("BFNF", "RegionSL"));
  problem->AddConnector(rc, "RegionBFNFConnector");

  //set the MPProblem pointer and build CD structures
  problem->SetMPProblem();
  problem->BuildCDStructures();
}

void
Vizmo::
Clean() {
  delete m_envModel;
  delete m_phantomManager;
  delete m_spaceMouseManager;
  delete m_mapModel;
  delete m_queryModel;
  delete m_pathModel;
  delete m_debugModel;
  m_envModel = NULL;
  m_phantomManager = NULL;
  m_spaceMouseManager = NULL;
  m_mapModel = NULL;
  m_queryModel = NULL;
  m_pathModel = NULL;
  m_debugModel = NULL;

  m_loadedModels.clear();
  m_selectedModels.clear();

  delete GetVizmoProblem();
  GetVizmoProblem() = NULL;
}

void
Vizmo::
Draw() {
  typedef vector<Model*>::iterator MIT;
  for(auto& model : m_loadedModels)
    model->DrawRender();

  glColor3f(1,1,0); //Selections are yellow, so set the color once now
  for(auto& model : m_selectedModels)
    model->DrawSelected();
}

void
Vizmo::
Select(const Box& _box) {
  //prepare for selection mode
  GLuint hitBuffer[1024];
  glSelectBuffer(1024, hitBuffer);
  glRenderMode(GL_SELECT);

  //get view port
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  //initialize stack
  glInitNames();

  //change view volume
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

  //unselect everything first
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
  /// Currently PQP_SOLID is used when an XML file is loaded; otherwise cd2 is
  /// used.
  if(m_envModel) {
    VizmoProblem::ValidityCheckerPointer vc;

    if(!m_xmlFilename.empty())
      vc = GetVizmoProblem()->GetValidityChecker("cd2");
    else
      vc = GetVizmoProblem()->GetValidityChecker("PQP_SOLID");

    bool b = vc->IsValid(_c, "Vizmo");
    _c.SetValidity(b);
    return b;
  }
  cerr << "Warning::Collision checking when there is no environment. "
    << "Returning false." << endl;
  return false;
}

void
Vizmo::
ProcessAvoidRegions() {
  //get avoid regions and graph
  typedef EnvModel::RegionModelPtr RegionModelPtr;

  const vector<RegionModelPtr>& avoidRegions = GetVizmo().GetEnv()->GetAvoidRegions();

  //check that some avoid region needs processing
  bool skipCheck = true;
  for(typename vector<RegionModelPtr>::const_iterator rit = avoidRegions.begin();
      rit != avoidRegions.end(); ++rit) {
    if(!(*rit)->IsProcessed()) {
      skipCheck = false;
      (*rit)->Processed();
    }
  }
  if(skipCheck)
    return;

  //check is needed. get env, graph, vc, and lp
  typedef Roadmap<VizmoTraits> RGraph;
  typedef RGraph::GraphType GraphType;
  typedef typename GraphType::vertex_descriptor VID;
  typedef typename GraphType::vertex_iterator VI;
  typedef typename GraphType::edge_descriptor EID;
  typedef typename GraphType::adj_edge_iterator EI;

  GraphType* g = GetVizmoProblem()->GetRoadmap()->GetGraph();
  Environment* env = GetVizmo().GetEnv()->GetEnvironment();
  VizmoProblem::ValidityCheckerPointer vc = GetVizmoProblem()->
    GetValidityChecker("AvoidRegionValidity");
  VizmoProblem::LocalPlannerPointer lp =
    GetVizmoProblem()->GetLocalPlanner("AvoidRegionSL");

  vector<VID> verticesToDel;
  vector<EID> edgesToDel;

  //re-validate graph with avoid region validity
  //loop over the graph testing vertices for deletion
  for(VI vit = g->begin(); vit != g->end(); ++vit)
    if(!vc->IsValid(vit->property(), vc->GetNameAndLabel()))
      verticesToDel.push_back(vit->descriptor());

  //loop over the graph testing edges for deletion
  for(typename GraphType::edge_iterator eit = g->edges_begin();
      eit != g->edges_end(); ++eit) {
    LPOutput<VizmoTraits> lpOutput;
    CfgModel collisionCfg;
    if(!lp->IsConnected(g->GetVertex((*eit).source()),
          g->GetVertex((*eit).target()), collisionCfg, &lpOutput,
          env->GetPositionRes(), env->GetOrientationRes()))
      edgesToDel.push_back((*eit).descriptor());
  }

  //handle deletion of invalid edges and vertices
  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
  for(typename vector<EID>::iterator eit = edgesToDel.begin();
      eit != edgesToDel.end(); ++eit)
    g->delete_edge(*eit);
  for(typename vector<VID>::iterator vit = verticesToDel.begin();
      vit != verticesToDel.end(); ++vit)
    g->delete_vertex(*vit);

  GetVizmo().GetMap()->RefreshMap(false);
}

pair<bool, double>
Vizmo::
VisibilityCheck(CfgModel& _c1, CfgModel& _c2) {
  /// Currently, StraightLine is used for the local plan.
  if(m_envModel) {
    Environment* env = GetVizmoProblem()->GetEnvironment();
    VizmoProblem::LocalPlannerPointer lp = GetVizmoProblem()->
      GetLocalPlanner("sl");
    LPOutput<VizmoTraits> lpout;
    if(lp->IsConnected(_c1, _c2, &lpout,
          env->GetPositionRes(), env->GetOrientationRes()))
      return make_pair(true, lpout.m_edge.first.GetWeight());
  }
  else
    cerr << "Warning::Visibility checking when there is no environment. "
      << "Returning false" << endl;
  return make_pair(false, EdgeModel::MaxWeight().Weight());
}

void
Vizmo::
PlaceRobots() {
  if(m_envModel) {
    vector<CfgModel> cfgs;
    if(m_queryModel)
      cfgs.emplace_back(m_queryModel->GetQueryCfg(0));
    else if(m_pathModel)
      cfgs.emplace_back(m_pathModel->GetConfiguration(0));
    m_envModel->PlaceRobots(cfgs,
        m_pathModel || m_mapModel || m_queryModel || m_debugModel);
  }
}

void
Vizmo::
ReadMap(const string& _name) {
  m_mapFilename = _name;
  GetVizmoProblem()->GetRoadmap()->Read(_name);
  delete m_mapModel;
  m_mapModel = new MapModel<CfgModel, EdgeModel>(
      GetVizmoProblem()->GetRoadmap()->GetGraph());
  m_loadedModels.push_back(m_mapModel);
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
}

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
  m_timers[_c].first.restart();
}

void
Vizmo::
StopClock(const string& _c) {
  if(m_timers.count(_c))
    m_timers[_c].second = m_timers[_c].first.elapsed()/1000.;
  else
    m_timers[_c].second = 0;
}

void
Vizmo::
PrintClock(const string& _c, ostream& _os) {
  _os << _c << ": " << m_timers[_c].second << " sec" << endl;
}

void
Vizmo::
AdjustClock(const string& _c1, const string& _c2, const string& _op) {
  /// Adjusts clock \c _c1 by \c +/- \c _c2.second.
  if(_op == "-")
    m_timers[_c1].first =
      m_timers[_c1].first.addMSecs( m_timers[_c2].second * 1000);
  else if (_op == "+")
    m_timers[_c1].first =
      m_timers[_c1].first.addMSecs(-m_timers[_c2].second * 1000);
  else
    throw PMPLException("ClockError", WHERE,
        "unknown clock adjustment operation.");
}

void
Vizmo::
SetPMPLMap() {
  /// \warning Erases the previous MapModel if one exists.
  if(GetVizmo().IsRoadMapLoaded()) {
    vector<Model*>::iterator mit = find(m_loadedModels.begin(),
        m_loadedModels.end(), m_mapModel);
    m_loadedModels.erase(mit);
    delete m_mapModel;
  }
  m_mapModel = new MapModel<CfgModel, EdgeModel>(
      GetVizmoProblem()->GetRoadmap()->GetGraph());
  m_mapModel->SetEnvFileName(m_envFilename);
  m_loadedModels.push_back(m_mapModel);
}

void
Vizmo::
Solve(const string& _strategy) {
  SRand(m_seed);
  VizmoProblem::MPStrategyPointer mps =
    GetVizmoProblem()->GetMPStrategy(_strategy);
  string name = mps->GetNameAndLabel();
  name = name.substr(0, name.find("::"));

  ostringstream oss;

  //If the xml file is loaded, GetModelDataDir will be empty
  //and the vizmo debug file should be made in the same
  //directory as the xml file

  stringstream mySeed;
  mySeed << GetSeed();

  string baseFilename = MPProblemBase::GetPath(_strategy + "." + mySeed.str());
  GetVizmoProblem()->SetBaseFilename(baseFilename);
  oss << GetVizmoProblem()->GetBaseFilename() << ".vd";

  //Initialize Vizmo Debug
  VDInit(oss.str());

  //Add this for region methods so that regions made prior to solving are added
  //to Vizmo debug
  if(name.find("Region"))
    AddInitialRegions();

  mps->operator()();

  //Close the debug file
  VDClose();

  GetVizmo().GetMap()->RefreshMap();
}

double
Vizmo::
GetMaxEnvDist() {
  return GetVizmo().GetEnv()->GetEnvironment()->GetBoundary()->GetMaxDist();
}

vector<string>
Vizmo::
GetAllSamplers() const {
  vector<string> names;
  const VizmoProblem::SamplerSet* ss = GetVizmoProblem()->GetSamplers();
  for(auto& method : *ss)
    names.emplace_back(method.second->GetNameAndLabel());
  return names;
}

vector<string>
Vizmo::
GetAllStrategies() const {
  vector<string> names;
  const VizmoProblem::MPStrategySet* mps = GetVizmoProblem()->GetMPStrategies();
  for(auto& method : *mps)
    names.emplace_back(method.second->GetNameAndLabel());
  return names;
}
