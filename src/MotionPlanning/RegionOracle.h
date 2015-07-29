#ifndef REGION_ORACLE_H_
#define REGION_ORACLE_H_

#include "MPStrategies/MPStrategyMethod.h"

#include "GUI/MainWindow.h"
#include "GUI/ModelSelectionWidget.h"

#include "Models/Vizmo.h"

#include "MotionPlanning/VizmoTraits.h"

template<class MPTraits>
class RegionOracle : public MPStrategyMethod<MPTraits> {

  public:

    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPProblemType::RoadmapType::VID VID;
    typedef typename MPProblemType::RoadmapType::GraphType GraphType;
    typedef typename GraphType::VI VI;
    typedef typename GraphType::EID EID;
    typedef EnvModel::RegionModelPtr RegionModelPtr;

    RegionOracle();
    RegionOracle(MPProblemType* _problem, XMLNode& _node);

    void Initialize();
    void Run();
    void Finalize();
    virtual void ParseXML(XMLNode& _node);
  private:
    vector<string> m_strategy;
};


template<class MPTraits>
RegionOracle<MPTraits>::
RegionOracle() : MPStrategyMethod<MPTraits>() {
  this->SetName("RegionOracle");
}


template<class MPTraits>
RegionOracle<MPTraits>::
RegionOracle(MPProblemType* _problem, XMLNode& _node) :
  MPStrategyMethod<MPTraits>(_problem, _node) {
    this->SetName("RegionOracle");
    ParseXML(_node);
  }


template<class MPTraits>
void
RegionOracle<MPTraits>::
ParseXML(XMLNode& _node) {
  for(auto& child : _node)
    if(child.Name() == "MPStrategy")
      m_strategy.emplace_back(child.Read("method", true, "", "MPStrategy from VizmoXML"));
}


template<class MPTraits>
void
RegionOracle<MPTraits>::
Initialize() {
  cout << "Initializing " << this->GetNameAndLabel() << "." << endl;

  //Make non-region objects non-selectable
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
}


template<class MPTraits>
void
RegionOracle<MPTraits>::
Run() {
  cout << "Running RegionOracle." << endl;

  GetVizmo().StartClock("RegionOracle");
  this->GetStatClass()->StartClock("RegionOracleMP");

  for(auto& label : m_strategy){
    cout << "RegionOracle Beginning Strategy: " << label << endl;
    (*this->GetMPStrategy(label))();
  }

  //stop clock
  GetVizmo().StopClock("RegionOracle");
  this->GetStatClass()->StopClock("RegionOracleMP");
}


template<class MPTraits>
void
RegionOracle<MPTraits>::
Finalize() {
  //set up variables
  StatClass* stats = this->GetStatClass();
  string basename = this->GetBaseFilename();

  cout << "Finalizing RegionOracle." << endl;

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //print clocks
  GetVizmo().PrintClock("Pre-regions", cout);
  GetVizmo().PrintClock("RegionOracle", cout);
  stats->PrintClock("RegionOracleMP", cout);

  //output stat class
  ofstream ostats((basename + ".stat").c_str());

  ostats << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(ostats, this->GetRoadmap());

  GetVizmo().PrintClock("Pre-regions", ostats);
  GetVizmo().PrintClock("RegionOracle", ostats);
  stats->PrintClock("RegionOracleMP", ostats);

  //output roadmap
  this->GetRoadmap()->Write(basename + ".map", this->GetEnvironment());

  //show results pop-up
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("Pre-regions", results);
  GetVizmo().PrintClock("RegionOracle", results);

  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
}

#endif
