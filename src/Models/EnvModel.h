#ifndef ENVMODEL_H_
#define ENVMODEL_H_

#include "Plum/GLModel.h"
#include "Plum/EnvObj/MultiBodyModel.h"
#include "Models/BoundaryModel.h"
#include "Plum/EnvObj/RobotInfo.h"

#include <graph.h>
#include <algorithms/graph_input_output.h>
#include <algorithms/connected_components.h>

class EnvModel : public GLModel {

  public:
    EnvModel(const string& _filename);
    ~EnvModel();

    //Access functions
    virtual const string GetName() const { return "Environment"; }
    int GetNumMultiBodies() const{ return m_numMultiBodies; }
    const MultiBodyInfo* GetMultiBodyInfo() const { return m_mbInfo; }
    string GetModelDataDir(){ return  m_modelDataDir; }
    int GetDOF(){ return m_dof; }
    vector<MultiBodyModel*> GetMultiBodies(){ return m_mbModels; }
    vector<Robot>& GetRobots(){ return m_robots; }
    BoundaryModel* GetBoundary() {return m_boundary;}
    double GetRadius() const { return m_radius; }
    const Point3d& GetCOM() const { return m_centerOfMass; }

    //Load functions
    virtual void ParseFile();
    void SetModelDataDir(const string _modelDataDir);
    void DecreaseNumMB(){ m_numMultiBodies = m_numMultiBodies - 1; }
    void IncreaseNumMB(){ m_numMultiBodies = m_numMultiBodies + 1; }
    void GetColor(istream& _is);
    void SetNewMultiBodyInfo(MultiBodyInfo* _mbi);
    void NewModelDir();
    void FreeMemory();
    void ParseFileHeader(ifstream& _ifs);
    void ParseBoundary(ifstream& _ifs);
    void ParseFileBody(ifstream& _ifs);
    void ParseMultiBody(ifstream& _ifs, MultiBodyInfo& _mbInfo);
    void ParseActiveBody(ifstream& _ifs, BodyModel& _bodyInfo);
    void ParseOtherBody(ifstream& _ifs, BodyModel& _bodyInfo);
    void ParseConnections(ifstream& _ifs, MultiBodyInfo& _mbInfo);
    void BuildRobotStructure();

    //Display functions
    void ChangeColor(); //changes object's color randomly
    vector<vector<PolyhedronModel> > GetMBPolyLists();
    void DeleteMBModel(MultiBodyModel* _mbl);
    void AddMBModel(MultiBodyInfo _newMBI);

    // TESTING to save Env. file
    bool SaveFile(const char* _filename);

    virtual void BuildModels();
    virtual void Draw(GLenum _mode);
    virtual void Select(unsigned int* _index, vector<GLModel*>& _sel);
    virtual void GetChildren(list<GLModel*>& _models);
    virtual vector<string> GetInfo() const;

  private:
    string m_modelDataDir;
    float m_color[3];
    int m_numMultiBodies;
    MultiBodyInfo* m_mbInfo;
    bool m_containsSurfaces;
    typedef stapl::graph<stapl::UNDIRECTED, stapl::NONMULTIEDGES, size_t> RobotGraph;
    RobotGraph m_robotGraph;
    vector<Robot> m_robots;
    int m_dof;
    vector<MultiBodyModel *> m_mbModels;
    double m_radius;
    Point3d m_centerOfMass;
    BoundaryModel* m_boundary;
};

#endif
