/**
 * This file defines class for main window of vimzo2. 
 */

#ifdef WIN32
#pragma warning(disable : 4786)
#endif


///////////////////////////////////////////////////////////////////////////////
// Qt Headers
#include <qmainwindow.h>
class QAction;

class VizmoAnimationGUI;
///////////////////////////////////////////////////////////////////////////////
// std Headers
#include <vector>
#include <string>
using namespace std;

///////////////////////////////////////////////////////////////////////////////
class VizGLWin;


class VizmoMainWin : public QMainWindow 
{
    Q_OBJECT

public:

    VizmoMainWin(QWidget * parent=0, const char * name=0);
    bool Init();
    bool InitVizmo();
    void SetArgs( vector<string> args ){ m_Args=args; }

    double R, G, B;

public slots:
 void updateScreen();
    
protected:

    bool CreateGUI();      //create tool bars

    void keyPressEvent ( QKeyEvent * e );

private slots:

    void load();
    void reset();
    void showmap();       //show roadmap
    void showpath();      //show path frame
    void showstartgoal(); //show start and goal position
    void showBBox();      //show Bounding Box
    void about();         //about dialog
    void aboutQt();       //about Qt dialog
    void contexmenu();    //create contex menu when right mouse clicked
    void notimp();        //not implemented yet.
    void changecolor();   //to change the background color for vizmo++
    void refreshEnv();    // refresh!!!
   

    /*
    void animate();       // show animation

    */


    void setSolid();      // turn object into solidMode
    void setWire();       // turn object into wire mode
    void setInvisible();  // turn the object invisible
   

private:
  
    bool CreateActions();   //Create Qt Actions
    void SetTips();         //Set Tip/Info about actions.
    void CreateToolbar();   //Create Qt Toolbar
    void CreateMenubar();   //Create Qt Menu
    

    QAction *showHideRoadmapAction, 
            *showHidePathAction, 
            *showHideSGaction,
            *quitAction,
            *fileOpenAction, 
            *cameraResetAction, 
            *wireFrameAction,
            *changeColorAction;
     

    VizmoAnimationGUI *animationGUI;

    vector<string> m_Args; //user input arguments.
    bool m_bVizmoInit;     //true if vizmo is init.
   


 protected:
    VizGLWin * m_GL;       //the scene window which displays environment..
};






