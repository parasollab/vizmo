/**
 * This file defines class for main window of vimzo2. 
 */

#ifdef WIN32
#pragma warning(disable : 4786)
#endif


///////////////////////////////////////////////////////////////////////////////
// Qt Headers
#include <qmainwindow.h>
#include <qtoolbutton.h>
#include <qdialog.h>

class QAction;

class VizmoAnimationGUI;
class VizmoScreenShotGUI;
class VizmoItemSelectionGUI;
class VizmoAttributeSelectionGUI;
class VizmoRoadmapNodesShapeGUI;

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

protected:

    bool CreateGUI();      //create tool bars

    void keyPressEvent ( QKeyEvent * e );

private slots:

    void load();
    void reset();
    void showmap();         //show roadmap
    void showpath();        //show path frame
    void showstartgoal();   //show start and goal positionx
    void showBBox();        //show Bounding Box
    void about();           //about dialog
    void aboutQt();         //about Qt dialog
    void contexmenu();      //create contex menu when right mouse clicked
    void notimp();          //not implemented yet.
    void refreshEnv();      // refresh!!!
    void changecolor();     //to change the background color for vizmo++
    void changeSize();      //Changes Robot's size
    void getSelectedItem(); //reads the index on the ListBox

    void setSolid();      // turn object into solidMode
    void setWire();       // turn object into wire mode
    void setInvisible();  // turn the object invisible
    void updateScreen();  // redraw GL scene
    void getOpenglSize(int *,int *);
   

private:
  
    bool CreateActions();   //Create Qt Actions
    void SetTips();         //Set Tip/Info about actions.
    void CreateToolbar();   //Create Qt Toolbar
    void CreateMenubar();   //Create Qt Menu
    void CreateScreenCapture(); // Create the screenshot gui
    void CreateObjectSelection(); // create object selection
    void CreateAttributeSelection(); // create object selection
    void CreateShapeSelection(); // Create listbox selection to change   

    QAction *showHideRoadmapAction, 
            *showHidePathAction, 
            *showHideSGaction,
            *quitAction,
            *fileOpenAction, 
            *cameraResetAction, 
            *wireFrameAction,
            *changeColorAction,
            *recordAction,
            *changeRobotSizeAction;
     

    VizmoAnimationGUI *animationGUI;
    VizmoScreenShotGUI *screenShotGUI;
    VizmoItemSelectionGUI *objectSelection;
    VizmoAttributeSelectionGUI *attributeSelection;
 
    VizmoRoadmapNodesShapeGUI *shapeSelection;

    vector<string> m_Args; //user input arguments.
    bool m_bVizmoInit;     //true if vizmo is init.
   
    QToolBar *vizmoTools;
    QToolButton *folderButton, *roadmapButton, *pathButton, *strtGoalButton;
    QToolButton  *cameraButton, *palletButton; 

 protected:
    VizGLWin * m_GL;       //the scene window which displays environment..
};





