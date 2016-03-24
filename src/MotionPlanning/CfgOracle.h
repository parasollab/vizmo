#ifndef CFG_ORACLE_H_
#define CFG_ORACLE_H_

#include "OracleStrategy.h"

#include "Models/Vizmo.h"

template<class MPTraits>
class CfgOracle : public OracleStrategy<MPTraits> {
  public:
    typedef typename MPTraits::MPProblemType MPProblemType;

    CfgOracle(const string& _inputCfgs = "", const string& _strategy = "");
    CfgOracle(MPProblemType* _problem, XMLNode& _node);

    void Initialize();

  private:
    string m_inputCfgs;
};

template<class MPTraits>
CfgOracle<MPTraits>::
CfgOracle(const string& _inputCfgs, const string& _strategy) :
  OracleStrategy<MPTraits>(_strategy), m_inputCfgs(_inputCfgs) {
  this->SetName("CfgOracle");
}

template<class MPTraits>
CfgOracle<MPTraits>::
CfgOracle(MPProblemType* _problem, XMLNode& _node) :
  OracleStrategy<MPTraits>(_problem, _node) {
    this->SetName("CfgOracle");
    m_inputCfgs = _node.Read("cfgFile", false, "", "Cfg Map Filename");
  }

template<class MPTraits>
void
CfgOracle<MPTraits>::
Initialize() {
  OracleStrategy<MPTraits>::Initialize();

  if(!m_inputCfgs.empty()) {
    auto r = this->GetRoadmap();
    r->Read(m_inputCfgs);
    cout << "Input Map: " << m_inputCfgs << endl;
    GetVizmo().GetMap()->RefreshMap();
  }
}

#endif
