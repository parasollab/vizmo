/****************************************************************
 * Class for the "Roadmap" submenu and associated toolbars
 * *************************************************************/

#ifndef ROADMAPOPTIONS_H_
#define ROADMAPOPTIONS_H_

#include "QButtonGroup"

#include "OptionsBase.h"
#include "Models/MapModel.h"
#include "Models/Vizmo.h"

class QPushButton;
class SliderDialog;
class NodeEditDialog;

class RoadmapOptions : public OptionsBase {

  Q_OBJECT

  public:
    RoadmapOptions(QWidget* _parent, MainWindow* _mainWindow);

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
    void ShowNodeEditDialog(); //Named box as reminder that it's not yet a QDialog as desired..
    void RandomizeCCColors();
    void MakeCCsOneColor();
    void ShowObjectContextMenu();
    void ChangeObjectColor();

  private:
    void CreateActions();
    void SetUpCustomSubmenu(); //submenu has its own submenus and cannot be set up with simple loop through m_actions
    void SetUpToolbar();       //a lot of toolbar functionalities for this class
    void Reset();
    void SetHelpTips();

    typedef MapModel<CfgModel, EdgeModel> MM;
    MM* GetMapModel(){
      if(!GetVizmo().GetMap())
        return NULL;
      else
        return GetVizmo().GetMap();
    }
    string GetNodeShape(){
      return (string)(m_nodeView->checkedButton())->text().toAscii();
    }

    QButtonGroup* m_nodeView;   //For ease of use--includes the 3 buttons below
    QPushButton* m_robotButton; //These 3 particular menu items have text only and thus look better as buttons
    QPushButton* m_boxButton;
    QPushButton* m_pointButton;
    QMenu* m_nodeShape;       //More submenus within the robot submenu
    QMenu* m_modifySelected;
    QMenu* m_modifyCCs;

    SliderDialog* m_nodeSizeDialog; //Provides slider to scale nodes
    SliderDialog* m_edgeThicknessDialog; //Slider to scale the edges
};

#endif
