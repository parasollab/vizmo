#ifndef PLANNING_OPTIONS_H_
#define PLANNING_OPTIONS_H_

#include "OptionsBase.h"
#include "Models/EnvModel.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief This class provides access to the user-guided planning tools.
////////////////////////////////////////////////////////////////////////////////
class PlanningOptions : public OptionsBase {

  Q_OBJECT

  public:

    typedef EnvModel::RegionModelPtr RegionModelPtr;

    PlanningOptions(QWidget* _parent);
    ~PlanningOptions();

    //planning thread access
    QThread* GetMPThread() {return m_thread ? m_thread : NULL;}
    void HaltMPThread(); ///< Deletes the current mapping thread if one exists.

    void StartPreInputTimer();

  private slots:

    //Region functions
    void AddRegionSphere();   ///< Add a new sphere region to the workspace.
    void AddRegionBox();      ///< Add a new box region to the workspace.
    void PlaceCfg();          ///< Place any cfgs that will be used in the planer
    void SaveCfg();           ///< Saves any cfgs placed in environment
    void LoadCfg();           ///< Loads cfgs and places them in the roadmap
    void DuplicateRegion();   ///< Create a noncommit copy of the selected region.
    void DeleteRegion();      ///< Delete the selected region.
    void MakeRegionAttract(); ///< Set the selected region to attract.
    void MakeRegionAvoid();   ///< Set the selected region to avoid.
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Change the type of the currently selected non-commit region to
    /// \a attract or \a avoid.
    /// \param[in] _attract Indicates \a attract (\c true) or \a avoid (\c false).
    void ChangeRegionType(bool _attract);
    void SaveRegion();        ///< Save all regions to a file.
    void LoadRegion();        ///< Load one or more regions from a file.

    //User Path functions
    void AddUserPath();       ///< Create a user path.
    void DeleteUserPath();    ///< Delete the selected user path.
    void PrintUserPath();     ///< Output the selected user path to a file.
    void HapticPathCapture(); ///< Build a user path from the haptic cursor.
    void CameraPathCapture(); ///< Build a user path from the camera position.
    void SavePath();          ///< Saves and user paths specified
    void LoadPath();          ///< Loads a set of user specified paths


    //Common planning functions
    void MapEnvironment();    ///< Start a mapping thread to run an MPStrategy.
    void ThreadDone();        ///< Clean-up after mapping thread finishes.

  private:

    //gui management
    void CreateActions();
    void SetUpSubmenu();
    void SetUpToolTab();
    void Reset();
    void SetHelpTips();

    //region helpers
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Choose the sampler to use for the currently selected region.
    void ChooseSamplerStrategy();
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Test that only a single region is selected. Alert the user if
    /// the test fails.
    /// \return A \c bool indicating whether the test passed.
    bool SingleRegionSelected();

    bool m_userInputStarted;       ///< Tracks whether user input timer is running.
    QThread* m_thread;             ///< Points to the current mapping thread.
    QMenu* m_addRegionMenu;        ///< Menu for adding new regions.
    QMenu* m_regionPropertiesMenu; ///< Menu for modifying regions.
    QMenu* m_pathsMenu;            ///< Menu for working with user paths.
};


////////////////////////////////////////////////////////////////////////////////
/// \brief This class launches interactive strategies.
////////////////////////////////////////////////////////////////////////////////
class MapEnvironmentWorker : public QObject {

  Q_OBJECT

  public:

    MapEnvironmentWorker(string _strategyLabel);

  public slots:

    void Solve();           ///< Executes the desired strategy.

  signals:

    void Finished();        ///< Indicates that the strategy has halted.

  private:

    string m_strategyLabel; ///< The label of the strategy to execute.

};

#endif
