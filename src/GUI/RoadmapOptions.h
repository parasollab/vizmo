/****************************************************************
 * Class for the "Roadmap" submenu and associated toolbars
 * *************************************************************/

#ifndef ROAD_MAP_OPTIONS_H_
#define ROAD_MAP_OPTIONS_H_

#include "OptionsBase.h"
#include "Models/MapModel.h"
#include "Models/Vizmo.h"

class QPushButton;
class SliderDialog;
class EdgeEditDialog;
class NodeEditDialog;

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

    RoadmapOptions(QWidget* _parent, MainWindow* _mainWindow);

  private slots:
    void ShowRoadmap();
    void ClickRobot(); //trim down these functions eventually
    void ClickPoint();
    void MakeSolid();
    void MakeWired();
    void MakeInvisible();
    void ShowNodeSizeDialog();
    void ScaleNodes();
    void ShowEdgeThicknessDialog();
    void ChangeEdgeThickness();
    void ShowNodeEditDialog();
    void ShowEdgeEditDialog();
    void AddNode();
    void AddStraightLineEdge();
    void DeleteSelectedItems();
    void MergeSelectedNodes();
    void RandomizeCCColors();
    void MakeCCsOneColor();
    void ShowObjectContextMenu();
    void ChangeObjectColor();

  private:
    void CreateActions();
    void SetUpCustomSubmenu(); //submenu has its own submenus and cannot be set up with simple loop through m_actions
    void SetUpToolbar();       //a lot of toolbar functionalities for this class
    void SetUpToolTab();
    void Reset();
    void SetHelpTips();

    QMenu* m_nodeShape;       //More submenus within the robot submenu
    QMenu* m_modifySelected;
    QMenu* m_modifyCCs;

    SliderDialog* m_nodeSizeDialog; //Provides slider to scale nodes
    SliderDialog* m_edgeThicknessDialog; //Slider to scale the edges
};

#endif
