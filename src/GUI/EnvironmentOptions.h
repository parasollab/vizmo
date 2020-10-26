#ifndef ENVIRONMENT_OPTIONS_H_
#define ENVIRONMENT_OPTIONS_H_
#include "OptionsBase.h"

#include "Utilities/Color.h"
  
using namespace std;

class ChangeBoundaryDialog;
class EditRobotDialog;
class EdgeModel;

////////////////////////////////////////////////////////////////////////////////
/// \brief This class creates the "Environment" tools and associated actions.
////////////////////////////////////////////////////////////////////////////////
class EnvironmentOptions : public OptionsBase {

  Q_OBJECT

  public:

    EnvironmentOptions(QWidget* _parent);
    std::map<size_t, double> m_vertexMap;
    std::map<std::pair<size_t, size_t>, double> m_edgeMap;

  private slots:

    //environment editing functions
    void AddObstacle();         ///< Create a new obstacle from a file.
    void DeleteObstacle();      ///< Delete the selected obstacle.
    void MoveObstacle();        ///< Launches an ObstaclePosDialog.
    void DuplicateObstacles();  ///< Duplicate the selected obstacles.
    void ChangeBoundaryForm();  ///< Launch a ChangeBoundaryDialog.
    void EditRobot();           ///< Launch an EditRobotDialog.
    //revision
    void LoadEllipses();        ///< Loads shapes from a file

    //skeleton functions
    void AddSkeleton();         	///< Adds a skeleton
    void AddAnnotations();
    void SaveSkeleton();        	///< Saves current skeleton
    void AddVertex();           	///< Adds a vertex to the skeleton
    void AddStraightLineEdge();		///< Adds an edge
    void AddSpecificEdge(EdgeModel* _e1, EdgeModel* _e2, int _i);
    void DeleteSelectedItems();		///< Deletes selected items
    void ChangeColor();                 ///< Changes Color of Selected Items
    void MergeEdges();                  ///< Merges two edges
    void CollapseEdge();                ///< Collapses an Edge

    //environment display functions
    void RefreshEnv();    ///< Reset the rendering mode and ModelSelectionWidget.
    void ClickRobot();          ///< Display configurations in robot mode.
    void ClickPoint();          ///< Display configurations ins point mode.
    void RandomizeEnvColors();  ///< Randomize obstacle colors.

  private:

    //gui management
    void CreateActions();
    void SetHelpTips();
    void SetUpSubmenu();
    void SetUpToolTab();
    void Reset();

    QMenu* m_obstacleMenu;  ///< Menu for obstacle manipulation.
    QMenu* m_nodeShape;     ///< Menu for selecting robot display mode.
    //Revisions
    QMenu* m_skeletonMenu;  ///< Menu for selecting skelton for workspace.

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The boundary editing dialog.
    QPointer<ChangeBoundaryDialog> m_changeBoundaryDialog;
    QPointer<EditRobotDialog> m_editRobotDialog; ///< The robot editing dialog.

    Color4 InterpolateColor(double value, double maxEl, double minEl);

};

#endif
