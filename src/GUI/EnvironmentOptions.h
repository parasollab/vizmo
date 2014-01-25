/*****************************************************************
* Class for the "Environment" submenu and associated action button
******************************************************************/

#ifndef ENVIRONMENTOPTIONS_H_
#define ENVIRONMENTOPTIONS_H_

#include "OptionsBase.h"

using namespace std;

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

    void MapEnvironment();

    void ThreadDone() {m_threadDone = true;}

  private:
    void CreateActions();
    void SetUpCustomSubmenu();
    void SetUpToolbar(); //Just randomize colors button
    void Reset();
    void SetHelpTips();

    bool m_regionsStarted, m_threadDone;
    QMenu* m_obstacleMenu;
    QThread* m_thread;
};

class MapEnvironmentWorker : public QObject {
  Q_OBJECT

  public slots:
    void Solve();

  signals:
    void Finished();
};

#endif
