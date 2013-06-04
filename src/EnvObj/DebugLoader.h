// PathLoader.h: interface for the CDebugLoader class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DEBUG_LOADER_H_
#define _DEBUG_LOADER_H_

#include <vector>
using namespace std;

#include "MapObj/Cfg.h"
#include "MapObj/MapLoader.h"
#include "MapObj/MapModel.h"
#include <ILoadable.h>
using namespace plum;

struct Instruction{
  virtual ~Instruction(){};
  string name;
  virtual void Print(ostream& oss)=0;
};

struct Default : public Instruction{
  Default(){name = "default";}
  virtual void Print(ostream& oss){
    oss<<name<<endl;
  }
};

struct AddNode : public Instruction {
  AddNode(CCfg c):cfg(c){name = "AddNode";};
  CCfg cfg;
  virtual void Print(ostream& oss){
    oss<<name<<" "<<cfg<<endl;
  }
};

struct AddEdge : public Instruction {
  float target_color[3], source_color[3];
  AddEdge(CCfg s, CCfg t):source(s), target(t){name = "AddEdge";};
  CCfg source, target;
  virtual void Print(ostream& oss){
    oss<<name<<"\tsource: "<<source<<"\ttarget: "<<target<<endl;
  }
};

struct AddTempCfg : public Instruction {
  AddTempCfg(CCfg c, bool v):cfg(c), valid(v){name = "AddTempCfg";};
  CCfg cfg;
  bool valid;
  virtual void Print(ostream& oss){
    oss<<name<<" "<<cfg<<" Valid::"<<valid<<endl;
  }
};

struct AddTempRay : public Instruction {
  AddTempRay(CCfg c):cfg(c){name = "AddTempRay";};
  CCfg cfg;
  virtual void Print(ostream& oss){
    oss<<name<<" "<<cfg<<endl;
  }
};

struct AddTempEdge : public Instruction {
  AddTempEdge(CCfg s, CCfg t):source(s), target(t){name = "AddTempEdge";};
  CCfg source, target;
  virtual void Print(ostream& oss){
    oss<<name<<"\tsource: "<<source<<"\ttarget: "<<target<<endl;
  }
};

struct ClearLastTemp : public Instruction {
  CCfg* tempRay;
  vector<CCfg> lastTempCfgs;
  vector<Edge> lastTempEdges;

  ClearLastTemp(){name = "ClearLastTemp";}
  virtual void Print(ostream& oss){
    oss<<name<<endl;
  }
};

struct ClearAll : public Instruction {
  CCfg* tempRay;
  vector<CCfg> tempCfgs;
  vector<Edge> tempEdges;
  vector<Edge> query;
  vector<string> comments;
  
  
  ClearAll(){name = "ClearAll";}
  virtual void Print(ostream& oss){
    oss<<name<<endl;
  }
};

struct ClearComments : public Instruction {
  vector<string> comments;
  ClearComments(){name = "ClearComments";}
  virtual void Print(ostream& oss){
    oss<<name<<endl;
  }
};

struct RemoveNode : public Instruction {
  RemoveNode(CCfg c):cfg(c){name = "RemoveNode";}
  CCfg cfg;
  virtual void Print(ostream& oss){
    oss<<name<<" "<<cfg<<endl;
  }
};

struct RemoveEdge : public Instruction {
  int edgeNum;
  RemoveEdge(CCfg s, CCfg t):source(s), target(t){name = "RemoveEdge";};
  CCfg source, target;
  virtual void Print(ostream& oss){
    oss<<name<<"\tsource: "<<source<<"\ttarget: "<<target<<endl;
  }
};

struct Comment : public Instruction {
  Comment(string s):comment(s){name = "Comment";}
  string comment;
  virtual void Print(ostream& oss){
    oss<<name<<" "<<comment<<endl;
  }
};

struct Query : public Instruction {
  vector<Edge> query;
  Query(CCfg s, CCfg t):source(s), target(t){name = "Query";};
  CCfg source, target;
  virtual void Print(ostream& oss){
    oss<<name<<"\tsource: "<<source<<"\ttarget: "<<target<<endl;
  }
};



/**
* This class reads path. OBPRM style path.
* Client how wants to retrive path could call LoadPath to
* read path form given file name and get individal cfg from 
* GetNextConfigure.
*/
typedef Instruction* FRAME;
class CDebugLoader : public I_Loadable
{
public:
    //////////////////////////////////////////////////////////////////////
    //    Constructors and Destructor
    ///////////////////////////////////////////////////////////////////////
    CDebugLoader();
    virtual ~CDebugLoader();
    virtual bool ParseFile();
    
    /////////////////////////x/////////////////////////////////////////////
    //    Access Methods
    ///////////////////////////////////////////////////////////////////////
    //Get a particular configuration
    virtual FRAME ConfigureFrame(int frame);
    virtual unsigned int GetDebugSize(){ return m_iList.size(); }

    //////////////////////////////////////////////////////////////////////
    //    Protected Methods and Data
    ///////////////////////////////////////////////////////////////////////
protected:
    
    ///Free memory
    virtual void FreeDebugList();
    virtual FRAME GetFrame(bool bOutputFrameNumber, int index );

private:
    
    vector<FRAME> m_iList;       ///instruction storage
};

#endif // !defined(_PATH_LOADER_H_)


