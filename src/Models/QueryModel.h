#ifndef QUERY_H_
#define QUERY_H_

#include "GLModel.h"

class RobotModel;

class QueryModel : public GLModel {
  public:
    QueryModel(const string& _filename, RobotModel* _robotModel);
    virtual ~QueryModel();

    virtual const string GetName() const { return "Query"; }
    virtual vector<string> GetInfo() const;
    size_t GetQuerySize() const {return m_queries.size();}
    const vector<double>& GetStartGoal(size_t _i) {return m_queries[_i];}

    virtual void ParseFile();
    virtual void BuildModels();
    virtual void Draw(GLenum _mode);

  private:
    vector<vector<double> > m_queries; //vector of queries
    size_t m_glQueryIndex; //Display list index
    RobotModel * m_robotModel; //robot model
};

#endif
