#ifndef FILE_OPTIONS_H_
#define FILE_OPTIONS_H_

#include "OptionsBase.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief This class provides the 'File' menu and its related actions.
////////////////////////////////////////////////////////////////////////////////
class FileOptions : public OptionsBase {

  Q_OBJECT

  public:

    FileOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

  private slots:

    void LoadFile();       ///< Load a file.
    void UpdateFiles();    ///< Reload currently loaded files.
    void SaveEnv();        ///< Save the current environment to a file.
    void SaveQryFile();    ///< Save the current query to a file.
    void SaveRoadmap();    ///< Save the current roadmap to a file.
    void SavePath();       ///< Save the current robot path to a file.

  private:

    void CreateActions();  ///< Create and connect actions.
    void SetUpToolbar();   ///< Create a tool bar.
    void Reset();          ///< Reset actions to their enabled state.
    void SetHelpTips();    ///< Set help messages for actions.
};

#endif
