#ifndef PATH_ORACLE_H_
#define PATH_ORACLE_H_

#include "MPStrategies/MPStrategyMethod.h"

#include "GUI/MainWindow.h"
#include "GUI/ModelSelectionWidget.h"

#include "Models/EnvModel.h"
#include "Models/UserPathModel.h"
#include "Models/Vizmo.h"

#include "MotionPlanning/VizmoTraits.h"

#include "Utilities/GLUtils.h"
template<class MPTraits>
class PathOracle : public MPStrategyMethod<MPTraits> {

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

    PathOracle(string _inputPathFilename="");
    PathOracle(MPProblemType* _problem, XMLNode& _node);
    virtual void ParseXML(XMLNode& _node);

    void Initialize();
    void Run();
    void Finalize();

  private:
    vector<string> m_strategy;
  protected:

    string m_inputPathFilename;
};


template<class MPTraits>
PathOracle<MPTraits>::
PathOracle(string _inputPathFilename) : MPStrategyMethod<MPTraits>() {
  this->SetName("PathOracle");
}


template<class MPTraits>
PathOracle<MPTraits>::
PathOracle(MPProblemType* _problem, XMLNode& _node) :
  MPStrategyMethod<MPTraits>(_problem, _node), m_inputPathFilename("") {
    this->SetName("PathOracle");
    ParseXML(_node);
  }

template<class MPTraits>
void
PathOracle<MPTraits>::
ParseXML(XMLNode& _node) {
  m_inputPathFilename = _node.Read("pathfile", false, "", "Path Filename");

  for(auto& child : _node)
    if(child.Name() == "MPStrategy"){
      m_strategy.emplace_back(child.Read("method", true, "", "MPStrategy from VizmoXML"));
    }
}

template<class MPTraits>
void
PathOracle<MPTraits>::
Initialize() {
  string basename = this->GetBaseFilename();
  //Make non-region objects non-selectable
  GetVizmo().GetMap()->SetSelectable(false);
  GetVizmo().GetEnv()->SetSelectable(false);
  GraphType* g = this ->GetRoadmap()->GetGraph();

  vector<UserPathModel*> userPaths;
  userPaths = GetVizmo().GetEnv()->GetUserPaths();
  shared_ptr<vector<CfgType> > cfgs;
  for(const auto& path : userPaths) {
    cfgs = path->GetCfgs();
    VID v1 = g->AddVertex(cfgs->front());
    VID v2 = g->AddVertex(cfgs->back());

    vector<CfgType> intermediates, backIntermediates;
    copy(cfgs->begin() + 1, cfgs->begin()+cfgs->size()-1,
        back_inserter(intermediates));
    reverse_copy(cfgs->begin() + 1, cfgs->begin()+cfgs->size()-1,
        back_inserter(backIntermediates));

    g->AddEdge(v1, v2, make_pair(WeightType("", 1, intermediates),
          WeightType("", 1, backIntermediates)));
  }
  this->GetRoadmap()->Write(basename + ".cfgmap", this->GetEnvironment());

  if(!m_inputPathFilename.empty()){
    RoadmapType* r = this->GetRoadmap();
    r->Read(m_inputPathFilename.c_str());
    GetVizmo().GetMap()->RefreshMap();
    cout << "Input Path: " << m_inputPathFilename << endl;
  }
}


template<class MPTraits>
void
PathOracle<MPTraits>::
Run() {
  cout << "Running PathOracle." << endl;

  GetVizmo().StartClock("PathOracle");
  this->GetStatClass()->StartClock("PathOracleMP");

  for(auto& label : m_strategy){
    cout << "PathOracle Beginning Strategy: " << label << endl;
    (*this->GetMPStrategy(label))();
  }

  //stop clock
  GetVizmo().StopClock("PathOracle");
  this->GetStatClass()->StopClock("PathOracleMP");
}


template<class MPTraits>
void
PathOracle<MPTraits>::
Finalize() {
  //set up variables
  StatClass* stats = this->GetStatClass();
  string basename = this->GetBaseFilename();

  cout << "Finalizing PathOracle." << endl;

  //redraw finished map
  GetVizmo().GetMap()->RefreshMap();
  GetMainWindow()->GetModelSelectionWidget()->CallResetLists();

  //print clocks
  GetVizmo().PrintClock("Pre-region", cout);
  GetVizmo().PrintClock("PathOracle", cout);
  stats->PrintClock("PathOracleMP", cout);

  //output stat class
  ofstream ostats((basename + ".stat").c_str());

  ostats << "NodeGen+Connection Stats" << endl;
  stats->PrintAllStats(ostats, this->GetRoadmap());

  GetVizmo().PrintClock("Pre-regions", ostats);
  GetVizmo().PrintClock("PathOracle", ostats);
  stats->PrintClock("PathOracleMP", ostats);

  //output roadmap
  this->GetRoadmap()->Write(basename + ".map", this->GetEnvironment());

  //show results pop-up
  ostringstream results;
  results << "Planning Complete!" << endl;
  GetVizmo().PrintClock("Pre-regions", results);
  GetVizmo().PrintClock("PathOracle", results);

  GetMainWindow()->AlertUser(results.str());

  //Make things selectable again
  GetVizmo().GetMap()->SetSelectable(true);
  GetVizmo().GetEnv()->SetSelectable(true);
}

//  for(const auto& interm : cfgs) {
//  this->GetRoadmap()->GetGraph()->AddVertex(interm);
//  }
/*
   make iterator that goes through vector<cfgtype> cfgs
   inside this iterator the variable will get the nth and n+1th term which are cfgs

   put this through a AddEdge(), and it returns an edge
   this edge is added to the environment
   the nth node is added to the environment


   for (int i=1 ;1!= cfgs->size; i++){
   shared_ptr<vector<CfgType>>::iterator cfgsIT;
   LPOutput<MPTraits> lpOutput;
   GraphType* g = this ->GetRoadmap()->GetGraph();
   g->AddVertex(cfgs->front());
   for (cfgIT = cfgs[i].begin(); cfgsIT != cfgs[i].end(); cfgsIT++) {
   VID nextpoint = g->AddVertex(*(cfgsIT[i]++));
   g->AddEdge(g->GetVID(*cfgsIT[i]) , nextpoint, );
   g->AddVertex(*(CfgsIT[i]++));
   }
   }*/
#endif
