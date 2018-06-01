#ifndef EDGE_MODEL_H_
#define EDGE_MODEL_H_

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <GL/gl.h>

#include "CfgModel.h"
#include "Model.h"

class CfgModel;

#ifdef PMPCfg
#include "MPProblem/Weight.h"
typedef DefaultWeight<CfgModel> EdgeType;
#elif defined(PMPState)
#include "Edges/StateEdge.h"
typedef StateEdge<CfgModel> EdgeType;
#endif

////////////////////////////////////////////////////////////////////////////////
/// \brief A drawable model for roadmap edges.
////////////////////////////////////////////////////////////////////////////////
class EdgeModel : public Model, public EdgeType {

  public:

    // Construction
    EdgeModel(string _lpLabel = "", double _weight = 1,
        const vector<CfgModel>& _intermediates = vector<CfgModel>());
    EdgeModel(const DefaultWeight<CfgModel>& _e);


    CfgModel* GetStartCfg() {return m_startCfg;} ///< Get the source vertex.
    CfgModel* GetEndCfg() {return m_endCfg;}     ///< Get the target vertex.

    vector<double> GetControl() {return vector<double>();}

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the name for this edge.
    void SetName();
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the source and target configurations for this edge.
    /// \param[in] _c1 The source configuration.
    /// \param[in] _c1 The target configuration.
    void Set(CfgModel* _c1, CfgModel* _c2);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set id, name, and source and target configurations for this edge.
    /// \param[in] _c1 The source configuration.
    /// \param[in] _c1 The target configuration.
    void Set(size_t _id, CfgModel* _c1, CfgModel* _c2);
    ////////////////////////////////////////////////////////////////////////////
    // \brief Calculate intermediates of edge when the edge is set in Vizmo
    // \param[in] _c A configuration in the edge
    void RecalculateEdges(CfgModel _c);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Get the EID of this edge.
    size_t GetID() {return m_id;}
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the validity of this edge.
    void SetValidity(bool _validity) { m_isValid = _validity; }
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Query the validity of this edge.
    bool IsValid() {return m_isValid;}

    // Model functions
    void Build() {}
    void Select(GLuint* _index, vector<Model*>& _sel) {};
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    void Print(ostream& _os) const;

    void DrawRenderInCC();

    // Class properties
    static double m_edgeThickness; ///< Rendering thickness for edge lines.

  private:

    CfgModel* m_startCfg; ///< Points to the source vertex.
    CfgModel* m_endCfg;   ///< Points to the target vertex.
    size_t m_id;          ///< This edge's EID in the RoadmapGraph.
    bool m_isValid;       ///< Indicates whether this edge is valid.
};

#endif


















