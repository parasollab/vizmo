// PathModel.h: interface for the CDebugModel class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DEBUGMODEL_H_
#define _DEBUGMODEL_H_

//////////////////////////////////////////////////////////////////////
//std Headers
#include <math.h>

//////////////////////////////////////////////////////////////////////
//Plum Headers
#include <GLModel.h>
using namespace plum;

//////////////////////////////////////////////////////////////////////
//OBPRMView Headers
#include "DebugLoader.h"
class OBPRMView_Robot;

class CDebugModel : public CGLModel
{
public:
    typedef CMapModel<CCfg, Edge> MapModel;
    typedef Edge EDGE;
    struct DModel{
      DModel(MapModel* m, vector<CCfg> v, vector<EDGE> e, vector<EDGE> q, CCfg* r, vector<string> c):
        mapModel(m), tempCfgs(v), tempEdges(e), query(q), tempRay(r), comments(c){}
      MapModel* mapModel;
      vector<CCfg> tempCfgs;
      vector<EDGE> tempEdges, query;
      CCfg* tempRay;
      vector<string> comments;
    };
    typedef DModel Model;

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////
    CDebugModel();
    virtual ~CDebugModel();
    
    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    void SetDebugLoader( CDebugLoader * pDebugLoader ){ m_pDebugLoader=pDebugLoader; }
    void SetModel( OBPRMView_Robot * pRobot ){ m_pRobot=pRobot; }
    
    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    virtual bool BuildModels();
    virtual void Draw( GLenum mode );
    virtual const string GetName() const { return "Debug"; }
    virtual list<string> GetInfo() const;

    void ConfigureFrame(int f);//{m_Index = f;}

    vector<Model>& GetModels() {return m_mapModels;}
    //output info to std ouput
    //virtual void DumpSelected();
    
    //////////////////////////////////////////////////////////////////////
    // Private functions and data
    //////////////////////////////////////////////////////////////////////
private:
    size_t m_Index;
    CDebugLoader * m_pDebugLoader;
    OBPRMView_Robot * m_pRobot;
    CMapLoader<CCfg,Edge>* m_mapLoader;
    vector<Model> m_mapModels;
};

#endif // !defined(_PATHMODEL_H_)
