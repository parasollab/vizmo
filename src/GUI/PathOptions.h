#ifndef PATH_OPTIONS_H_
#define PATH_OPTIONS_H_

#include "OptionsBase.h"

class CustomizePathDialog;


////////////////////////////////////////////////////////////////////////////////
/// \brief Provides the "Path" submenu and associated action buttons.
////////////////////////////////////////////////////////////////////////////////
class PathOptions : public OptionsBase {

  Q_OBJECT

  public:

    PathOptions(QWidget* _parent);

  private slots:

    //path functions
    void ShowHidePath();        ///< Toggle path display on or off.
    void PathDisplayOptions();  ///< Show the path display customization dialog.

  private:

    //gui management
    void CreateActions();
    void SetHelpTips();
    void Reset();

    ////////////////////////////////////////////////////////////////////////////
    /// \brief The path display customization dialog.
    QPointer<CustomizePathDialog> m_customizePathDialog;
};

#endif
