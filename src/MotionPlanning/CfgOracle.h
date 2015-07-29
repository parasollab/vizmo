#ifndef CFG_ORACLE_H_
#define CFG_ORACLE_H_

#include "RegionStrategy.h"
#include "MPStrategies/MPStrategyMethod.h"

#include "GUI/MainWindow.h"
#include "GUI/ModelSelectionWidget.h"

#include "Models/Vizmo.h"

#include "MotionPlanning/VizmoTraits.h"

template<class MPTraits>
class CfgOracle : public MPStrategyMethod<MPTraits> {

  public:

    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;
    typedef typename MPProblemType::RoadmapType::VID VID;
    typedef typename MPProblemType::RoadmapType RoadmapType;
    typedef typename MPProblemType::RoadmapType::GraphType GraphType;
    typedef typename GraphType::VI VI;
    typedef typename GraphType::EID EID;
    typedef EnvModel::RegionModelPtr RegionModelPtr;

    CfgOracle(string _inputMapFilename="");
    CfgOracle(MPProblemType* _problem, XMLNode& _node);

    void Initialize();
    void Run();
    void Finalize();
    virtual void ParseXML(XMLNode& _node);
    string m_cfgmap;
    string m_baseXMLName;
  private:
    vector<string> m_strategy;
  protected:
    string m_inputMapFilename;

};


template<class MPTraits>
CfgOracle<MPTraits>::
CfgOracle(string _inputMapFilename) : MPStrategyMethod<MPTraits>() {
  this->SetName("CfgOracle");
}


template<class MPTraits>
CfgOracle<MPTraits>::
CfgOracle(MPProblemType* _problem, XMLNode& _node) :
  MPStrategyMethod<MPTraits>(_problem, _node), m_inputMapFilename("") {
    this->SetName("CfgOracle");
    ParseXML(_node);
  }


template<class MPTraits>
void
CfgOracle<MPTraits>::
ParseXML(XMLNode& _node) {
  m_inputMapFilename = _node.Read("cfgMap", false, "", "Cfg Map Filename");
 m_baseXMLName = _node.Read("baseFilename", false, "", "Base file name from the XML.");

  for(auto& child : _node)
    if(child.Name() == "MPStrategy"){
      m_strategy.emplace_back(child.Read("method", true, "", "MPStrategy from VizmoXML"));
    }
 }


template<class MPTraits>
void
CfgOracle<MPTraits>::
Initialize() {
  cout << "Initializing " << this->GetNameAndLabel() << "." << endl;

  //Make non-region objects non-selectable
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);

  RoadmapType* r = this->GetRoadmap();
  r->Read(m_inputMapFilename.c_str());
  GetVizmo().GetMap()->RefreshMap();
  cout << "Input Map: " << m_inputMapFilename << endl;

  //usleep(1000000);
}


template<class MPTraits>
void
CfgOracle<MPTraits>::
Run() {

  GetVizmo().StartClock("CfgOracle");
  this->GetStatClass()->StartClock("CfgOracleMP");
  GetVizmo().StopClock("Pre-cfgs");   

  cout<< "Running CfgOracle."<< endl;

  for(auto& label : m_strategy){
    cout << "CfgOracle Beginning Strategy: " << label << endl;
    (*this->GetMPStrategy(label))();
  }

  GetVizmo().GetMap()->RefreshMap();
  //stop clock
  GetVizmo().StopClock("CfgOracle");
  this->GetStatClass()->StopClock("CfgOracleMP");
}


template<class MPTraits>
void
CfgOracle<MPTraits>::
Finalize() {
  //set up variables
  StatClass* stats = this->GetStatClass();
  string basename = this->GetBaseFilename(); //m_baseXMLName;
  cout << "Finalizing CfgOracle." << endl;

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //print clocks
  GetVizmo().PrintClock("Pre-cfgs", cout);
  GetVizmo().PrintClock("CfgOracle", cout);
  stats->PrintClock("CfgOracleMP", cout);

  //output stat class
  ofstream ostats((basename + ".stat").c_str());

  ostats << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(ostats, this->GetRoadmap());

  GetVizmo().PrintClock("Pre-cfgs", ostats);
  GetVizmo().PrintClock("CfgOracle", ostats);
  stats->PrintClock("CfgOracleMP", ostats);

  //output roadmap
  this->GetRoadmap()->Write(basename + ".map", this->GetEnvironment());

  //show results pop-up
  ostringstream results;
  results << "Planning Cfg Complete!" << endl;
  GetVizmo().PrintClock("Pre-cfgs", results);
  GetVizmo().PrintClock("CfgOracle", results);

  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
}

#endif
