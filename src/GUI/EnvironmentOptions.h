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

    void MapEnvironment();

  private:
    void CreateActions();
    void SetUpCustomSubmenu();
    void SetUpToolbar(); //Just randomize colors button
    void Reset();
    void SetHelpTips();

    QMenu* m_obstacleMenu;
};

class MapEnvironmentWorker : public QObject {
  Q_OBJECT

  public slots:
    void Solve();

  signals:
    void Finished();
};

#endif
