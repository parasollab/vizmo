/*****************************************************************
 * Class for the "Path" submenu and associated action buttons
 *****************************************************************/

#ifndef QUERY_OPTIONS_H_
#define QUERY_OPTIONS_H_

#include "OptionsBase.h"

class QueryEditDialog;

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
    void SetUpToolTab();
    void Reset();
    void SetHelpTips();

    QPointer<QueryEditDialog> m_queryEditDialog;
};

#endif
