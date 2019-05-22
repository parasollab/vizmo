#ifndef OPTIONS_BASE_H_
#define OPTIONS_BASE_H_

#include <QtGui>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
/// \brief Abstract base class for implementing sets of user actions and their
/// corresponding GUI elements.
/// \details Derived classes have a map of associated actions and several GUIs
/// for presenting them to the user, including a drop-down menu, a tool bar,
/// and a tool tab.
////////////////////////////////////////////////////////////////////////////////
class OptionsBase : public QWidget {

  Q_OBJECT

  public:

    OptionsBase(QWidget* _parent, const QString& _label) : QWidget(_parent),
        m_submenu(NULL), m_toolbar(NULL), m_toolTab(NULL), m_label(_label) { }

    virtual void Reset() = 0; ///< Reset all actions when the window is refreshed.

    QMenu* GetSubMenu() {return m_submenu;}    ///< Get the drop-down menu.
    QToolBar* GetToolbar() {return m_toolbar;} ///< Get the tool bar.
    QWidget* GetToolTab() {return m_toolTab;}  ///< Get the tool tab.
    const QString& GetLabel() const {return m_label;} ///< Get the label.

  protected:

    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create and connect actions.
    virtual void CreateActions() = 0;
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Set the help and tool tips for each action.
    virtual void SetHelpTips() = 0;
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a drop-down menu for accessing actions.
    virtual void SetUpSubmenu();
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a tool bar for accessing actions.
    virtual void SetUpToolbar();
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a tool tab for accessing actions.
    virtual void SetUpToolTab();
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Create a tool tab 3 buttons wide from a list of action names.
    /// \param[in] _buttonList The list of actions to use.
    void CreateToolTab(const vector<string>& _buttonList);

    map<string, QAction*> m_actions; ///< The actions for this option group.
    QMenu* m_submenu;    ///< Drop-down menu for access through the MainMenu.
    QToolBar* m_toolbar; ///< Toolbar for access from the MainWindow.
    QWidget* m_toolTab;  ///< Tool tab for access from the MainWindow.
    QString m_label;     ///< The name of the option group.
};

#endif
