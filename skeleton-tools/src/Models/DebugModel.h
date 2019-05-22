#ifndef DEBUG_MODEL_H_
#define DEBUG_MODEL_H_

#include "CfgModel.h"
#include "MapModel.h"
#include "Model.h"

class RegionModel;

////////////////////////////////////////////////////////////////////////////////
/// \brief   Describes a set of instructions for use by the debug model.
/// \details Instructions capture the actions of a planner in sequence, and are
///          used to replay the sequence of events that occur during a given
///          execution.
////////////////////////////////////////////////////////////////////////////////
namespace DebugInstructions {

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Base class for debug instructions.
  //////////////////////////////////////////////////////////////////////////////
  struct Instruction {
    Instruction(const string& _name = "default") : m_name(_name) {}
    virtual ~Instruction() {}
    string m_name;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Adds a node to the debug map model.
  //////////////////////////////////////////////////////////////////////////////
  struct AddNode : public Instruction {
    AddNode(const CfgModel& _c) : Instruction("AddNode"), m_cfg(_c) {}
    CfgModel m_cfg;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Adds an edge to the debug map model.
  //////////////////////////////////////////////////////////////////////////////
  struct AddEdge : public Instruction {
    AddEdge(const CfgModel& _s, const CfgModel& _t) : Instruction("AddEdge"),
        m_source(_s), m_target(_t) {}
    Color4 m_targetColor, m_sourceColor;
    CfgModel m_source, m_target;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Adds a temporary Cfg in the scene of either red or black color.
  //////////////////////////////////////////////////////////////////////////////
  struct AddTempCfg : public Instruction {
    AddTempCfg(const CfgModel& _c, bool _v) : Instruction("AddTempCfg"),
        m_cfg(_c), m_valid(_v) {}
    CfgModel m_cfg;
    bool m_valid;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Adds a directional ray from the previous temporary Cfg.
  //////////////////////////////////////////////////////////////////////////////
  struct AddTempRay : public Instruction {
    AddTempRay(const CfgModel& _c) : Instruction("AddTempRay"), m_cfg(_c) {}
    CfgModel m_cfg;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Adds a tempory edge in the scene.
  //////////////////////////////////////////////////////////////////////////////
  struct AddTempEdge : public Instruction {
    AddTempEdge(const CfgModel& _s, const CfgModel& _t) :
        Instruction("AddTempEdge"), m_source(_s), m_target(_t) {}
    CfgModel m_source, m_target;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Removes the last temporary instruction called.
  //////////////////////////////////////////////////////////////////////////////
  struct ClearLastTemp : public Instruction {
    ClearLastTemp() : Instruction("ClearLastTemp"), m_tempRay(NULL) {}
    CfgModel* m_tempRay;
    vector<CfgModel> m_lastTempCfgs;
    vector<EdgeModel> m_lastTempEdges;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Removes all temporaries and comments.
  //////////////////////////////////////////////////////////////////////////////
  struct ClearAll : public Instruction {
    ClearAll() : Instruction("ClearAll"), m_tempRay(NULL) {}
    CfgModel* m_tempRay;
    vector<CfgModel> m_tempCfgs;
    vector<EdgeModel> m_tempEdges;
    vector<EdgeModel> m_query;
    vector<string> m_comments;
    vector<RegionModel*> m_regions;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Removes all comments.
  //////////////////////////////////////////////////////////////////////////////
  struct ClearComments : public Instruction {
    ClearComments() : Instruction("ClearComments") {}
    vector<string> m_comments;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Removes a node from the debug map model.
  //////////////////////////////////////////////////////////////////////////////
  struct RemoveNode : public Instruction {
    RemoveNode(const CfgModel& _c) : Instruction("RemoveNode"), m_cfg(_c) {}
    CfgModel m_cfg;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Removes an edge from the debug map model.
  //////////////////////////////////////////////////////////////////////////////
  struct RemoveEdge : public Instruction {
    RemoveEdge(const CfgModel& _s, const CfgModel& _t) :
        Instruction("RemoveEdge"), m_edgeNum(-1), m_source(_s), m_target(_t) {}
    int m_edgeNum;
    CfgModel m_source, m_target;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Adds a string comment to the text box in the window.
  //////////////////////////////////////////////////////////////////////////////
  struct Comment : public Instruction {
    Comment(const string& _s) : Instruction("Comment"), m_comment(_s) {}
    string m_comment;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Highlights a path in the scene if one can be found.
  //////////////////////////////////////////////////////////////////////////////
  struct QueryInstruction : public Instruction {
    QueryInstruction(const CfgModel& _s, const CfgModel& _t) :
        Instruction("QueryInstruction"), m_source(_s), m_target(_t) {}
    CfgModel m_source, m_target;
    vector<EdgeModel> m_query;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Adds a user-guidance region to the scene.
  //////////////////////////////////////////////////////////////////////////////
  struct AddRegion : public Instruction {
    AddRegion(RegionModel* _mod) : Instruction("AddRegion"),
        m_regionModel(_mod) {}
    RegionModel* m_regionModel;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Removes a user-guidance region to the scene.
  //////////////////////////////////////////////////////////////////////////////
  struct RemoveRegion : public Instruction {
    RemoveRegion(RegionModel* _mod) : Instruction("RemoveRegion"),
        m_regionModel(_mod) {}
    RegionModel* m_regionModel;
  };
}


////////////////////////////////////////////////////////////////////////////////
/// \brief Stores a map model and temporary vectors of objects to display the
///        debug at any given frame.
////////////////////////////////////////////////////////////////////////////////
class DebugModel : public LoadableModel {

  public:

    typedef MapModel<CfgModel, EdgeModel> MM;

    // Construction
    DebugModel(const string& _filename);
    ~DebugModel();

    size_t GetSize() {return m_instructions.size();}
    vector<string> GetComments();
    MM* GetMapModel() {return m_mapModel;}

    // Model functions
    void ParseFile();
    void Build();
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void DrawRender();
    void DrawSelect() {}
    void DrawSelected() {}
    void Print(ostream& _os) const;

    void ConfigureFrame(int _frame);
    void BuildForward();
    void BuildBackward();

  private:

    //instructions
    vector<DebugInstructions::Instruction*> m_instructions;

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
    vector<RegionModel*> m_regions;
};

#endif
