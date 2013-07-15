#ifndef ENVMODEL_H_
#define ENVMODEL_H_

#include "Plum/GLModel.h"
#include "MultiBodyModel.h"
#include "EnvObj/BoundaryModel.h"

#include "RobotInfo.h"
#include <graph.h>
#include <algorithms/graph_input_output.h>
#include <algorithms/connected_components.h>

namespace plum{

  class EnvModel : public GLModel {
    public:
      //EnvModel(const string& _filename); 
      EnvModel(); 
      ~EnvModel(); 

      //Load functions 
      virtual bool ParseFile();
      virtual void SetModelDataDir(const string _modelDataDir);
      virtual int GetNumberOfMultiBody() const{ return m_cNumberOfMultiBody; }
      virtual const CMultiBodyInfo* GetMultiBodyInfo() const { return m_pMBInfo; }
      string GetModelDirString(){ return  m_strModelDataDir; }
      int GetDOF(){ return DoF; }
      vector<MultiBodyModel*> GetMBody(){ return m_pMBModel; }  
      vector<Robot>& GetRobots(){ return robotVec; }
      BoundaryModel* GetBoundaryModel(){ return m_boundary; } 
      void DecreaseNumMB(){ m_cNumberOfMultiBody = m_cNumberOfMultiBody - 1; }
      void IncreaseNumMB(){ m_cNumberOfMultiBody = m_cNumberOfMultiBody + 1; }
      
      bool FileExists() const;
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
      
      double GetRadius() const { return m_R; }
      const Point3d& GetCOM() const { return m_COM; }
      void ChangeColor(); //changes object's color randomly
      vector<vector<PolyhedronModel> > GetPoly();

      void DeleteMBModel(MultiBodyModel* _mbl);
      void AddMBModel(CMultiBodyInfo _newMBI);

      //////////////////////////////////
      // TESTING to save Env. file
      /////////////////////////////////
      bool SaveFile(const char* _filename);

      //////////////////////////////////////////////////////////////////////
      // Action functions
      //////////////////////////////////////////////////////////////////////

      virtual bool BuildModels();
      virtual void Draw(GLenum _mode);
      virtual void Select(unsigned int* _index, vector<gliObj>& _sel);
      virtual const string GetName() const { return "Environment"; }
      virtual void GetChildren( list<GLModel*>& models ){ 
        typedef vector<MultiBodyModel *>::iterator MIT;
        for(MIT i=m_pMBModel.begin();i!=m_pMBModel.end();i++){
          if((*i)->IsFixed())
            models.push_back(*i);
        }
        models.push_back(m_boundary);
      }
      virtual vector<string> GetInfo() const;

      BoundaryModel* GetBoundary() {return m_boundary;}

    private:
      string m_strModelDataDir;
      float m_color[3]; 
      int m_cNumberOfMultiBody;
      CMultiBodyInfo* m_pMBInfo;
      bool m_ContainsSurfaces;
      typedef stapl::graph<stapl::UNDIRECTED, stapl::NONMULTIEDGES, size_t> RobotGraph;
      RobotGraph m_robotGraph;
      vector<Robot> robotVec;
      int DoF; 
      
      vector<MultiBodyModel *> m_pMBModel; //an array of MultiBodyModel
      double m_R;    //radius
      Point3d m_COM; //center of mass
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

}//namespace plum

#endif // !defined(_ENVMODEL_H_)
