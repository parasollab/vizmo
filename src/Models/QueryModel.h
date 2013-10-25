#ifndef QUERY_H_
#define QUERY_H_

#include "Model.h"

class RobotModel;

class QueryModel : public LoadableModel {
  public:
    QueryModel(const string& _filename, RobotModel* _robotModel);
    ~QueryModel();

    size_t GetQuerySize() const {return m_queries.size();}
    const vector<double>& GetStartGoal(size_t _i) {return m_queries[_i];}

    void ParseFile();
    void BuildModels();
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void Draw(GLenum _mode);
    void DrawSelect() {}
    void Print(ostream& _os) const;

    void SaveQuery(const string& _filename);

  private:
    vector<vector<double> > m_queries; //vector of queries
    size_t m_glQueryIndex; //Display list index
    RobotModel* m_robotModel; //robot model
};

#endif
