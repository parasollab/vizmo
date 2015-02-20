#ifndef QUERY_OPTIONS_H_
#define QUERY_OPTIONS_H_

#include "OptionsBase.h"

class QueryEditDialog;


////////////////////////////////////////////////////////////////////////////////
/// \brief Provides the "Query" submenu and associated action buttons.
////////////////////////////////////////////////////////////////////////////////
class QueryOptions : public OptionsBase {

  Q_OBJECT

  public:

    QueryOptions(QWidget* _parent);

  private slots:

    //query functions
    void ShowHideQuery();   ///< Toggle query display on or off.
    void EditQuery();       ///< Open the query edit dialog.

  private:

    //gui management
    void CreateActions();
    void SetHelpTips();
    void Reset();

    QPointer<QueryEditDialog> m_queryEditDialog; ///< The query edit dialog.
};

#endif
