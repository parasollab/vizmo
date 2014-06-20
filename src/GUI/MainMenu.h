/*************************************************************
 * The Main (drop-down) menu class, which owns various "options" classes (e.g.
 File and Roadmap) that have associated buttons/toolbars and the submenus.
 **************************************************************/

#ifndef MAINMENU_H_
#define MAINMENU_H_

#include <QtGui>

class MainWindow;
class OptionsBase;
class QMenuBar;
class ToolTabOptions;

using namespace std;

class MainMenu : public QWidget {

  Q_OBJECT

  public:
    MainMenu(MainWindow* _mainWindow);

    OptionsBase* m_fileOptions;         //File tool button and submenu
    OptionsBase* m_glWidgetOptions;        //2 scene buttons and submenu
    OptionsBase* m_environmentOptions;  //randomize colors button and submenu
    OptionsBase* m_roadmapOptions;      //large toolbar and submenu
    OptionsBase* m_pathOptions;         //2 buttons and submenu
    OptionsBase* m_planningOptions;
    OptionsBase* m_queryOptions;         //2 buttons and submenu
    OptionsBase* m_captureOptions;      //3 buttons and submenu
    ToolTabOptions* m_toolTabOptions;      //toggle tool tabs on/off
    OptionsBase* m_help;                //The What's This? button

    QMenuBar* m_menuBar;
    QAction* m_end;                     //track the last menu item

  public slots:
      void CallReset();

  private slots:
      void SetUpMainMenu();

  private:
    OptionsBase* m_robotOptions;        //just a submenu

};

#endif
