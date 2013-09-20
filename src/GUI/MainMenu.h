/*************************************************************
 * The Main (drop-down) menu class, which owns various "options" classes (e.g.
 File and Roadmap) that have associated buttons/toolbars and the submenus.
 **************************************************************/

#ifndef MAINMENU_H_
#define MAINMENU_H_

#include <QMenuBar>
#include <QWidget>

class MainWindow;
class OptionsBase;

using namespace std;

class MainMenu : public QWidget {

  Q_OBJECT

  public:
    MainMenu(MainWindow* _mainWindow);

    OptionsBase* m_fileOptions;         //File tool button and submenu
    OptionsBase* m_glWidgetOptions;        //2 scene buttons and submenu
    OptionsBase* m_robotOptions;        //just a submenu
    OptionsBase* m_environmentOptions;  //randomize colors button and submenu
    OptionsBase* m_roadmapOptions;      //large toolbar and submenu
    OptionsBase* m_pathOptions;         //2 buttons and submenu
    OptionsBase* m_captureOptions;      //3 buttons and submenu
    OptionsBase* m_help;                //The What's This? button

    QMenuBar* m_menubar;

    void CallReset() {Reset();}

  public slots:
      void SetUpMainMenu();

  private:
    void Reset();

};

#endif
