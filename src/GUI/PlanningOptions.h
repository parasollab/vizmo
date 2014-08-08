#ifndef PLANNINGOPTIONS_H_
#define PLANNINGOPTIONS_H_

#include "OptionsBase.h"

using namespace std;

class PlanningOptions : public OptionsBase {

  Q_OBJECT

  public:
    PlanningOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

    QThread* GetMPThread() {return m_threadDone ? NULL : m_thread;}

  private slots:
    void AddRegionSphere();
    void AddRegionBox();
    void DeleteRegion();
    void MakeRegionAttract();
    void MakeRegionAvoid();
    void DuplicateRegion();
    void ChangeRegionType(bool _attract);
    void SaveRegion();
    void LoadRegion();
    void RegionRRT();
    void SparkRegion();

    void HandleTimer();
    void MapEnvironment();
    void ThreadDone();

    void AddUserPath();
    void DeleteUserPath();
    void PrintUserPath();
    void HapticPathCapture();
    void CameraPathCapture();

  private:
    void CreateActions();
    void SetUpCustomSubmenu();
    void SetUpToolbar();
    void SetUpToolTab();
    void Reset();
    void SetHelpTips();
    void ChooseSamplerStrategy();

    bool m_regionsStarted;
    bool m_threadDone;
    QThread* m_thread;
    QTimer* m_timer;
    QMenu* m_addRegionMenu;
    QMenu* m_regionPropertiesMenu;
    QMenu* m_pathsMenu;
    short m_userPathCount;
};

class MapEnvironmentWorker : public QObject {

  Q_OBJECT

  public:
    MapEnvironmentWorker(string _strategyLabel);

  public slots:
    void Solve();

  signals:
    void Finished();

  private:
    string m_strategyLabel;
};

#endif
