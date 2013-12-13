#ifndef DEBUGMODEL_H_
#define DEBUGMODEL_H_

#include "CfgModel.h"
#include "MapModel.h"
#include "Model.h"

////////////////////////////////////////////////////////////////////////////////
// Instruction classes for the Debug model
//
// Instruction - base class
// AddNode - Adds a node to the debug map model
// AddEdge - Adds an edge to the debug map model
// AddTempCfg - Adds a temporary robot in the scene of either red or black color
// AddTempRay - Adds a directional ray from the previous temporary Cfg
// AddTempEdge - Adds a tempory edge in the scene
// ClearLastTemp - Removes the last temporary instruction called
// ClearAll - Removes all temporaries and comments
// ClearComments - Removes all comments
// RemoveNode - Removes a node from the debug map model
// RemoveEdge - Removes an edge from the debug map model
// Comment - Adds a string comment to the text box in the window
// Query - If a path can be found, it highlights a path in the scene
////////////////////////////////////////////////////////////////////////////////
struct Instruction{
  Instruction(const string& _name = "default") : m_name(_name) {}
  string m_name;
};

struct AddNode : public Instruction {
  AddNode(const CfgModel& _c) : Instruction("AddNode"), m_cfg(_c) {}
  CfgModel m_cfg;
};

struct AddEdge : public Instruction {
  AddEdge(const CfgModel& _s, const CfgModel& _t) : Instruction("AddEdge"), m_source(_s), m_target(_t) {}
  Color4 m_targetColor, m_sourceColor;
  CfgModel m_source, m_target;
};

struct AddTempCfg : public Instruction {
  AddTempCfg(const CfgModel& _c, bool _v) : Instruction("AddTempCfg"), m_cfg(_c), m_valid(_v) {}
  CfgModel m_cfg;
  bool m_valid;
};

struct AddTempRay : public Instruction {
  AddTempRay(const CfgModel& _c) : Instruction("AddTempRay"), m_cfg(_c) {}
  CfgModel m_cfg;
};

struct AddTempEdge : public Instruction {
  AddTempEdge(const CfgModel& _s, const CfgModel& _t) : Instruction("AddTempEdge"), m_source(_s), m_target(_t) {}
  CfgModel m_source, m_target;
};

struct ClearLastTemp : public Instruction {
  ClearLastTemp() : Instruction("ClearLastTemp"), m_tempRay(NULL) {}
  CfgModel* m_tempRay;
  vector<CfgModel> m_lastTempCfgs;
  vector<EdgeModel> m_lastTempEdges;
};

struct ClearAll : public Instruction {
  ClearAll() : Instruction("ClearAll"), m_tempRay(NULL) {}
  CfgModel* m_tempRay;
  vector<CfgModel> m_tempCfgs;
  vector<EdgeModel> m_tempEdges;
  vector<EdgeModel> m_query;
  vector<string> m_comments;
};

struct ClearComments : public Instruction {
  ClearComments() : Instruction("ClearComments") {}
  vector<string> m_comments;
};

struct RemoveNode : public Instruction {
  RemoveNode(const CfgModel& _c) : Instruction("RemoveNode"), m_cfg(_c) {}
  CfgModel m_cfg;
};

struct RemoveEdge : public Instruction {
  RemoveEdge(const CfgModel& _s, const CfgModel& _t) : Instruction("RemoveEdge"), m_edgeNum(-1), m_source(_s), m_target(_t) {}
  int m_edgeNum;
  CfgModel m_source, m_target;
};

struct Comment : public Instruction {
  Comment(const string& _s) : Instruction("Comment"), m_comment(_s) {}
  string m_comment;
};

struct Query : public Instruction {
  Query(const CfgModel& _s, const CfgModel& _t) : Instruction("Query"), m_source(_s), m_target(_t) {}
  CfgModel m_source, m_target;
  vector<EdgeModel> m_query;
};

////////////////////////////////////////////////////////////////////////////////
// Debug Model
//
// Stores a map model and temporary vectors of objects to display the debug at
// any given frame
////////////////////////////////////////////////////////////////////////////////
class DebugModel : public LoadableModel {
  public:
    typedef MapModel<CfgModel, EdgeModel> MM;

    DebugModel(const string& _filename);
    virtual ~DebugModel();

    size_t GetSize(){ return m_instructions.size(); }
    vector<string> GetComments();
    MM* GetMapModel() {return m_mapModel;}

    void ParseFile();
    void BuildModels();
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void Draw();
    void DrawSelect() {}
    void Print(ostream& _os) const;

    void ConfigureFrame(int _frame);
    void BuildForward();
    void BuildBackward();

  private:
    //instructions
    vector<Instruction*> m_instructions;

    //indices for frame construction
    int m_index;
    int m_prevIndex;

    //debug model components
    MM* m_mapModel;
    int m_edgeNum;
    vector<CfgModel> m_tempCfgs;
    vector<EdgeModel> m_tempEdges, m_query;
    CfgModel* m_tempRay;
    vector<string> m_comments;
};

#endif
