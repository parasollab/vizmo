/****************************************************************
 * Class for the "Roadmap" submenu and associated toolbars
 * *************************************************************/

#ifndef ROADMAP_OPTIONS_H
#define ROADMAP_OPTIONS_H

#include <string>

#include "QButtonGroup"

#include "OptionsBase.h"
#include "Models/MapModel.h"
#include "Models/DebugModel.h"
#include "Models/Vizmo.h"

class QPushButton;
class SliderDialog;

class RoadmapOptions : public OptionsBase {

  Q_OBJECT

  public:
    RoadmapOptions(QWidget* _parent, VizmoMainWin* _mainWin);
    void CreateActions();
    void SetUpCustomSubmenu(); //submenu has its own submenus and cannot be set up with simple loop through m_actions
    void SetUpToolbar();       //a lot of toolbar functionalities for this class
    void Reset();
    void SetHelpTips();

    typedef MapModel<CfgModel, EdgeModel> MM;
    MM* GetMapModel() {
      if(!GetVizmo().GetMap())
        return NULL;
      else
        return GetVizmo().GetMap();
    }
    string GetNodeShape() {return (string)(m_nodeView->checkedButton())->text().toAscii();}

  private slots:
    void ShowRoadmap();
    void ChangeNodeShape();
    void ClickRobot(); //trim down these functions eventually
    void ClickBox();
    void ClickPoint();
    void MakeSolid();
    void MakeWired();
    void MakeInvisible();
    void ShowNodeSizeDialog();
    void ScaleNodes();
    void ShowEdgeThicknessDialog();
    void ChangeEdgeThickness();
    void RandomizeCCColors();
    void MakeCCsOneColor();
    void ShowObjectContextMenu();
    void SaveQueryStart();
    void SaveQueryGoal();
    void ChangeObjectColor();

  private:
    QButtonGroup* m_nodeView;   //For ease of use--includes the 3 buttons below
    QPushButton* m_robotButton; //These 3 particular menu items have text only and thus look better as buttons
    QPushButton* m_boxButton;
    QPushButton* m_pointButton;
    QMenu* m_nodeShape;       //More submenus within the robot submenu
    QMenu* m_modifySelected;
    QMenu* m_modifyCCs;

    SliderDialog* m_nodeSizeDialog; //Provides slider to scale nodes
    SliderDialog* m_edgeThicknessDialog; //Slider to scale the edges

    //  queryGUI* m_nodeGUI;  ***TEMPORARY DISABLE
    //  void UpdateNodeCfg();
};

#endif
