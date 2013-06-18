#ifndef _MAIN_WIN_H_
#define _MAIN_WIN_H_

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

///////////////////////////////////////////////////////////////////////////////
// Qt Headers
#include <QMainWindow>  
#include <QToolBar>
#include <QKeyEvent>

class QAction;
class QGridLayout;
class QHBoxLayout; 
class QVBoxLayout; 

class VizmoAnimationGUI;
class VizmoScreenShotGUI;
class VizmoItemSelectionGUI;
class VizmoAttributeSelectionGUI;
class queryGUI;
class TextGUI; 
class MainMenu;  

#include<stdio.h>
#include <stdlib.h>
///////////////////////////////////////////////////////////////////////////////
// std Headers
#include <vector>
#include <string>

using namespace std;

#define M_MAX 999.99
#define M_MIN -999.99
#define STEP 0.1
#define D_MAX 1
#define D_MIN 0
///////////////////////////////////////////////////////////////////////////////
class VizGLWin;

class VizmoMainWin : public QMainWindow {
  
  Q_OBJECT

  public:
    VizmoMainWin(QWidget* _parent = 0);
    virtual ~VizmoMainWin(); 
    
    bool Init();
    bool InitVizmo();

    vector<string>& GetArgs() { return m_args; }  
    void SetArgs(vector<string> _args){ m_args = _args; }
    void SetVizmoInit(bool _tf) { m_bVizmoInit = _tf; } 
    bool GetVizmoInit() { return m_bVizmoInit; }
    VizGLWin* GetGLScene() { return m_GL; } 
    TextGUI* GetOutbox(){ return m_outbox; }  
    VizmoAnimationGUI* GetAnimationGUI() { return m_animationGUI; }
    VizmoAnimationGUI* GetAnimationDebugGUI() { return m_animationDebugGUI; }  
    VizmoItemSelectionGUI* GetObjectSelection() { return m_objectSelection; }  

    //command line to be executed to make a new roadmap
    string command;
    QStringList obprm_comm;
    bool m_setQS, m_setQG;  //used to know if values in window will need to be updated
    string m_firstQryFile;   //to hold name of first query file
    MainMenu* m_mainMenu;   //top menubar...owns everything  
    TextGUI* m_outbox;      //Q3TextView that displays node/edge(s) selection info, debug, etc.  

  protected:
    bool CreateGUI();  //create toolbars
    void keyPressEvent(QKeyEvent* _e);

  private slots:
    void updateScreen();  // redraw GL scene
    void objectEdit();    // change object property
    //void runCode();       //Call obprm  *Does this work?  
   // void createQryFile(); // generate qry command line 

  private:
    void SetUpLayout();     //Set a QGridLayout for invisible central widget
    
    vector<string> m_args; //user input arguments.
    bool m_bVizmoInit;     //true if vizmo is init.

    QGridLayout* m_layout;
    QHBoxLayout* m_toolbarLayout;
    QToolBar* m_allTogether;       //all of the toolbars in one...keeps them together for window resize 
    QVBoxLayout* m_objTextLayout;  //contains the Environment Objects selection list and the TextGUI 
    QVBoxLayout* m_animationBarLayout;  //contains the animation and debug controls 
    QWidget* m_layoutWidget;  //placeholder to hold the overall layout. This is the invisible central widget. 
    VizGLWin* m_GL;           //the scene window which displays environment..
    VizmoAnimationGUI *m_animationGUI, *m_animationDebugGUI;
    VizmoItemSelectionGUI *m_objectSelection;
};

#endif /*_MAIN_WIN_H_*/



