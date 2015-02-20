#ifndef ROAD_MAP_OPTIONS_H_
#define ROAD_MAP_OPTIONS_H_

#include "OptionsBase.h"
#include "Models/MapModel.h"
#include "Models/Vizmo.h"

class SliderDialog;


////////////////////////////////////////////////////////////////////////////////
/// \brief Provides the "Roadmap" submenu and associated actions.
////////////////////////////////////////////////////////////////////////////////
class RoadmapOptions : public OptionsBase {

  Q_OBJECT

  public:

    typedef MapModel<CfgModel, EdgeModel> Map;
    typedef Map::Graph Graph;
    typedef Map::EI EI;
    typedef Map::VI VI;
    typedef Map::VID VID;
    typedef Map::EID EID;
    typedef vector<Model*>::iterator MIT;

    RoadmapOptions(QWidget* _parent);

  private slots:

    //roadmap functions
    void ShowRoadmap();         ///< Toggle roadmap display on or off.
    void ShowNodeSizeDialog();  ///< Show dialog for node size.
    void ScaleNodes();          ///< Change the rendering size of nodes.
    void ShowEdgeThicknessDialog(); ///< Show dialog for edge thickness.
    void ChangeEdgeThickness(); ///< Change the rendering thickness of edges.
    void ShowNodeEditDialog();  ///< Show the node edit dialog.
    void ShowEdgeEditDialog();  ///< Show the edge edit dialog.
    void AddNode();             ///< Add a new node to the roadmap.
    void AddStraightLineEdge(); ///< Add a new edge to the roadmap.
    void DeleteSelectedItems(); ///< Delete selected nodes and edges.
    void MergeSelectedNodes();  ///< Merge multiple nodes into a super node.
    void RandomizeCCColors();   ///< Randomly set the CC display color.
    void MakeCCsOneColor();     ///< Unify all CC colors.

  private:

    //gui management
    void CreateActions();
    void SetHelpTips();
    void SetUpSubmenu();
    void SetUpToolTab();
    void Reset();

    QMenu* m_modifyCCs;   ///< Submenu for CC display options.

    SliderDialog* m_nodeSizeDialog;      ///< Slider for scaling nodes.
    SliderDialog* m_edgeThicknessDialog; ///< Slider for scaling edges.
};

#endif
