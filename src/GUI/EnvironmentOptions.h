#ifndef ENVIRONMENT_OPTIONS_H_
#define ENVIRONMENT_OPTIONS_H_

#include "OptionsBase.h"

using namespace std;

class ChangeBoundaryDialog;
class EditRobotDialog;


////////////////////////////////////////////////////////////////////////////////
/// \brief This class creates the "Environment" tools and associated actions.
////////////////////////////////////////////////////////////////////////////////
class EnvironmentOptions : public OptionsBase {

  Q_OBJECT

  public:

    EnvironmentOptions(QWidget* _parent);

  private slots:

    //environment editing functions
    void AddObstacle();         ///< Create a new obstacle from a file.
    void DeleteObstacle();      ///< Delete the selected obstacle.
    void MoveObstacle();        ///< Launches an ObstaclePosDialog.
    void DuplicateObstacles();  ///< Duplicate the selected obstacles.
    void ChangeBoundaryForm();  ///< Launch a ChangeBoundaryDialog.
    void EditRobot();           ///< Launch an EditRobotDialog.
    //Revisions
    void AddSkeleton();         ///< Adds a skeleton
    void SaveSkeleton();        ///< Saves current skeleton
    void AddVertex();           ///< Adds a vertex to the skeleton
    void ShowSkeletonInfo();    ///< Toggle showing skeleton information.
    void DeleteSelectedItems();  ///< Deletes selected items
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
};

#endif
