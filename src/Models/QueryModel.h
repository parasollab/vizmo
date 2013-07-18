#ifndef QUERY_H_
#define QUERY_H_

#include "Plum/GLModel.h"
using namespace plum;

class OBPRMView_Robot;

class QueryModel : public GLModel {
  public:
    QueryModel(string _filename);
    virtual ~QueryModel();

    virtual const string GetName() const { return "Query"; }
    virtual vector<string> GetInfo() const;
    size_t GetQuerySize() const {return m_queries.size();}
    const vector<double>& GetStartGoal(size_t _i) {return m_queries[_i];}

    void SetModel(OBPRMView_Robot* _robotModel){m_robotModel = _robotModel;}

    virtual bool ParseFile();
    virtual bool BuildModels();
    virtual void Draw(GLenum _mode);

  private:
    vector<vector<double> > m_queries; //vector of queries
    size_t m_glQueryIndex; //Display list index
    OBPRMView_Robot * m_robotModel; //robot model
};

#endif
