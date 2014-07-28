#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

using namespace std;

#include <QtGui>

class QAction;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;
class QTimer;

class AnimationWidget;
class VizmoScreenShotGUI;
class ModelSelectionWidget;
class VizmoAttributeSelectionGUI;
class TextWidget;
class ToolTabWidget;
class MainMenu;
class GLWidget;

#define M_MAX 999.99
#define M_MIN -999.99
#define STEP 0.1
#define D_MAX 1
#define D_MIN 0

class MainWindow : public QMainWindow {

  Q_OBJECT

  public:
    MainWindow(QWidget* _parent = 0);

    bool Init();
    bool InitVizmo();
    vector<string>& GetArgs() { return m_args; }
    void SetVizmoInit(bool _tf) { m_vizmoInit = _tf; }
    bool GetVizmoInit() { return m_vizmoInit; }
    GLWidget* GetGLWidget() { return m_gl; }
    AnimationWidget* GetAnimationWidget() { return m_animationWidget; }
    ModelSelectionWidget* GetModelSelectionWidget() { return m_modelSelectionWidget; }
    QDockWidget* GetDialogDock() { return m_dialogDock; }
    QTimer* GetMainClock() {return m_timer;}
    void ShowDialog(QDialog* _dialog);
    void ResetDialogs();

    MainMenu* m_mainMenu;   //top menubar...owns everything
    ToolTabWidget* m_toolTabWidget; //tab widget, owns tool tabs

    void closeEvent(QCloseEvent* _event);

  private slots:
      void UpdateScreen();  // redraw GL scene
    void HideDialogDock();

  private:
    string m_command;    //command line to be executed to make a new roadmap
    QStringList m_obprmComm;
    bool m_setQS, m_setQG;  //used to know if values in window will need to be updated
    string m_firstQryFile;   //to hold name of first query file
    TextWidget* m_textWidget;      //Q3TextView that displays node/edge(s) selection info, debug, etc.

    void SetUpLayout();     //Set a QGridLayout for invisible central widget
    void keyPressEvent(QKeyEvent* _e);
    bool CreateGUI();

    vector<string> m_args; //user input arguments.
    bool m_vizmoInit;     //true if vizmo is init.

    GLWidget* m_gl;           //the scene window which displays environment..
    AnimationWidget* m_animationWidget;
    ModelSelectionWidget* m_modelSelectionWidget;
    QDockWidget* m_dialogDock;

    QTimer* m_timer;
};

#endif /*_MAIN_WIN_H_*/

