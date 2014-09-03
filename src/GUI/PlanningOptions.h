#ifndef PLANNING_OPTIONS_H_
#define PLANNING_OPTIONS_H_

#include "OptionsBase.h"

using namespace std;

//This class provides access to the user-guided planning tools
class PlanningOptions : public OptionsBase {

  Q_OBJECT

  public:
    PlanningOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

    QThread* GetMPThread() {return m_threadDone ? NULL : m_thread;}

  private slots:
    //Region functions
    void AddRegionSphere();
    void AddRegionBox();
    void DeleteRegion();
    void MakeRegionAttract();
    void MakeRegionAvoid();
    void DuplicateRegion();
    void ChangeRegionType(bool _attract);
    void SaveRegion();
    void LoadRegion();

    //Thread and timer functions
    void MapEnvironment();
    void ThreadDone();

    //User Path functions
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

    bool m_regionsStarted;
    bool m_threadDone;
    QThread* m_thread;
    QMenu* m_addRegionMenu;
    QMenu* m_regionPropertiesMenu;
    QMenu* m_pathsMenu;
    short m_userPathCount;
};

//This helper class launches interactive strategies.
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
