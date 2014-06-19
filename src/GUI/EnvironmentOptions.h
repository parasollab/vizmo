/*****************************************************************
* Class for the "Environment" submenu and associated action button
******************************************************************/

#ifndef ENVIRONMENT_OPTIONS_H_
#define ENVIRONMENT_OPTIONS_H_

#include "OptionsBase.h"

using namespace std;

class ChangeBoundaryDialog;
class EditRobotDialog;
class ModelSelectionWidget;
class MultiBodyModel;

class EnvironmentOptions : public OptionsBase {

  Q_OBJECT

  public:
    EnvironmentOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

    QThread* GetMPThread() {return m_threadDone ? NULL : m_thread;}

  private slots:
    void RefreshEnv();
    void RandomizeEnvColors();

    void AddObstacle();
    void DeleteObstacle();
    void MoveObstacle();
    void DuplicateObstacles();
    void ChangeBoundaryForm();
    void EditRobot();

    void AddRegionBox();
    void AddRegionSphere();
    void DeleteRegion();
    void MakeRegionAttract();
    void MakeRegionAvoid();
    void ChangeRegionType(bool _attract);

    void AddUserPath();
    void DeleteUserPath();
    void PrintUserPath();

    void HandleTimer();
    void MapEnvironment();
    void ThreadDone();

  private:
    void CreateActions();
    void SetUpCustomSubmenu();
    void SetUpToolbar(); //Just randomize colors button
    void SetUpToolTab();
    void Reset();
    void SetHelpTips();

    bool m_regionsStarted, m_threadDone;
    QMenu* m_obstacleMenu;
    QThread* m_thread;
    QTimer* m_timer;
    short m_userPathCount;

    QPointer<EditRobotDialog> m_editRobotDialog;
    QPointer<ChangeBoundaryDialog> m_changeBoundaryDialog;
};

class MapEnvironmentWorker : public QObject {
  Q_OBJECT

  public slots:
    void Solve();

  signals:
    void Finished();
};

#endif
