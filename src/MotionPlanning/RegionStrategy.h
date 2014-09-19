#ifndef REGION_STRATEGY_H_
#define REGION_STRATEGY_H_

#include "MPStrategies/MPStrategyMethod.h"

#include "GUI/MainWindow.h"
#include "GUI/ModelSelectionWidget.h"

#include "Models/RegionModel.h"
#include "Models/RegionSphereModel.h"
#include "Models/RegionSphere2DModel.h"
#include "Models/Vizmo.h"

#include "MotionPlanning/VizmoTraits.h"

template<class MPTraits>
class RegionStrategy : public MPStrategyMethod<MPTraits> {
  public:
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPProblemType::RoadmapType::VID VID;
    typedef typename MPProblemType::RoadmapType::GraphType GraphType;
    typedef typename GraphType::VI VI;
    typedef typename GraphType::EID EID;

    RegionStrategy();
    RegionStrategy(MPProblemType* _problem, XMLNodeReader& _node);

    void Initialize();
    void Run();
    void Finalize();

    virtual void Print(ostream& _os) const;

  protected:
    //helper functions
    size_t SelectRegion();
    void SampleRegion(size_t _index, vector<CfgType>& _samples);
    void ProcessAvoidRegions();
    void AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids);
    void Connect(vector<VID>& _vids, size_t _i);
    void RecommendRegions(vector<VID>& _vids, size_t _i);
    void UpdateRegionStats();
    void UpdateRegionColor(size_t _i);
    bool EvaluateMap();
    void SetupTools();

  private:
    RegionModel* m_samplingRegion;
    vector<VID> m_toDel;

    string m_connectorLabel;
    string m_lpLabel;
    string m_samplerLabel;
    string m_vcLabel;

    string m_regionSamplerLabel;
    string m_regionConnectorLabel;

    Query<MPTraits>* m_query;
};

template<class MPTraits>
RegionStrategy<MPTraits>::
RegionStrategy() : MPStrategyMethod<MPTraits>(),
    m_connectorLabel("BFNF"), m_lpLabel("sl"), m_samplerLabel("PQP_SOLID"),
    m_vcLabel("PQP_SOLID"), m_regionSamplerLabel("RegionSampler"),
    m_regionConnectorLabel("RegionConnector"), m_query(NULL) {
  this->SetName("RegionStrategy");
}

template<class MPTraits>
RegionStrategy<MPTraits>::
RegionStrategy(MPProblemType* _problem, XMLNodeReader& _node) :
    MPStrategyMethod<MPTraits>(_problem, _node),
    m_regionSamplerLabel("RegionSampler"),
    m_regionConnectorLabel("RegionConnector"), m_query(NULL) {
  this->SetName("RegionStrategy");
  m_connectorLabel = _node.stringXMLParameter("connectionLabel", false, "", "Connection Strategy");
  m_lpLabel = _node.stringXMLParameter("lpLabel", false, "", "Local Planner");
  m_samplerLabel = _node.stringXMLParameter("samplerLabel", false, "", "Sampler Strategy");
  m_vcLabel = _node.stringXMLParameter("vcLabel", false, "", "Validity Checker");
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::
Print(ostream& _os) const {
  _os << this->GetNameAndLabel() << endl;
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::
SetupTools() {

  //set up supporting tools for region strategy
  typename MPProblemType::ValidityCheckerPointer arv(
      new AvoidRegionValidity<MPTraits>());
  this->GetMPProblem()->AddValidityChecker(arv, "AvoidRegionValidity");

  vector<string> vcList;
  vcList.push_back(m_vcLabel);
  vcList.push_back("AvoidRegionValidity");
  typename MPProblemType::ValidityCheckerPointer rv(
      new ComposeValidity<MPTraits>(ComposeValidity<MPTraits>::AND, vcList));
  this->GetMPProblem()->AddValidityChecker(rv, "RegionValidity");

  typename MPProblemType::LocalPlannerPointer rsl(
      new StraightLine<MPTraits>("RegionValidity", true));
  this->GetMPProblem()->AddLocalPlanner(rsl, "RegionSL");

  typename MPProblemType::LocalPlannerPointer arsl(
      new StraightLine<MPTraits>("AvoidRegionValidity", true));
  this->GetMPProblem()->AddLocalPlanner(arsl, "AvoidRegionSL");

  typename MPProblemType::SamplerPointer rus(
      new UniformRandomSampler<MPTraits>("RegionValidity"));
  this->GetMPProblem()->AddSampler(rus, m_regionSamplerLabel);

  typename MPProblemType::ConnectorPointer rc(
      new NeighborhoodConnector<MPTraits>("BFNF", "RegionSL"));
  this->GetMPProblem()->AddConnector(rc, m_regionConnectorLabel);

  if(GetVizmo().IsQueryLoaded()) {
    //setup region query evaluator
    m_query = new Query<MPTraits>(GetVizmo().GetQryFileName(),
        vector<string>(1, m_regionConnectorLabel));
    m_query->SetPathFile(this->GetBaseFilename() + ".path");
    typename MPProblemType::MapEvaluatorPointer rq(m_query);
    this->GetMPProblem()->AddMapEvaluator(rq, "regionQuery");

    //set up bounded region query evaluator
    vector<string> evals;
    evals.clear();
    evals.push_back("NodesEval");
    evals.push_back("regionQuery");
    typename MPProblemType::MapEvaluatorPointer brq(
        new ComposeEvaluator<MPTraits>(ComposeEvaluator<MPTraits>::OR, evals));
    this->GetMPProblem()->AddMapEvaluator(brq, "BoundedRegionQuery");
  }

  this->GetMPProblem()->SetMPProblem();
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::
Initialize() {
  cout << "Initializing Region Strategy." << endl;

  string basename = this->GetBaseFilename();

  SetupTools();

  if(GetVizmo().IsQueryLoaded())
    boost::static_pointer_cast<Query<MPTraits> >(this->GetMPProblem()->
        GetMapEvaluator("Query"))->SetPathFile(basename + ".path");

  //Make non-region objects non-selectable
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
  GetVizmo().GetRobot()->SetSelectable(false);
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::Run() {
  cout << "Running Region Strategy." << endl;

  //start clock
  GetVizmo().StartClock("RegionStrategy");
  this->GetMPProblem()->GetStatClass()->StartClock("RegionStrategyMP");

  size_t iter = 0;
  while(!EvaluateMap()) {
    //pick a region
    size_t index = SelectRegion();

    //sample region
    vector<CfgType> samples;
    SampleRegion(index, samples);

    //add sample to map
    vector<VID> vids;
    AddToRoadmap(samples, vids);

    //connect roadmap
    Connect(vids, iter);

    if(iter > 200) {
      //recommend regions based upon vids
      RecommendRegions(vids, iter);
    }

    ProcessAvoidRegions();

    //refresh map and selection widget
    //if(++iter % 20 == 0) {
      GetVizmo().GetMap()->RefreshMap();
    //}
    //usleep(10000);
  }

  //stop clock
  GetVizmo().StopClock("RegionStrategy");
  this->GetMPProblem()->GetStatClass()->StopClock("RegionStrategyMP");
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::
Finalize() {
  //set up variables
  StatClass* stats = this->GetMPProblem()->GetStatClass();
  string basename = this->GetBaseFilename();

  cout << "Finalizing Region Strategy." << endl;

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //print clocks
  GetVizmo().PrintClock("Pre-regions", cout);
  GetVizmo().PrintClock("RegionStrategy", cout);
  stats->PrintClock("RegionStrategyMP", cout);

  //output stat class
  ofstream ostats((basename + ".stats").c_str());

  ostats << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(ostats, this->GetMPProblem()->GetRoadmap());

  GetVizmo().PrintClock("Pre-regions", ostats);
  GetVizmo().PrintClock("RegionStrategy", ostats);
  stats->PrintClock("RegionStrategyMP", ostats);

  //output roadmap
  ofstream ofs((basename + ".map").c_str());
  this->GetMPProblem()->GetRoadmap()->Write(ofs, this->GetMPProblem()->GetEnvironment());

  //output a path file
  if(GetVizmo().IsQueryLoaded())
    m_query->PerformQuery(this->GetMPProblem()->GetRoadmap());

  //show results pop-up
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("Pre-regions", results);
  GetVizmo().PrintClock("RegionStrategy", results);

  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
  GetVizmo().GetRobot()->SetSelectable(true);
}

template<class MPTraits>
size_t
RegionStrategy<MPTraits>::
SelectRegion() {
  //get regions from vizmo
  const vector<RegionModel*>& regions = GetVizmo().GetEnv()->GetAttractRegions();

  //randomly choose a region
  return rand() % (regions.size() + 1);
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::
SampleRegion(size_t _index, vector<CfgType>& _samples) {
  //setup access pointers
  shared_ptr<Boundary> samplingBoundary;
  const vector<RegionModel*>& regions =
    GetVizmo().GetEnv()->GetAttractRegions();
  typename MPProblemType::SamplerPointer sp =
    this->GetMPProblem()->GetSampler(m_regionSamplerLabel);

  //check if the selected region is a region or the environment boundary.  if it
  //is the env boundary, set m_samplingRegion to null
  if(_index == regions.size()) {
    m_samplingRegion = NULL;
    samplingBoundary = this->GetMPProblem()->GetEnvironment()->GetBoundary();
  }
  else {
    m_samplingRegion = regions[_index];
    samplingBoundary = m_samplingRegion->GetBoundary();
    sp = this->GetMPProblem()->GetSampler(regions[_index]->GetSampler());
  }

  //attempt to sample the region. track failures in col for density calculation.
  try {
    //track failures in col for density calculation
    vector<CfgType> col;
    sp->Sample(this->GetMPProblem()->GetEnvironment(), samplingBoundary,
        *this->GetMPProblem()->GetStatClass(), 1, 10,
        back_inserter(_samples), back_inserter(col));

    //if this region is not the environment boundary, update failure count
    if(m_samplingRegion != NULL)
      m_samplingRegion->IncreaseFACount(col.size());
  }
  //catch Boundary too small exception
  catch(PMPLException _e) {
    //static size_t numE = 0;
    //cout << _e.what() << endl << numE++ << endl;
    return;
  }
  //catch all others and exit
  catch(exception _e) {
    cerr << _e.what() << endl;
    exit(1);
  }
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::
ProcessAvoidRegions() {
  //get avoid regions and graph
  vector<RegionModel*> avoidRegions = GetVizmo().GetEnv()->GetAvoidRegions();

  //check that some avoid region needs processing
  bool skipCheck = true;
  for(typename vector<RegionModel*>::iterator rit = avoidRegions.begin();
      rit != avoidRegions.end(); ++rit) {
    if(!(*rit)->IsProcessed()) {
      skipCheck = false;
      (*rit)->Processed();
    }
  }
  if(skipCheck)
    return;

  //check is needed. get env, graph, vc, and lp
  GraphType* g = this->GetMPProblem()->GetRoadmap()->GetGraph();
  Environment* env = this->GetMPProblem()->GetEnvironment();
  typename MPProblemType::ValidityCheckerPointer vc =
    this->GetMPProblem()->GetValidityChecker("AvoidRegionValidity");
  typename MPProblemType::LocalPlannerPointer lp =
    this->GetMPProblem()->GetLocalPlanner("AvoidRegionSL");

  vector<VID> verticesToDel;
  vector<EID> edgesToDel;

  //re-validate graph with avoid region validity
  //loop over the graph testing vertices for deletion
  for(VI vit = g->begin(); vit != g->end(); ++vit)
    if(!vc->IsValid(vit->property(), this->GetNameAndLabel()))
      verticesToDel.push_back(vit->descriptor());
  //loop over the graph testing edges for deletion
  for(typename GraphType::edge_iterator eit = g->edges_begin();
      eit != g->edges_end(); ++eit) {
    LPOutput<MPTraits> lpOutput;
    CfgType collisionCfg;
    if(!lp->IsConnected(g->GetVertex((*eit).source()),
          g->GetVertex((*eit).target()), collisionCfg, &lpOutput,
          env->GetPositionRes(), env->GetOrientationRes()))
      edgesToDel.push_back((*eit).descriptor());
  }

  //handle deletion of invalid edges and vertices
  for(typename vector<EID>::iterator eit = edgesToDel.begin();
      eit != edgesToDel.end(); ++eit)
    g->delete_edge(*eit);
  for(typename vector<VID>::iterator vit = verticesToDel.begin();
      vit != verticesToDel.end(); ++vit)
    g->delete_vertex(*vit);

  GetVizmo().GetMap()->RefreshMap();
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::
AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids) {
  //add nodes in _samples to graph. store VID's in _vids for connecting
  _vids.clear();
  for(typename vector<CfgType>::iterator cit = _samples.begin();
      cit != _samples.end(); cit++) {
    VID addedNode = this->GetMPProblem()->GetRoadmap()->GetGraph()->AddVertex(*cit);
    _vids.push_back(addedNode);
  }
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::
Connect(vector<VID>& _vids, size_t _i) {
  stapl::sequential::vector_property_map<typename GraphType::GRAPH, size_t> cMap;
  typename MPProblemType::ConnectorPointer cp =
    this->GetMPProblem()->GetConnector(m_regionConnectorLabel);
  cp->Connect(this->GetMPProblem()->GetRoadmap(),
      *(this->GetMPProblem()->GetStatClass()), cMap, _vids.begin(), _vids.end());

  UpdateRegionStats();
  UpdateRegionColor(_i);
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::
RecommendRegions(vector<VID>& _vids, size_t _i) {
  if(!m_samplingRegion) {
    GraphType* g = this->GetMPProblem()->GetRoadmap()->GetGraph();
    typedef typename vector<VID>::iterator VIT;
    for(VIT vit = _vids.begin(); vit != _vids.end(); ++vit) {
      if(g->get_degree(*vit) < 1) {
        //node is not connected to anything
        //recommend a region
        RegionModel* r;
        if(GetVizmo().GetRobot()->IsPlanar())
          r = new RegionSphere2DModel(g->find_vertex(*vit)->property().GetPoint(),
              this->GetMPProblem()->GetEnvironment()->GetPositionRes() * 100, false);
        else
          r = new RegionSphereModel(g->find_vertex(*vit)->property().GetPoint(),
              this->GetMPProblem()->GetEnvironment()->GetPositionRes() * 100, false);
        r->SetCreationIter(_i);
        GetVizmo().GetEnv()->AddNonCommitRegion(r);
      }
    }
  }
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::
UpdateRegionStats() {
  if(m_samplingRegion) {
    //set up access pointers
    Environment* ep = this->GetMPProblem()->GetEnvironment();
    GraphType* g = this->GetMPProblem()->GetRoadmap()->GetGraph();
    shared_ptr<Boundary> bbx = m_samplingRegion->GetBoundary();

    //clear m_numVertices
    m_samplingRegion->ClearNodeCount();

    //iterate through graph to find which vertices are in the modified region
    //typedef CCModel<CfgType, WeightType>* CCM;
    //typedef set<CCM> SetCCs;
    //SetCCs ccs;
    for(typename GraphType::iterator git = g->begin(); git != g->end(); git++) {
      const CfgType& c = g->GetVertex(git);
      if(ep->InBounds(c, bbx)) {
        m_samplingRegion->IncreaseNodeCount(1);
        //ccs.insert(c.GetCC());
      }
    }
  }
}

template<class MPTraits>
void
RegionStrategy<MPTraits>::
UpdateRegionColor(size_t _i) {
  if(m_samplingRegion) {
    //update region color based on node density
    double densityRatio = exp(-sqr(m_samplingRegion->NodeDensity()));
    //double densityRatio = 1 - exp(-sqr(m_samplingRegion->CCDensity()));
    m_samplingRegion->SetColor(Color4(1 - densityRatio, densityRatio, 0., 0.5));
  }
  typedef vector<RegionModel*>::const_iterator RIT;
  vector<RegionModel*> nonCommit = GetVizmo().GetEnv()->GetNonCommitRegions();
  for(RIT rit = nonCommit.begin(); rit != nonCommit.end(); ++rit) {
    if((*rit)->GetCreationIter() != size_t(-1)) {
      double iterCount = 1000 - _i + (*rit)->GetCreationIter();
      if(iterCount <= 0)
        GetVizmo().GetEnv()->DeleteRegion(*rit);
      else
        (*rit)->SetColor(Color4(0, 0, 1, iterCount/1250.));
    }
  }
}

template<class MPTraits>
bool
RegionStrategy<MPTraits>::
EvaluateMap() {
  //set up map evaluator. Currently, NodesEval is used unless a query file is
  //loaded.
  vector<string> evalLabel;
  if(GetVizmo().IsQueryLoaded())
    evalLabel.push_back("BoundedRegionQuery");
  else
    evalLabel.push_back("NodesEval");

  bool eval = MPStrategyMethod<MPTraits>::EvaluateMap(evalLabel);
  return eval;
}

#endif
