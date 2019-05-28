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

////////////////////////////////////////////////////////////////////////////////
/// \brief   The original user-guided region PRM. It uses attract regions as
///          sub-problems and avoid regions as virtual obstacles.
/// \details The paper reference is Denny, Sandstrom, Julian, Amato, "A
///          Region-based Strategy for Collaborative Roadmap Construction",
///          Algorithmic Foundations of Robotics XI (WAFR 2014).
////////////////////////////////////////////////////////////////////////////////
template<class MPTraits>
class RegionStrategy : public MPStrategyMethod<MPTraits> {

  public:

    ///\name Motion Planning Types
    ///@{

    typedef typename MPTraits::MPProblemType               MPProblemType;
    typedef typename MPTraits::CfgType                     CfgType;
    typedef typename MPTraits::WeightType                  WeightType;
    typedef typename MPProblemType::RoadmapType::VID       VID;
    typedef typename MPProblemType::RoadmapType::GraphType GraphType;
    typedef EnvModel::RegionModelPtr                       RegionModelPtr;

    ///@}
    ///\name Construction
    ///@{

    RegionStrategy();
    RegionStrategy(MPProblemType* _problem, XMLNode& _node);

    ///@}
    ///\name MPStrategyMethod Overrides
    ///@{

    virtual void Initialize() override;
    virtual void Iterate() override;
    virtual void Finalize() override;

    ///@}

  protected:

    ///\name Helpers
    ///@{

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Add a set of configurations to the roadmap.
    /// \param[in] _samples The new configurations to add.
    /// \param[out] _vids   The resulting VIDs of the new samples.
    void AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Try to connect a set of nodes.
    /// \param[in] _vids The VIDs of the nodes to connect.
    /// \param[in] _i    The current iteration number. Used in updating region
    ///                  colors.
    void Connect(vector<VID>& _vids, size_t _i);

    ///@}
    ///\name Region Functions
    ///@{

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Selects an attract region or the entire environment with uniform
    ///         probability.
    /// \return The index of the selected region, or the number of regions if
    ///         the entire environment was chosen.
    size_t SelectRegion();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Sample the selected region, tracking failure information for
    ///         usefulness feedback.
    /// \param[in] _index    The index of the selected region.
    /// \param[out] _samples The newly created samples.
    void SampleRegion(size_t _index, vector<CfgType>& _samples);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Checks the VID list for unconnected nodes and recommends a
    ///         region around them.
    /// \param[in] _vids The list of nodes to check.
    /// \param[in] _i    The current iteration number. Used for tracking the
    ///                  region creation time.
    void RecommendRegions(vector<VID>& _vids, size_t _i);

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Checks over the graph to re-validate the sample count within the
    ///         current sampling region.
    void UpdateRegionStats();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief  Updates the usefulness color of the current sampling region and
    ///         deletes lingering recommended regions.
    void UpdateRegionColor(size_t _i);

    ///@}
    ///\name PMPL Object Labels
    ///@{

    string m_vcLabel{"RegionValidity"};             ///< The validity checker.
    string m_samplerLabel{"RegionUniformSampler"};  ///< The sampler.
    string m_lpLabel{"RegionSL"};                   ///< The local planner.
    string m_connectorLabel{"RegionBFNFConnector"}; ///< The connector.

    ///@}
    ///\name Other Internal State
    ///@{

    size_t m_iteration;               ///< Tracks number of iterations.
    RegionModelPtr m_samplingRegion;  ///< Points to the current sampling region.

    ///@}
};

/*------------------------------- Construction -------------------------------*/

template<class MPTraits>
RegionStrategy<MPTraits>::
RegionStrategy() : MPStrategyMethod<MPTraits>() {
  this->SetName("RegionStrategy");
}


template<class MPTraits>
RegionStrategy<MPTraits>::
RegionStrategy(MPProblemType* _problem, XMLNode& _node) :
    MPStrategyMethod<MPTraits>(_problem, _node) {
  this->SetName("RegionStrategy");
  m_vcLabel = _node.Read("vcLabel", false, m_vcLabel, "Validity Checker");
  m_samplerLabel = _node.Read("sLabel", false, m_samplerLabel,
      "Sampler Strategy");
  m_lpLabel = _node.Read("lpLabel", false, m_lpLabel, "Local Planner");
  m_connectorLabel = _node.Read("cLabel", false, m_connectorLabel,
      "Connection Strategy");
}

/*------------------------- MPStrategyMethod Overrides -----------------------*/

template<class MPTraits>
void
RegionStrategy<MPTraits>::
Initialize() {
  cout << "Initializing " << this->GetNameAndLabel() << "." << endl;

  //Make non-region objects non-selectable
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);

  m_iteration = 0;

  cout << "Running Region Strategy." << endl;
  GetVizmo().StartClock("RegionStrategy");
}


template<class MPTraits>
void
RegionStrategy<MPTraits>::
Iterate() {
  ++m_iteration;

  //pick a region
  size_t index = SelectRegion();

  //sample region
  vector<CfgType> samples;
  SampleRegion(index, samples);

  //add sample to map
  vector<VID> vids;
  AddToRoadmap(samples, vids);

  //connect roadmap
  Connect(vids, m_iteration);

  if(m_iteration > 200)
    RecommendRegions(vids, m_iteration);

  GetVizmo().ProcessAvoidRegions();
  GetVizmo().GetMap()->RefreshMap();
}


template<class MPTraits>
void
RegionStrategy<MPTraits>::
Finalize() {
  GetVizmo().StopClock("RegionStrategy");

  cout << "Finalizing Region Strategy." << endl;

  StatClass* stats = this->GetStatClass();
  string basename = this->GetBaseFilename();

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //print clocks
  GetVizmo().PrintClock("Pre-input", cout);
  GetVizmo().PrintClock("RegionStrategy", cout);
  stats->PrintClock("RegionStrategyMP", cout);

  //output stat class
  ofstream ostats((basename + ".stat").c_str());

  ostats << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(ostats, this->GetRoadmap());

  GetVizmo().PrintClock("Pre-input", ostats);
  GetVizmo().PrintClock("RegionStrategy", ostats);
  stats->PrintClock("RegionStrategyMP", ostats);

  //output roadmap
  this->GetRoadmap()->Write(basename + ".map", this->GetEnvironment());

  //show results pop-up
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("Pre-input", results);
  GetVizmo().PrintClock("RegionStrategy", results);

  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
}

/*--------------------------------- Helpers ----------------------------------*/

template<class MPTraits>
void
RegionStrategy<MPTraits>::
AddToRoadmap(vector<CfgType>& _samples, vector<VID>& _vids) {
  //add nodes in _samples to graph. store VID's in _vids for connecting
  _vids.clear();
  for(auto& cfg : _samples) {
    VID addedNode = this->GetRoadmap()->GetGraph()->AddVertex(cfg);
    _vids.push_back(addedNode);
  }
}


template<class MPTraits>
void
RegionStrategy<MPTraits>::
Connect(vector<VID>& _vids, size_t _i) {
  auto cp = this->GetConnector(m_connectorLabel);
  cp->Connect(this->GetRoadmap(), _vids.begin(), _vids.end());

  UpdateRegionStats();
  UpdateRegionColor(_i);
}

/*----------------------------- Region Functions -----------------------------*/

template<class MPTraits>
size_t
RegionStrategy<MPTraits>::
SelectRegion() {
  //get regions from vizmo
  const vector<RegionModelPtr>& regions = GetVizmo().GetEnv()->
      GetAttractRegions();

  //randomly choose a region
  return rand() % (regions.size() + 1);
}


template<class MPTraits>
void
RegionStrategy<MPTraits>::
SampleRegion(size_t _index, vector<CfgType>& _samples) {
  //setup access pointers
  shared_ptr<Boundary> samplingBoundary;
  const vector<RegionModelPtr>& regions = GetVizmo().GetEnv()->
      GetAttractRegions();
  auto sp = this->GetSampler(m_samplerLabel);

  //check if the selected region is a region or the environment boundary.  if it
  //is the env boundary, set m_samplingRegion to null
  if(_index == regions.size()) {
    m_samplingRegion.reset();
    samplingBoundary = this->GetEnvironment()->GetBoundary();
  }
  else {
    m_samplingRegion = regions[_index];
    samplingBoundary = m_samplingRegion->GetBoundary();
    sp = this->GetSampler(regions[_index]->GetSampler());
  }

  //attempt to sample the region. track failures in col for density calculation.
  try {
    //track failures in col for density calculation
    vector<CfgType> col;
    sp->Sample(1, 1, samplingBoundary,
        back_inserter(_samples), back_inserter(col));

    //if this region is not the environment boundary, update failure count
    if(m_samplingRegion != NULL)
      m_samplingRegion->IncreaseFACount(col.size());
  }
  //catch Boundary too small exception
  catch(const PMPLException& _e) {
    m_samplingRegion->IncreaseFACount(1000000);
    return;
  }
  //catch all others and exit
  catch(const std::exception& _e) {
    cerr << _e.what() << endl;
    exit(1);
  }
}


template<class MPTraits>
void
RegionStrategy<MPTraits>::
RecommendRegions(vector<VID>& _vids, size_t _i) {
  if(!m_samplingRegion) {
    GraphType* g = this->GetRoadmap()->GetGraph();
    typedef typename vector<VID>::iterator VIT;
    for(VIT vit = _vids.begin(); vit != _vids.end(); ++vit) {
      if(g->get_degree(*vit) < 1) {
        //node is not connected to anything
        //recommend a region
        RegionModelPtr r;
        if(GetVizmo().GetEnv()->IsPlanar())
          r = RegionModelPtr(new RegionSphere2DModel(
              g->GetVertex(*vit).GetPoint(),
              this->GetEnvironment()->GetPositionRes() * 100, false));
        else
          r = RegionModelPtr(new RegionSphereModel(g->GetVertex(*vit).GetPoint(),
              this->GetEnvironment()->GetPositionRes() * 100, false));
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
    Environment* ep = this->GetEnvironment();
    GraphType* g = this->GetRoadmap()->GetGraph();
    shared_ptr<Boundary> bbx = m_samplingRegion->GetBoundary();

    //clear m_numVertices
    m_samplingRegion->ClearNodeCount();

    //iterate through graph to find which vertices are in the modified region
    for(typename GraphType::iterator git = g->begin(); git != g->end(); git++) {
      const CfgType& c = g->GetVertex(git);
      if(ep->InBounds(c, bbx))
        m_samplingRegion->IncreaseNodeCount(1);
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
  typedef vector<RegionModelPtr>::const_iterator RIT;
  vector<RegionModelPtr> nonCommit = GetVizmo().GetEnv()->GetNonCommitRegions();
  for(RIT rit = nonCommit.begin(); rit != nonCommit.end(); ++rit) {
    if((*rit)->GetCreationIter() != size_t(-1)) {
      double iterCount = 1000 - _i + (*rit)->GetCreationIter();
      if(iterCount <= 0)
        GetVizmo().GetEnv()->DeleteRegion(*rit);
      else
        (*rit)->SetColor(Color4(0, 0, 1, iterCount / 1250.));
    }
  }
}

/*----------------------------------------------------------------------------*/

#endif
