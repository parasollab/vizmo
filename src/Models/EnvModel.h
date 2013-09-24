#ifndef ENVMODEL_H_
#define ENVMODEL_H_

#include "Plum/GLModel.h"
#include "Plum/EnvObj/MultiBodyModel.h"
#include "Models/BoundaryModel.h"
#include "Models/RobotInfo.h"

#include <graph.h>
#include <algorithms/graph_input_output.h>
#include <algorithms/connected_components.h>

class EnvModel : public GLModel {

  public:
    EnvModel(const string& _filename);
    ~EnvModel();

    //Access functions
    virtual const string GetName() const { return "Environment"; }
    string GetModelDataDir(){ return  m_modelDataDir; }
    int GetDOF(){ return m_dof; }
    vector<MultiBodyModel*> GetMultiBodies(){ return m_multibodies; }
    vector<Robot>& GetRobots(){ return m_robots; }
    BoundaryModel* GetBoundary() {return m_boundary;}
    double GetRadius() const { return m_radius; }
    const Point3d& GetCOM() const { return m_centerOfMass; }

    //Load functions
    virtual void ParseFile();
    void SetModelDataDir(const string _modelDataDir);
    void NewModelDir();
    void ParseBoundary(ifstream& _ifs);
    void BuildRobotStructure();

    //Display functions
    void ChangeColor(); //changes object's color randomly
    void DeleteMBModel(MultiBodyModel* _mbl);
    void AddMBModel(MultiBodyModel* _newMBI);

    // TESTING to save Env. file
    bool SaveFile(const char* _filename);

    virtual void BuildModels();
    virtual void Draw(GLenum _mode);
    virtual void Select(unsigned int* _index, vector<GLModel*>& _sel);
    virtual void GetChildren(list<GLModel*>& _models);
    virtual vector<string> GetInfo() const;

  private:
    string m_modelDataDir;
    bool m_containsSurfaces;

    typedef stapl::graph<stapl::UNDIRECTED, stapl::NONMULTIEDGES, size_t> RobotGraph;
    RobotGraph m_robotGraph;
    vector<Robot> m_robots;

    int m_dof;

    vector<MultiBodyModel*> m_multibodies;

    double m_radius;
    Point3d m_centerOfMass;
    BoundaryModel* m_boundary;
};

#endif
