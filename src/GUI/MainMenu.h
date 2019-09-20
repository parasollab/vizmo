#ifndef MAIN_MENU_H_
#define MAIN_MENU_H_

#include <QtGui>
#include <QWidget>

class MainWindow;
class OptionsBase;
class QMenuBar;

using namespace std;


////////////////////////////////////////////////////////////////////////////////
/// \brief The main menu class, which owns the various "options" classes (e.g.
/// File and Roadmap) and displays their submenus.
////////////////////////////////////////////////////////////////////////////////
class MainMenu : public QWidget {

  Q_OBJECT

  public:

    MainMenu(MainWindow* _mainWindow);

    OptionsBase* m_fileOptions;         ///< IO functions.
    OptionsBase* m_glWidgetOptions;     ///< GL scene functions.
    OptionsBase* m_environmentOptions;  ///< Environment functions.
    OptionsBase* m_roadmapOptions;      ///< Roadmap functions.
    OptionsBase* m_pathOptions;         ///< Path functions.
    OptionsBase* m_rvOptions;           ///< RV functions.
    OptionsBase* m_planningOptions;     ///< Motion planning functions.
    OptionsBase* m_queryOptions;        ///< Query functions.
    OptionsBase* m_captureOptions;      ///< Movie/screenshot functions.
    OptionsBase* m_toolTabOptions;      ///< Controls tool tab display.
    OptionsBase* m_help;                ///< Provides "what's this" and about.

    QMenuBar* m_menuBar;                ///< The main menu.
    QAction* m_end;                     ///< Track the last submenu.

    void CallReset();                   ///< Reset all option classes.
    void ConfigureToolTabMenu();        ///< Initialize the tool tab menu.
};

#endif
