/*****************************************************************
 * Class for the "Path" submenu and associated action buttons
 *****************************************************************/

#ifndef PATH_OPTIONS_H_
#define PATH_OPTIONS_H_

#include "OptionsBase.h"

class CustomizePathDialog;

class PathOptions : public OptionsBase {

  Q_OBJECT

  public:
    PathOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

  private slots:
    void ShowHidePath();
    void PathDisplayOptions();

  private:
    void CreateActions();
    void SetUpCustomSubmenu(); //like RoadmapOptions
    void SetUpToolbar();       //show/hide path and show/hide start/goal
    void SetUpToolTab();
    void Reset();
    void SetHelpTips();

    QPointer<CustomizePathDialog> m_customizePathDialog; //pop-up dialog for gradient input, etc.
};

#endif
