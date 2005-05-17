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
#include <qtextedit.h> 

class QAction;

class VizmoAnimationGUI;
class VizmoScreenShotGUI;
class VizmoItemSelectionGUI;
class VizmoAttributeSelectionGUI;
class VizmoRoadmapGUI;

#include<stdio.h>
#include <stdlib.h>
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

    //double R, G, B;

protected:

    bool CreateGUI();      //create tool bars

    void keyPressEvent ( QKeyEvent * e );

    void reset(); //reset when new vizmo files are loaded

private slots:

    void load();
    void updatefiles();     //update opened/reopen files
    void resetCamera();     //reset camera

    void showmap();         //show roadmap
    void showpath();        //show path frame
    void showstartgoal();   //show start and goal positionx
    void showBBox();        //show Bounding Box

    void about();           //about dialog
    void obj_contexmenu();  //create contex menu when right mouse clicked on objs
    void gen_contexmenu();  //create contex menu when right moise clicked on nothing
    void notimp();          //not implemented yet.

    void refreshEnv();      // refresh!!!
    void changeBGcolor();     //to change the background color for vizmo++
    void envObjsRandomColor();

    void saveEnv();           // save environment file
    void saveQryFile();           //save query file (preliminary work)
    void saveQryStart();      //save start cfg
    void saveQryGoal();      //save goal cfg
    void saveRoadmap();     //save a new roadmap file with new info. about nodes

    void setSolid();      // turn object into solidMode
    void setWire();       // turn object into wire mode
    void setInvisible();  // delete the object (obstacle)
    void setNewColor();   // change the object's color

    void updateScreen();  // redraw GL scene
    void getOpenglSize(int *,int *);
    void objectEdit();    // change object property

    void resetRobotPosition(); //reset Robot to initial Cfg.
   
    void enablDisablCD(); //Enable/disable collision detection
    void runCode();  //Call obprm
    void createQryFile(); // generate qry command line 
    void createEditor(); // create qry file for obprm

    void addObstacle();
private:
  
    bool CreateActions();   //Create Qt Actions
    void SetTips();         //Set Tip/Info about actions.
    void CreateToolbar();   //Create Qt Toolbar
    void CreateMenubar();   //Create Qt Menu
    void CreateScreenCapture(); // Create the screenshot gui
    void CreateObjectSelection(); // create object selection
    void CreateAttributeSelection(); // create object selection
    void CreateRoadmapToolbar(); //Create color box for CC's 

    QAction *showHideRoadmapAction, 
            *showHidePathAction, 
            *showHideSGaction,
            *quitAction,
            *fileOpenAction, 
            *fileUpdateAction,
            *fileSaveAction,
            *fileSaveQryAction,
            *fileSaveMapAction,
            *cameraResetAction, 
            *wireFrameAction,
            *changeBGcolorAction,
			*randObjcolorAction,
			*showAxisAction,
      *showGridAction,
      *resetRobotPosAction,
      *addObstacleAction;

    VizmoAnimationGUI *animationGUI;
    VizmoScreenShotGUI *screenShotGUI;
    VizmoItemSelectionGUI *objectSelection;
    VizmoAttributeSelectionGUI *attributeSelection;

     VizmoRoadmapGUI *roadmapGUI;  //for CC's

    vector<string> m_Args; //user input arguments.
    bool m_bVizmoInit;     //true if vizmo is init.
   
    QToolBar *vizmoTools;
    QToolButton *folderButton, *roadmapButton, *pathButton, *strtGoalButton;
    QToolButton  *cameraButton, *palletButton, *envButton, *CDButton; 
    QToolButton *editorButton;

    VizGLWin * m_GL;       //the scene window which displays environment..


};






