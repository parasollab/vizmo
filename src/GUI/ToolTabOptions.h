#ifndef TOOL_TAB_OPTIONS_H_
#define TOOL_TAB_OPTIONS_H_

#include <QtGui>
#include "OptionsBase.h"


////////////////////////////////////////////////////////////////////////////////
/// \brief Provides the drop-down menu that allows for enabling/disabling
/// specific tool tabs.
////////////////////////////////////////////////////////////////////////////////
class ToolTabOptions : public OptionsBase {

  Q_OBJECT

  public:

    ToolTabOptions(QWidget* _parent) : OptionsBase(_parent, "Tool Tabs") { }

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Provides a public interface to calling CreateActions. This is
    /// needed because the ToolTabOptions are created before the tool tabs. Init
    /// must be called after the tabs have been constructed.
    void Init() {CreateActions();}

  private:

    //gui management
    void CreateActions();
    void SetHelpTips() {}
    void SetUpToolbar() {}
    void Reset() {}
};

#endif
