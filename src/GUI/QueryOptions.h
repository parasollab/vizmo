/*****************************************************************
 * Class for the "Path" submenu and associated action buttons
 *****************************************************************/

#ifndef QUERYOPTIONS_H_
#define QUERYOPTIONS_H_

#include "OptionsBase.h"

class QueryOptions : public OptionsBase {

  Q_OBJECT

  public:
    QueryOptions(QWidget* _parent = 0, MainWindow* _mainWindow = 0);

  private slots:
    void ShowHideQuery();
    void EditQuery();

  private:
    void CreateActions();
    void SetUpCustomSubmenu(); //like RoadmapOptions
    void SetUpToolbar();       //show/hide path and show/hide start/goal
    void Reset();
    void SetHelpTips();
};

#endif
