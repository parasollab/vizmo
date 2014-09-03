/*****************************************************************
 * Class for the "Environment" submenu and associated action button
 ******************************************************************/

#ifndef ENVIRONMENT_OPTIONS_H_
#define ENVIRONMENT_OPTIONS_H_

#include "OptionsBase.h"

using namespace std;

class ChangeBoundaryDialog;
class EditRobotDialog;

class EnvironmentOptions : public OptionsBase {

  Q_OBJECT

  public:
    EnvironmentOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

  private slots:
    void RefreshEnv();

    void AddObstacle();
    void DeleteObstacle();
    void MoveObstacle();
    void DuplicateObstacles();
    void ChangeBoundaryForm();
    void EditRobot();

    void ClickRobot();
    void ClickPoint();
    void RandomizeEnvColors();

  private:
    void CreateActions();
    void SetUpCustomSubmenu();
    void SetUpToolbar();
    void SetUpToolTab();
    void Reset();
    void SetHelpTips();

    QMenu* m_obstacleMenu;
    QMenu* m_nodeShape;

    QPointer<EditRobotDialog> m_editRobotDialog;
    QPointer<ChangeBoundaryDialog> m_changeBoundaryDialog;
};

#endif
