#ifndef PATH_ORACLE_H_
#define PATH_ORACLE_H_

#include "OracleStrategy.h"

#include "Models/EnvModel.h"
#include "Models/UserPathModel.h"
#include "Models/Vizmo.h"

template<class MPTraits>
class PathOracle : public OracleStrategy<MPTraits> {
  public:
    typedef typename MPTraits::MPProblemType MPProblemType;
    typedef typename MPTraits::CfgType CfgType;
    typedef typename MPTraits::WeightType WeightType;

    PathOracle(const string& _inputPaths = "", const string& _strategy = "");
    PathOracle(MPProblemType* _problem, XMLNode& _node);

    void Initialize();

  private:
    string m_inputPaths;
};

template<class MPTraits>
PathOracle<MPTraits>::
PathOracle(const string& _inputPaths, const string& _strategy) :
  OracleStrategy<MPTraits>(_strategy), m_inputPaths(_inputPaths) {
    this->SetName("PathOracle");
  }

template<class MPTraits>
PathOracle<MPTraits>::
PathOracle(MPProblemType* _problem, XMLNode& _node) :
  OracleStrategy<MPTraits>(_problem, _node) {
    this->SetName("PathOracle");
    m_inputPaths = _node.Read("pathFile", false, "", "Path Filename");
  }

template<class MPTraits>
void
PathOracle<MPTraits>::
Initialize() {
  OracleStrategy<MPTraits>::Initialize();

  auto g = this ->GetRoadmap()->GetGraph();
  g->clear();

  if(!m_inputPaths.empty()) {
    GetVizmo().GetEnv()->LoadUserPaths(m_inputPaths);
    cout << "Input Path: " << m_inputPaths << endl;
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

#endif
