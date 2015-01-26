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

    EnvironmentOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

  private slots:

    void RefreshEnv();  ///< Reset the rendering mode and ModelSelectionWidget.

    void AddObstacle();         ///< Create a new obstacle from a file.
    void DeleteObstacle();      ///< Delete the selected obstacle.
    void MoveObstacle();        ///< Launches an ObstaclePosDialog.
    void DuplicateObstacles();  ///< Duplicate the selected obstacles.
    void ChangeBoundaryForm();  ///< Launch a ChangeBoundaryDialog.
    void EditRobot();           ///< Launch an EditRobotDialog.

    void ClickRobot();          ///< Display configurations in robot mode.
    void ClickPoint();          ///< Display configurations ins point mode.
    void RandomizeEnvColors();  ///< Randomize obstacle colors.

  private:

    void CreateActions();       ///< Create and connect actions.
    void SetUpCustomSubmenu();  ///< Create a menu.
    void SetUpToolbar();        ///< Create a toolbar.
    void SetUpToolTab();        ///< Create a tool tab.
    void Reset();               ///< Reset actions to their enabled state.
    void SetHelpTips();         ///< Set help messages for actions.

    QMenu* m_obstacleMenu;  ///< Menu for obstacle manipulation.
    QMenu* m_nodeShape;     ///< Menu for selecting robot display mode.

    QPointer<EditRobotDialog> m_editRobotDialog; ///< The robot editing dialog.
    ////////////////////////////////////////////////////////////////////////////
    /// \brief The boundary editing dialog.
    QPointer<ChangeBoundaryDialog> m_changeBoundaryDialog;
};

#endif
