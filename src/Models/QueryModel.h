#ifndef QUERY_H_
#define QUERY_H_

#include "Model.h"
#include "CfgModel.h"

class RobotModel;

class QueryModel : public LoadableModel {
  public:
    QueryModel(const string& _filename);
    ~QueryModel();

    size_t GetQuerySize() {return m_cfgs.size();}
    CfgModel& GetQueryCfg(size_t _i) {return m_cfgs[_i];}

    void AddCfg(int _num);
    void SwapUp(size_t _i) {swap(m_cfgs[_i], m_cfgs[_i-1]);}
    void SwapDown(size_t _i) {swap(m_cfgs[_i], m_cfgs[_i+1]);}
    void DeleteQuery(size_t _i) {m_cfgs.erase(m_cfgs.begin()+_i);}

    void ParseFile();
    void BuildModels();
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void Draw();
    void DrawSelect() {}
    void Print(ostream& _os) const;

    void SaveQuery(const string& _filename);

  private:
    vector<CfgModel> m_cfgs; //query points
    size_t m_glQueryIndex; //Display list index
};

#endif
