#ifndef SPARK_REGION_H_
#define SPARK_REGION_H_

#include "RegionStrategy.h"

template<class MPTraits>
class SparkRegion : public RegionStrategy<MPTraits> {
  public:
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPProblemType::RoadmapType::VID VID;
    typedef typename MPProblemType::GraphType GraphType;

    typedef EnvModel::RegionModelPtr RegionModelPtr;

    SparkRegion();
    SparkRegion(MPProblemType* _problem, XMLNodeReader& _node);

    void Initialize();
    void Run();
    void Finalize();

  protected:
    void DeleteRegion(size_t _index);
};

template<class MPTraits>
SparkRegion<MPTraits>::
SparkRegion() {
  this->SetName("SparkRegion");
}

template<class MPTraits>
SparkRegion<MPTraits>::
SparkRegion(MPProblemType* _problem, XMLNodeReader& _node) : RegionStrategy<MPTraits>(_problem, _node) {
  this->SetName("SparkRegion");
}

template<class MPTraits>
void
SparkRegion<MPTraits>::
Initialize() {
  cout << "Initializing Spark Region Strategy." << endl;
  if(GetVizmo().IsQueryLoaded()) {
    string basename = this->GetBaseFilename();
    this->SetQuery();
  }

  //Make non-region objects non-selectable
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
  GetVizmo().GetRobot()->SetSelectable(false);
}

template<class MPTraits>
void
SparkRegion<MPTraits>::
Run() {
  cout << "Running Spark Region Strategy." << endl;

  //start clock
  GetVizmo().StartClock("SparkRegion");
  this->GetMPProblem()->GetStatClass()->StartClock("SparkRegionMP");

  size_t iter = 0;
  while(!this->EvaluateMap()) {

    vector<CfgType> samples;
    size_t index = this->SelectRegion();
    this->SampleRegion(index, samples);

    this->ProcessAvoidRegions();

    vector<VID> vids;
    this->AddToRoadmap(samples, vids);

    this->Connect(vids, iter);

    if(samples.size() && index != GetVizmo().GetEnv()->GetAttractRegions().size()) {
      typedef typename vector<VID>::iterator VIT;
      for(VIT vit = vids.begin(); vit != vids.end(); ++vit)
        this->CheckNarrowPassageSample(*vit);
      //DeleteRegion(index);
    }

    //if(++iter % 20 == 0)
      GetVizmo().GetMap()->RefreshMap();
    //usleep(10000);
  }

  //stop clock
  GetVizmo().StopClock("SparkRegion");
  this->GetMPProblem()->GetStatClass()->StopClock("SparkRegionMP");
}

template<class MPTraits>
void
SparkRegion<MPTraits>::
Finalize() {
  cout << "Finalizing Spark Region Strategy." << endl;
  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //base filename
  string basename = this->GetBaseFilename();

  //print clocks + output a stat class
  StatClass* stats = this->GetMPProblem()->GetStatClass();
  GetVizmo().PrintClock("Pre-regions", cout);
  GetVizmo().PrintClock("SparkRegion", cout);
  stats->PrintClock("SparkRegionMP", cout);

  ofstream ostats((basename + ".stat").c_str());
  ostats << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(ostats, this->GetMPProblem()->GetRoadmap());
  GetVizmo().PrintClock("Pre-regions", ostats);
  GetVizmo().PrintClock("SparkRegion", ostats);
  stats->PrintClock("SparkRegionMP", ostats);

  //output roadmap
  ofstream ofs((basename + ".map").c_str());
  this->GetMPProblem()->GetRoadmap()->Write(ofs, this->GetMPProblem()->GetEnvironment());

  //show results pop-up
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("Pre-regions", results);
  GetVizmo().PrintClock("SparkRegion", results);

  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
  GetVizmo().GetRobot()->SetSelectable(true);
}

template<class MPTraits>
void
SparkRegion<MPTraits>::
DeleteRegion(size_t _index) {
  const vector<RegionModelPtr>& regions = GetVizmo().GetEnv()->GetAttractRegions();
  GetVizmo().GetSelectedModels().clear();
  GetMainWindow()->GetGLWidget()->SetCurrentRegion();
  GetVizmo().GetEnv()->DeleteRegion(regions[_index]);
}

#endif
