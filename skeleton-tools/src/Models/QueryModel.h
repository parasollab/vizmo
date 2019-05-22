#ifndef QUERY_MODEL_H_
#define QUERY_MODEL_H_

#include "Model.h"
#include "CfgModel.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief Graphic representation of a PMPL query.
////////////////////////////////////////////////////////////////////////////////
class QueryModel : public LoadableModel {

  public:

    ///\name Construction
    ///@{

    QueryModel(const string& _filename);
    ~QueryModel();

    ///@}
    ///\name Interface
    ///@{

    size_t GetQuerySize() {return m_cfgs.size();}
    CfgModel& GetQueryCfg(size_t _i) {return m_cfgs[_i];}

    void AddCfg(int _num);
    void SwapUp(size_t _i) {swap(m_cfgs[_i], m_cfgs[_i-1]);}
    void SwapDown(size_t _i) {swap(m_cfgs[_i], m_cfgs[_i+1]);}
    void DeleteQuery(size_t _i) {m_cfgs.erase(m_cfgs.begin()+_i);}
    void SaveQuery(const string& _filename);

    ///@}
    ///\name Model Overrides
    ///@{

    void Build();
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void DrawRender();
    void DrawSelect();
    void DrawSelected() {}
    void Print(ostream& _os) const;

    ///\name LoadableModel Overrides
    ///@{

    virtual void ParseFile() override;

    ///@}

  private:

    ///\name Internal State
    ///@{

    vector<CfgModel> m_cfgs;           ///< Query points.
    size_t m_glQueryIndex{(size_t)-1}; ///< Display list index.

    ///@}
};

#endif
