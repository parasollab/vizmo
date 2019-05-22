#ifndef PATH_ORACLE_H_
#define PATH_ORACLE_H_

#include "OracleStrategy.h"

#include "Models/EnvModel.h"
#include "Models/UserPathModel.h"
#include "Models/Vizmo.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief Provide one or more paths to a planner prior to execution.
////////////////////////////////////////////////////////////////////////////////
template<class MPTraits>
class PathOracle : public OracleStrategy<MPTraits> {

  public:

    ///\name Motion Planning Types
    ///@{

    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType       CfgType;
    typedef typename MPTraits::WeightType    WeightType;

    ///@}
    ///\name Construction
    ///@{

    PathOracle(const string& _strategy = "", const string& _inputPaths = "");
    PathOracle(MPProblemType* _problem, XMLNode& _node);

    ///@}
    ///\name MPStrategyMethod Overrides
    ///@{

    virtual void Initialize() override;

    ///@}
};

/*------------------------------- Construction -------------------------------*/

template<class MPTraits>
PathOracle<MPTraits>::
PathOracle(const string& _inputPaths, const string& _strategy) :
    OracleStrategy<MPTraits>(_strategy, _inputPaths) {
  this->SetName("PathOracle");
}


template<class MPTraits>
PathOracle<MPTraits>::
PathOracle(MPProblemType* _problem, XMLNode& _node) :
    OracleStrategy<MPTraits>(_problem, _node) {
  this->SetName("PathOracle");
  this->m_oracleInput = _node.Read("pathFile", false, "", "Path Filename");
}

/*------------------------- MPStrategyMethod Overrides -----------------------*/

template<class MPTraits>
void
PathOracle<MPTraits>::
Initialize() {
  OracleStrategy<MPTraits>::Initialize();

  auto g = this ->GetRoadmap()->GetGraph();
  g->clear();

  if(!this->m_oracleInput.empty()) {
    GetVizmo().GetEnv()->LoadUserPaths(this->m_oracleInput);
    cout << "Input Path: " << this->m_oracleInput << endl;
  }

  const vector<UserPathModel*>& userPaths = GetVizmo().GetEnv()->GetUserPaths();
  shared_ptr<vector<CfgType>> cfgs;

  for(const auto& path : userPaths) {
    cfgs = path->GetCfgs();
    auto v1 = g->AddVertex(cfgs->front());
    auto v2 = g->AddVertex(cfgs->back());

    vector<CfgType> intermediates, backIntermediates;
    copy(cfgs->begin() + 1, cfgs->end() - 1, back_inserter(intermediates));
    copy(cfgs->rbegin() + 1, cfgs->rend() - 1, back_inserter(backIntermediates));

    g->AddEdge(v1, v2, make_pair(WeightType("", 1, intermediates),
          WeightType("", 1, backIntermediates)));
  }
  GetVizmo().GetMap()->RefreshMap();
}

/*----------------------------------------------------------------------------*/

#endif
