#include "vizmo2.h"
#include "main_win.h"  
#include "scene_win.h"
#include "animation_gui.h"
#include "snapshot_gui.h"
#include "itemselection_gui.h"
#include "attributeselectio_gui.h"
#include "roadmapShape.h"

///////////////////////////////////////////////////////////////////////////////
//Include Qt Headers
#include <qapplication.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qcolordialog.h> 
#include <qslider.h>
#include <qlcdnumber.h>
#include <qtimer.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qvbox.h>


#include "icon/Eye.xpm"
#include "icon/Folder.xpm"
#include "icon/Pen.xpm"
#include "icon/Camera.xpm"
/*#include "icon/Board.xpm"*/
#include "icon/navigate.xpm"
#include "icon/Flag.xpm"
#include "icon/pallet.xpm"
#include "icon/tapes.xpm"


#include "sync.h"

VizmoMainWin::VizmoMainWin(QWidget * parent, const char * name)
:QMainWindow(parent, name), m_bVizmoInit(false)
{ 
    setMinimumSize( 800, 600 );
    setCaption("vizmo2"); 
    m_GL=NULL;
    animationGUI=NULL;
    move(0,0);
}

bool VizmoMainWin::Init()
{
    this->setIcon(QPixmap(Eye));
    //m_ModelName=modelName;
    
    //Create GLModel
    if( (m_GL=new VizGLWin(this,"GL"))==NULL ) return false;
    setCentralWidget( m_GL );
    
    //Create Other GUI
    if( CreateGUI()==false ) return false;
    
    statusBar()->message("Ready");
    return true;
}

bool VizmoMainWin::InitVizmo()
{

    if( m_bVizmoInit ) return true;
    m_bVizmoInit=true;
    if( m_Args.empty() ) return true; //nothing to init...
	/*
	  Here we use the first argument, but in the future
	  we should use all of them to load files.
	*/


  Sync sync(MODULE_VIZMO);

  reset();


  while (1) {
    char path_filename[SYNC_FILENAME_MAX_LEN];
    sync.GetFileNameRead(path_filename);


    char buffer[SYNC_FILENAME_MAX_LEN];
    sprintf(buffer, "cp %s ./campus.path", path_filename);
    int sr = system(buffer);
    if ((sr == -1) || (sr == 127)) {   
      cout << "cannot copy " << path_filename << "to campus.path\n" <<
        flush;
      return false;
    }


//    m_Args.push_back(path_filename);
    vector<string> files=GetVizmo().GetAccessFiles(m_Args[0]);
    if( GetVizmo().InitVizmoObject(files)==false ){
        return false;
    }

    cout << "main_vizmo: opened  = " << path_filename << endl << flush;  
     
GetVizmo().ShowPathFrame(true);


    cout << "main_vizmo: writing snapshot file" << endl << flush;  

    // write snapshot file
    char snapshot_filename[SYNC_FILENAME_MAX_LEN];
    sync.GetFileNameWrite(snapshot_filename);

    // remove extension
    snapshot_filename[strlen(snapshot_filename) - 3] = 0;
    cout << "main_vizmo: writing snapshot file = " << 
      snapshot_filename << "\n" << flush;


  resetCamera(); //reset camera
    screenShotGUI->takeCampusSnapshot(snapshot_filename, MODULE_SNAPSHOT_EXT);
  }

   exit(0);

   //m_Argdsf


    
    return true;

}





/////////////////////////////////////////////////////////////////////
//
//  Protected
//
/////////////////////////////////////////////////////////////////////
bool VizmoMainWin::CreateGUI()
{
    ///////////////////////////////////////////////////////////////////////////
    animationGUI=new VizmoAnimationGUI(this);    
    connect(animationGUI,SIGNAL(callUpdate()),this,SLOT(updateScreen()));

    ///////////////////////////////////////////////////////////////////////////
    CreateShapeSelection();
    CreateActions();
    CreateMenubar();
    CreateToolbar();
    SetTips();
    CreateScreenCapture();
    CreateObjectSelection();
    //CreateAttributeSelection();

    connect(m_GL, SIGNAL(selectByRMB()), this, SLOT(contexmenu()));
    connect(m_GL, SIGNAL(selected()), objectSelection, SLOT(select()));

    return true;
}

void VizmoMainWin::keyPressEvent ( QKeyEvent * e )
{
    switch(e->key()){
    case Qt::Key_Escape: qApp->quit();
    }
}

void VizmoMainWin::reset()
{
    if( showHideRoadmapAction!=NULL ){
        if( !GetVizmo().IsRoadMapLoaded() ){
            showHideRoadmapAction->setEnabled(false);
            roadmapButton->setEnabled(false);
        }
        else {
            showHideRoadmapAction->setEnabled(true);
            roadmapButton->setEnabled(true);
        }
    }

    if( showHidePathAction!=NULL ){
        if( GetVizmo().GetPathSize()==0 ){
            showHidePathAction->setEnabled(false);
            pathButton->setEnabled(false);
        }
        else {
            showHidePathAction->setEnabled(true);
            pathButton->setEnabled(true);
        }
    }

    if( showHideSGaction!=NULL ){
        if( !GetVizmo().IsQueryLoaded() ){
            showHideSGaction->setEnabled(false);
            strtGoalButton->setEnabled(false);
        }
        else{
            showHideSGaction->setEnabled(true);
            strtGoalButton->setEnabled(true);
        }
    }
}

/////////////////////////////////////////////////////////////////////
//
//  Slots
//
/////////////////////////////////////////////////////////////////////
void VizmoMainWin::load()
{
    QString fn = QFileDialog::getOpenFileName( QString::null, 
        "OBPRM Files (*.map *.env)" , this);
    QFileInfo fi(fn);
    
    if ( !fn.isEmpty() ){
        m_Args.push_back(fn.latin1());
        m_bVizmoInit=false;
        setCaption("Environment : "+fi.baseName());
        statusBar()->message( "File Loaded : "+fn );
    }
    else statusBar()->message( "Loading aborted" );
}

void VizmoMainWin::resetCamera()
{
    m_GL->resetCamera();
    m_GL->updateGL();
}

void VizmoMainWin::showmap()          //show roadmap
{
    static bool show=false;
    show=!show;
    GetVizmo().ShowRoadMap(show);
    m_GL->updateGL();
}

void VizmoMainWin::showpath()      //show path frame
{
    static bool show=false;
    show=!show;
    GetVizmo().ShowPathFrame(show);
    m_GL->updateGL();
}

void VizmoMainWin::showstartgoal() //show start and goal position
{
    static bool show=false;
    show=!show;
    
    GetVizmo().ShowQueryFrame(show);
    m_GL->updateGL();
}


//BSS - Bounding Bos
void VizmoMainWin::showBBox()
{
    static bool show=true;  // The box is shown when the file is loaded
    show=!show;
    GetVizmo().ShowBBox(show);
    m_GL->updateGL();
}


void VizmoMainWin::contexmenu()
{
    QPopupMenu cm(this,"contex");
    cm.insertItem( "Solid",this,SLOT(setSolid()) );
    cm.insertItem( "Wire", this,SLOT(setWire()));
    cm.insertItem( "Invisible",this,SLOT(setInvisible()));
    
    cm.exec(QCursor::pos());
    m_GL->updateGL();    
}

void VizmoMainWin::setSolid()
{
    GetVizmo().ChangeAppearance(0); 
}

void VizmoMainWin::setWire()
{
    GetVizmo().ChangeAppearance(1); 
}

void VizmoMainWin::setInvisible()
{
    GetVizmo().ChangeAppearance(2); 
}

void VizmoMainWin::refreshEnv()
{
    GetVizmo().RefreshEnv();
    m_GL->updateGL();
}

void VizmoMainWin::updateScreen()
{
    m_GL->updateGL();
}

void VizmoMainWin::getOpenglSize(int *w,int *h)
{
    m_GL->getWidthHeight(w,h);
}

void VizmoMainWin::about()
{
    QMessageBox::about
        (this,"Vizmo++\n",
        "A 3D Vizualiztion tool\n"
        "Authors:\n"
        "Jyh-Ming Lien\n"
        "Aimee Vargas Estrada\n"
        "Bharatinder Singh Sandhu\n"
        );
    //pop up an about dialog
}

void VizmoMainWin::notimp()
{
    QMessageBox::information
        (this,"Ouch!!",
        "This function will be implemented very soon!!!!",QMessageBox::Ok,QMessageBox::NoButton);
}

void VizmoMainWin::changecolor(){
    
    QColor color = QColorDialog::getColor( white, this, "color dialog" );
    if ( color.isValid() ){
        R = (double)(color.red()) / 255.0;
        G = (double)(color.green()) / 255.0;
        B = (double)(color.blue()) / 255.0;
        
        m_GL->R = R;
        m_GL->G = G;
        m_GL->B  = B;
        m_GL->updateGL();
    }
    
}

///////////////////////////////////////////////////////////////////////////////
//
// GUI creation 
//
///////////////////////////////////////////////////////////////////////////////

bool VizmoMainWin::CreateActions()
{
    ///////////////////////////////////////////////////////////////////////////////
    // File Open
    fileOpenAction = new QAction( "Open File", QPixmap( Folder ), "&Open", CTRL+Key_O, this, "open" );
    connect( fileOpenAction, SIGNAL( activated() ) , this, SLOT( load() ) );
    
    ///////////////////////////////////////////////////////////////////////////////
    // Show/Hide Roadmap
    //showHideRoadmapAction = new QAction( "Show/Hide Roadmap", QPixmap(Board), "&Show/Hide",  CTRL+Key_S, this, "show hide road",true);
    showHideRoadmapAction = new QAction( "Show/Hide Roadmap", QPixmap(navigate), "&Show/Hide",  CTRL+Key_S, this, "show hide road",true);
    connect(showHideRoadmapAction, SIGNAL(activated()), this, SLOT(showmap()) );
    showHideRoadmapAction->setEnabled(false);

    ///////////////////////////////////////////////////////////////////////////////
    // Show/Hide Path
    showHidePathAction = new QAction("Show/Hide Path", QPixmap(Pen), "&Show/Hide", CTRL+Key_S, this, "show hide path",true);
    connect(showHidePathAction, SIGNAL(activated()), this, SLOT(showpath()));
    showHidePathAction->setEnabled(false);

    ///////////////////////////////////////////////////////////////////////////////
    // Show/Hide Start and Goal
    showHideSGaction = new QAction("Show/Hide Start/Goal", QPixmap(Flag), "&Show/HIde", CTRL+Key_S, this, "show hide robot",true);
    connect(showHideSGaction, SIGNAL(activated()), this, SLOT(showstartgoal()));
    showHideSGaction->setEnabled(false);

    ///////////////////////////////////////////////////////////////////////////////
    // Reset Camera View
    cameraResetAction = new QAction( "Reset Camera", QPixmap( Camera ), "&Reset", CTRL+Key_R, this, "reset" );
    connect( cameraResetAction, SIGNAL(activated()) , this, SLOT(resetCamera()));
    
    ///////////////////////////////////////////////////////////////////////////////
    // Quit
    quitAction = new QAction("Quit", "&Quit", CTRL+Key_Q, this, "quit");
    connect(quitAction, SIGNAL( activated() ) , qApp, SLOT( closeAllWindows() ) );

    ///////////////////////////////////////////////////////////////////////////////
    // Change background Color
    changeColorAction = new QAction("ColorPalette", QPixmap( pallet ), " Color Pale&tte", CTRL+Key_T, this, "color palette");
    connect(changeColorAction, SIGNAL(activated()), this, SLOT(changecolor()));
    
    ///////////////////////////////////////////////////////////////////////////////
    // Change robot's size
    /*
    changeRobotSizeAction = new QAction("Change size", QPixmap( pallet ), 
                            "Change Si&ze", CTRL+Key_Z, this, "change size");
    connect(changeRobotSizeAction, SIGNAL(activated()), shapeSelection, SLOT(changeSize()));
    */
    
    return true;
}

void VizmoMainWin::SetTips(){
    
    (void)QWhatsThis::whatsThisButton( vizmoTools );
    
    const char * roadmapText = "<p>Click this button to visualize the <em>Roadmap</em>. <br>"
        "You can also select the <b>Show/Hide</b> option "
        "from the <b>Roadmap</b> menu.</p>";
    QWhatsThis::add( roadmapButton, roadmapText );   
    
    const char * pathText = "<p>Click this button to visualize the <em>Path</em>. <br>"
        "You can also select the <b>Show/Hide</b> option  "
        "from the <b>Path</b> menu.</p>";
    
    QWhatsThis::add( pathButton, pathText );
    
    const char * startGoalText = "<p>Click this button to visualize the <em>Start and Goal cfgs.</em><br> "
        "You can also select the <b>Show/Hide</b> option "
        "from the <b>Path</b> menu.";
    
    QWhatsThis::add( strtGoalButton, startGoalText );
    
    const char * changeColorText = "<p>Click this button to change the background color";
    
    QWhatsThis::add( palletButton, changeColorText );
}

void VizmoMainWin::CreateToolbar(){
    
    QPixmap folderIcon, pathIcon, cameraIcon, roadmapIcon, strtGoalIcon, palletIcon;
    
    vizmoTools = new QToolBar( this, "vizmo operations" );
    vizmoTools->setLabel( "Vizmo Operations" );
    
    //////////////////////////
    /// create buttons
    //////////////////////////
    folderIcon = QPixmap( Folder );
    folderButton = new QToolButton(folderIcon, "File", "Open File", this,
        SLOT(load()), vizmoTools, "file");
    //folderButton->setToggleButton(true);
    folderButton->setUsesTextLabel ( true );
    
    vizmoTools->addSeparator();
    
    //roadmapIcon = QPixmap(Board);
    roadmapIcon = QPixmap(navigate);
    roadmapButton = new QToolButton(roadmapIcon, "Roadmap", "Load Roadmap",
        this, SLOT(showmap()), vizmoTools, "roadmap");
    roadmapButton->setToggleButton(true);
    roadmapButton->setUsesTextLabel ( true );
    roadmapButton->setEnabled(false);
    
    pathIcon = QPixmap( Pen);
    pathButton = new QToolButton( pathIcon, "Path", "Load Path",
        this, SLOT(showpath()), vizmoTools, "path" );
    pathButton->setToggleButton(true);
    pathButton->setUsesTextLabel ( true );
    pathButton->setEnabled(false);
    
    strtGoalIcon = QPixmap( Flag);
    strtGoalButton = new QToolButton(strtGoalIcon, "Start/Goal", "Load Start/Goal posotions",this, SLOT(showstartgoal()), vizmoTools, "start goal" );
    strtGoalButton->setToggleButton(true);
    strtGoalButton->setUsesTextLabel ( true );
    strtGoalButton->setEnabled(false);
    
    vizmoTools->addSeparator(); 
    
    cameraIcon = QPixmap(Camera);
    cameraButton = new QToolButton(cameraIcon, "Camera", " Reset Camera", this,
        SLOT(resetCamera()), vizmoTools, "Camera");
    cameraButton->setUsesTextLabel ( true );
    
    palletIcon = QPixmap(pallet);
    palletButton = new QToolButton(palletIcon, "BgColor", "Change Backgroundg Color", this,SLOT(changecolor()), vizmoTools, "background color");
    palletButton->setUsesTextLabel ( true );
}

void VizmoMainWin::CreateMenubar()
{ 
    //create FILE menu
    
    QPopupMenu * file = new QPopupMenu( this );
    menuBar()->insertItem( "&File", file );
    
    fileOpenAction->addTo( file );
    quitAction->addTo( file );
    
    ////////////////////////////////////////////// 
    // ROADMAP menu
    // first create submenus: param
    //////////////////////////////////////////////
    
    QPopupMenu * param = new QPopupMenu( this );
    param->insertTearOffHandle();
    //param->insertItem(  "Change size of nodes", this, SLOT(changeSize()));
    int numberId = param->insertItem( "Number of &nodes", this, SLOT(notimp()) );
    int defId =param->insertItem( "&Define start/goal", this, SLOT(notimp()) );
    param->setItemEnabled( numberId, FALSE );
    param->setItemEnabled( defId, FALSE );
    
    QPopupMenu * roadmapMenu = new QPopupMenu( this );
    menuBar()->insertItem( "Road&map", roadmapMenu );
    
    showHideRoadmapAction->addTo( roadmapMenu);
    roadmapMenu->insertSeparator();
    int genId = roadmapMenu->insertItem( "&Generate...", this, SLOT(notimp()),  CTRL+Key_G );
    roadmapMenu->setItemEnabled( genId, FALSE );
    roadmapMenu->insertSeparator();
    roadmapMenu->insertItem( "S&et parameters", param, CTRL+Key_E );
    roadmapMenu->insertSeparator();
    
    /////////////////////////////////////////////////
    // PATH menu
    // first create submenus: opt
    ////////////////////////////////////////////////
    
    QPopupMenu * opt = new QPopupMenu( this );
    opt->insertTearOffHandle();
    opt->insertItem( "S&mooth", this, SLOT(notimp()) );
    
    QPopupMenu * pathMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&Path", pathMenu );
    
    showHidePathAction->addTo( pathMenu );
    pathMenu->insertSeparator();
    pathMenu->insertItem( "&Optimization", opt, CTRL+Key_O );
    pathMenu->insertSeparator();
    
    ///////////////////////////////////////////////
    // ROBOT menu
    // first create submenus: robotRigid, robotArt
    ///////////////////////////////////////////////
    
    QPopupMenu * robotRigid = new QPopupMenu( this );
    robotRigid->insertTearOffHandle();
    robotRigid->insertItem(  "Change start/goal position", this, SLOT(notimp()));
    // robotRigid->insertItem(  "Show/Hide", this, SLOT(notimp()));
    showHideSGaction->addTo( robotRigid );
    
    QPopupMenu * robotArt = new QPopupMenu( this );
    robotArt->insertTearOffHandle();
    robotArt->insertItem(  "Build", this, SLOT(notimp()) );
    robotArt->insertItem(  "Change start/goal position", this, SLOT(notimp()));
    robotArt->insertItem(  "Show/Hide", this, SLOT(notimp()));
    
    QPopupMenu * robotMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&Robot", robotMenu );
    
    robotMenu->insertItem( "R&igid", robotRigid, CTRL+Key_I );
    robotMenu->insertSeparator();
    robotMenu->insertItem( "&Articulated", robotArt, CTRL+Key_A );
    robotMenu->insertSeparator();
    
    /////////////////////////////////////////////  
    // ENVIRONMENT menu
    /////////////////////////////////////////////
    
    QPopupMenu * envMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&Environment", envMenu );
    envMenu->insertItem( "&Bounding Box", this, SLOT(showBBox()),  CTRL+Key_B );   
    envMenu->insertItem( "&Refresh", this, SLOT(refreshEnv()));   
    
    envMenu->insertSeparator();
    
    /////////////////////////////////////////////  
    // Scene menu
    /////////////////////////////////////////////
    
    QPopupMenu * sceneMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&Scene", sceneMenu );
    cameraResetAction->addTo( sceneMenu );
    
    
    //create HELP menu
    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertItem( "&Help", help );
    help->insertItem( "&About", this, SLOT(about()), Key_F1 );
    help->insertItem( "What's &This", this, SLOT(whatsThis()), SHIFT+Key_F1 );  
}



void VizmoMainWin::CreateScreenCapture()
{
    screenShotGUI=new VizmoScreenShotGUI(this,"Screen Shot");
    connect(screenShotGUI,SIGNAL(getScreenSize(int *,int *)),this,SLOT(getOpenglSize(int *,int *)));
    connect(screenShotGUI,SIGNAL(togleSelectionSignal()),m_GL,SLOT(togleSlectionSlot()));
    connect(screenShotGUI,SIGNAL(getBoxDimensions(int *,int *,int *,int *)),m_GL,SLOT(getBoxDimensions(int *,int *,int *,int *)));
    connect(screenShotGUI,SIGNAL(callUpdate()),this,SLOT(updateScreen()));
    connect(screenShotGUI,SIGNAL(goToFrame(int)),animationGUI,SLOT(goToFrame(int)));
}

void VizmoMainWin::CreateObjectSelection()
{
    objectSelection= new VizmoItemSelectionGUI(this,"ObjectSelection");
    connect(objectSelection,SIGNAL(callUpdate()),this,SLOT(updateScreen()));
}

void VizmoMainWin::CreateAttributeSelection()
{
    attributeSelection= new VizmoAttributeSelectionGUI(this,"AttributeSelection");
}

void VizmoMainWin::CreateShapeSelection()
{   

  //    shapeSelection = new VizmoRoadmapNodesShapeGUI(this, "ShapeSelection");
  //  connect(shapeSelection,SIGNAL(callUpdate()),this,SLOT(updateScreen()));

}






