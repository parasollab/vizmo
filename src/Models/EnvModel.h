#ifndef ENVMODEL_H_
#define ENVMODEL_H_

#include "Plum/GLModel.h"
#include "Plum/EnvObj/MultiBodyModel.h"
#include "Models/BoundaryModel.h"
#include "Plum/EnvObj/RobotInfo.h"

#include <graph.h>
#include <algorithms/graph_input_output.h>
#include <algorithms/connected_components.h>


class EnvModel : public plum::GLModel {
  
  public:
    EnvModel(); 
    ~EnvModel(); 

    //Access functions 
    virtual const string GetName() const { return "Environment"; }
    virtual int GetNumMultiBodies() const{ return m_numMultiBodies; }
    virtual const CMultiBodyInfo* GetMultiBodyInfo() const { return m_mBInfo; }
    string GetModelDataDir(){ return  m_modelDataDir; }
    int GetDOF(){ return m_dof; }
    vector<MultiBodyModel*> GetMultiBodies(){ return m_mBModels; }  
    vector<Robot>& GetRobots(){ return m_robots; }
    BoundaryModel* GetBoundary() {return m_boundary;}
    double GetRadius() const { return m_radius; }
    const Point3d& GetCOM() const { return m_centerOfMass; }


    //Load functions 
    virtual bool ParseFile();
    virtual void SetModelDataDir(const string _modelDataDir);
    void DecreaseNumMB(){ m_numMultiBodies = m_numMultiBodies - 1; }
    void IncreaseNumMB(){ m_numMultiBodies = m_numMultiBodies + 1; }
    string ReadFieldString(istream& _is, string _error, bool _toUpper = true); 
    void GetColor(istream& _in); 
    bool IsCommentLine(char _c); 
    void SetNewMultiBodyInfo(CMultiBodyInfo* _mbi);
    void NewModelDir();
    void FreeMemory();
    virtual bool ParseFileHeader(ifstream& _ifs);
    virtual bool ParseBoundary(ifstream& _ifs);
    virtual bool ParseFileBody(ifstream& _ifs);
    virtual bool ParseMultiBody(ifstream& _ifs, CMultiBodyInfo& _mBInfo);
    virtual bool ParseActiveBody(ifstream& _ifs, CBodyInfo& _bodyInfo);
    virtual bool ParseOtherBody(ifstream& _ifs, CBodyInfo& _bodyInfo);
    virtual bool ParseConnections(ifstream& _ifs, CMultiBodyInfo& _mBInfo);
    void BuildRobotStructure(); 
    
    //Display functions 
    void ChangeColor(); //changes object's color randomly
    vector<vector<PolyhedronModel> > GetMBPolyLists();
    void DeleteMBModel(MultiBodyModel* _mbl);
    void AddMBModel(CMultiBodyInfo _newMBI);

    // TESTING to save Env. file
    bool SaveFile(const char* _filename);

    virtual bool BuildModels();
    virtual void Draw(GLenum _mode);
    virtual void Select(unsigned int* _index, vector<gliObj>& _sel);
    virtual void GetChildren(list<GLModel*>& _models);  
    virtual vector<string> GetInfo() const;

  private:
    string m_modelDataDir;
    float m_color[3]; 
    int m_numMultiBodies;
    CMultiBodyInfo* m_mBInfo;
    bool m_containsSurfaces;
    typedef stapl::graph<stapl::UNDIRECTED, stapl::NONMULTIEDGES, size_t> RobotGraph;
    RobotGraph m_robotGraph;
    vector<Robot> m_robots;
    int m_dof; 
    vector<MultiBodyModel *> m_mBModels;  
    double m_radius;    
    Point3d m_centerOfMass; 
    BoundaryModel* m_boundary;
};

template <class T>   
T ReadField(istream& _is, string _error){

  char c;
  string line;
  T element;
  
  while(_is){
    
    c = _is.peek();
    if(c == '#') 
      getline(_is, line);
    
    else if(isspace(c) == false){
      if (!(_is >> element)){
        cerr << "Error in Reading Field::" << _error << endl;
        exit(1);
      }   
      else
        break;
    }
    
    else
      _is.get(c);
  }
  
  if(_is.eof()){
    cerr << "Error end of file reached in Reading Field::" << _error << endl;
    exit(1);
  }
  return element;
}

#endif // !defined(_ENVMODEL_H_)
