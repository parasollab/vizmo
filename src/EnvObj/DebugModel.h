// DebugModel.h: interface for the DebugModel class.
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

class DebugModel : public CGLModel {
  public:
    typedef CMapModel<CCfg, Edge> MapModel;

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////
    DebugModel();
    virtual ~DebugModel();
    
    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    void SetDebugLoader( CDebugLoader* _debugLoader ){ m_debugLoader = _debugLoader; }
    void SetModel( OBPRMView_Robot* _robot ){ m_robot = _robot; }
    
    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    virtual bool BuildModels();
    virtual void BuildForward();
    virtual void BuildBackward();
    virtual void Draw(GLenum _mode);
    virtual const string GetName() const { return "Debug"; }
    virtual vector<string> GetInfo() const;
    vector<string> GetComments();  

    MapModel* GetMapModel(){return m_mapModel;}

    void ConfigureFrame(int _frame);//{m_index = f;}

    //////////////////////////////////////////////////////////////////////
    // Private functions and data
    //////////////////////////////////////////////////////////////////////
  private:
    int m_index;
    int m_prevIndex;
    CDebugLoader* m_debugLoader;
    OBPRMView_Robot* m_robot;
    CMapLoader<CCfg,Edge>* m_mapLoader;

    int m_edgeNum;
    MapModel* m_mapModel;
    vector<CCfg> m_tempCfgs;
    vector<Edge> m_tempEdges, m_query;
    CCfg* m_tempRay;
    vector<string> m_comments;

};

#endif // !defined(_PATHMODEL_H_)
