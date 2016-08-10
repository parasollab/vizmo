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

/*------------------------------- Singletons ---------------------------------*/

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

/*------------------------------- Construction -------------------------------*/

bool
Vizmo::
InitModels() {
  Clean();
  try {
    // Create environment first.
    if(m_envFilename.empty())
      throw ParseException(WHERE, "Vizmo must load an environment file.");

    m_envModel = new EnvModel(m_envFilename);
    m_loadedModels.push_back(m_envModel);

    // Create PMPL object and set environment to Vizmo's env.
    VizmoProblem*& problem = GetVizmoProblem();
    problem = new VizmoProblem();
    problem->SetEnvironment(m_envModel->GetEnvironment());

    // Use the query specified by Vizmo.
    if(!m_queryFilename.empty()) {
      problem->SetQueryFilename(m_queryFilename);
      m_queryModel = new QueryModel(m_queryFilename);
      m_loadedModels.push_back(m_queryModel);
      cout << "Loaded Query File : " << m_queryFilename << endl;
    }

    // Parse XML file now that env and query are set.
    problem->ReadXMLFile(m_xmlFilename, problem);

    // Try to initialize PHANToM.
    m_phantomManager = new Haptics::Manager();

    // Try to initialize space mouse.
    m_spaceMouseManager = new SpaceMouseManager();
    m_spaceMouseManager->Enable();
    m_spaceMouseManager->EnableCamera();

    // Create map model.
    if(!m_mapFilename.empty()) {
      m_mapModel = new MapModel<CfgModel, EdgeModel>(m_mapFilename);
      m_loadedModels.push_back(m_mapModel);
      cout << "Loaded Map File : " << m_mapFilename << endl;
    }

    // Create path model.
    if(!m_pathFilename.empty()) {
      m_pathModel = new PathModel(m_pathFilename);
      m_loadedModels.push_back(m_pathModel);
      cout << "Loaded Path File : " << m_pathFilename << endl;
    }

    // Create debug model.
    if(!m_debugFilename.empty()){
      m_debugModel = new DebugModel(m_debugFilename);
      m_loadedModels.push_back(m_debugModel);
      cout << "Loaded Debug File : " << m_debugFilename << endl;
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

  // Put robot in start cfg, if availiable.
  PlaceRobots();

  return true;
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
  m_envModel = nullptr;
  m_phantomManager = nullptr;
  m_spaceMouseManager = nullptr;
  m_mapModel = nullptr;
  m_queryModel = nullptr;
  m_pathModel = nullptr;
  m_debugModel = nullptr;

  m_loadedModels.clear();
  m_selectedModels.clear();

  delete GetVizmoProblem();
  GetVizmoProblem() = nullptr;
}

/*-------------------------------- Rendering ---------------------------------*/

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

/*-------------------------------- Selection ---------------------------------*/

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

    delete[] curName;  //free preallocated mem
  }

  //only the closest
  if(!_all) {
    // analyze selected item
    if(selName && selName[0] <= m_loadedModels.size()) {
      Model* selectModel = m_loadedModels[selName[0]];
      selectModel->Select(&selName[1], m_selectedModels);
    }
  }
  delete[] selName;
}

/*-------------------------- Environment Functions ---------------------------*/

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


double
Vizmo::
GetMaxEnvDist() {
  return GetVizmo().GetEnv()->GetEnvironment()->GetBoundary()->GetMaxDist();
}

/*----------------------------- Roadmap Functions ----------------------------*/

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

/*----------------------- Collision Detection Functions ----------------------*/

bool
Vizmo::
CollisionCheck(CfgModel& _c) {
  if(m_envModel) {
    VizmoProblem::ValidityCheckerPointer vc;
    vc = GetVizmoProblem()->GetValidityChecker("cd4");

    bool b = vc->IsValid(_c, "Vizmo");
    _c.SetValidity(b);
    return b;
  }
  cerr << "Warning::Collision checking when there is no environment. "
    << "Returning false." << endl;
  return false;
}

pair<bool, double>
Vizmo::
VisibilityCheck(CfgModel& _c1, CfgModel& _c2) {
  /// \note Currently, StraightLine is used for the local plan.
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

/*------------------------- Motion Planning Functions ------------------------*/

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


void
Vizmo::
ProcessAvoidRegions() {
  //get avoid regions and graph
  typedef EnvModel::RegionModelPtr RegionModelPtr;

  const vector<RegionModelPtr>& avoidRegions = GetVizmo().GetEnv()->
      GetAvoidRegions();

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
  auto vc = GetVizmoProblem()->GetValidityChecker("AvoidRegionValidity");
  auto lp = GetVizmoProblem()->GetLocalPlanner("AvoidRegionSL");

  vector<VID> verticesToDel;
  vector<EID> edgesToDel;

  //re-validate graph with avoid region validity
  //loop over the graph testing vertices for deletion
  for(auto vit = g->begin(); vit != g->end(); ++vit)
    if(!vc->IsValid(vit->property(), vc->GetNameAndLabel()))
      verticesToDel.push_back(vit->descriptor());

  //loop over the graph testing edges for deletion
  for(auto eit = g->edges_begin(); eit != g->edges_end(); ++eit) {
    LPOutput<VizmoTraits> lpOutput;
    CfgModel collisionCfg;
    if(!lp->IsConnected(g->GetVertex((*eit).source()),
          g->GetVertex((*eit).target()), collisionCfg, &lpOutput,
          env->GetPositionRes(), env->GetOrientationRes()))
      edgesToDel.push_back((*eit).descriptor());
  }

  //handle deletion of invalid edges and vertices
  QMutexLocker locker(&GetVizmo().GetMap()->AcquireMutex());
  for(auto& e : edgesToDel)
    g->delete_edge(e);
  for(auto& v : verticesToDel)
    g->delete_vertex(v);

  GetVizmo().GetMap()->RefreshMap(false);
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

/*---------------------------------- Timing ----------------------------------*/

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

/*----------------------------------------------------------------------------*/
