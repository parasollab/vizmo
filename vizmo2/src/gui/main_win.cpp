///////////////////////////////////////////////////////////////////////////////
//Include Vizmo2 Headers
#include "vizmo2.h"
#include "main_win.h"  
#include "scene_win.h"
#include "animation_gui.h"
#include "snapshot_gui.h"
#include "itemselection_gui.h"
#include "roadmap.h"
#include "filelistDialog.h"
#include "obj_property.h"
#include "obprmGUI.h"
#include "addObjDialog.h"
#include "vizmoEditor.h"

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
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qcolordialog.h> 
#include <qslider.h>
#include <qlcdnumber.h>
#include <qtimer.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qvbox.h>

///////////////////////////////////////////////////////////////////////////////
//Icons

#include "icon/eye.xpm"
#include "icon/folder.xpm"
#include "icon/pen.xpm"
#include "icon/camera.xpm"
#include "icon/navigate.xpm"
#include "icon/flag.xpm"
#include "icon/pallet.xpm"
#include "icon/tapes.xpm"
#include "icon/crash_burn.xpm"
#include "icon/edit.xpm"
#include "icon/floppy2_green.xpm"
#include "icon/3D_yellow.xpm"
#include "icon/3D_blue.xpm"
#include "icon/3D_green.xpm"
#include "icon/update.xpm"
///////////////////////////////////////////////////////////////////////////////

VizmoMainWin::VizmoMainWin(QWidget * parent, const char * name)
:QMainWindow(parent, name), m_bVizmoInit(false)
{ 
    setMinimumSize( 800, 600 );
    setCaption("Vizmo++"); 
    m_GL=NULL;
    animationGUI=NULL;
    move(0,0);
}

bool VizmoMainWin::Init()
{
    this->setIcon(QPixmap(icon_eye));
    //m_ModelName=modelName;
    
    //Create GLModel
    if( (m_GL=new VizGLWin(this))==NULL ) return false;
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
    GetVizmo().GetAccessFiles(m_Args[0]);
	FileListDialog * flDialog=new FileListDialog(this,"Vizmo File List");
	if( flDialog->exec()!=QDialog::Accepted )
		return false;


    if( GetVizmo().InitVizmoObject()==false ){
        return false;
    }
    m_Args.clear();
    resetCamera(); //reset camera
    
    //reset guis
    animationGUI->reset();
    objectSelection->reset();
    screenShotGUI->reset();
    roadmapGUI->reset();

    reset();
    
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
    CreateActions();
    CreateToolbar();
    CreateScreenCapture();
    CreateObjectSelection();
    CreateRoadmapToolbar();
    CreateMenubar();
    SetTips();

    connect(m_GL, SIGNAL(selectByRMB()), this, SLOT(obj_contexmenu()));
    connect(m_GL, SIGNAL(clickByRMB()), this, SLOT(gen_contexmenu()));
    connect(m_GL, SIGNAL(selectByLMB()), objectSelection, SLOT(select()));
    connect(m_GL, SIGNAL(selectByLMB()), roadmapGUI, SLOT(handleSelect()));
    connect(m_GL, SIGNAL(MRbyGLI()), roadmapGUI, SLOT(MoveNode()));

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

    addObstacleAction->setEnabled(true);
    
    cameraButton->setEnabled(true);
    palletButton->setEnabled(true);
    envButton->setEnabled(true);
    CDButton->setEnabled(true);
    editorButton->setEnabled(true);
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
        setCaption("Vizmo++ - "+fi.baseName()+ " environment");
        statusBar()->message( "File Loaded : "+fn );
    }
    else statusBar()->message( "Loading aborted" );

    m_GL->resetTransTool();
    m_GL->updateGL();
}

void VizmoMainWin::updatefiles()
{
	m_bVizmoInit=false;
	FileListDialog * flDialog=new FileListDialog(this,"Vizmo File List");
	if( flDialog->exec()!=QDialog::Accepted )
		return;

    if( GetVizmo().InitVizmoObject()==false ){
        return;
    }
    
    //reset guis
    animationGUI->reset();
    objectSelection->reset();
    screenShotGUI->reset();
    roadmapGUI->reset();
    reset();

    m_GL->resetTransTool();

}

void VizmoMainWin::saveRoadmap()
{
  QString fn = QFileDialog::getSaveFileName( QString::null, 
					     "Files (*.map)" ,
					     this,
					     "save file dialog",
					     "Choose a filename to save under" );
  
  QFileInfo fi(fn);
  if ( !fn.isEmpty() ) {
    string filename = fn;
    const char* f;
    f = filename.c_str();
    //GetVizmo().SaveMap(f);
    roadmapGUI->SaveNewRoadmap(f);
  } else {
    statusBar()->message( "Saving aborted", 2000 );
  }
  m_GL->updateGL();

}

void VizmoMainWin::saveEnv()
{
  
  QString fn = QFileDialog::getSaveFileName( QString::null, 
					     "Files (*.env)" ,
					     this,
					     "save file dialog",
					     "Choose a filename to save under" );
  
  QFileInfo fi(fn);
  if ( !fn.isEmpty() ) {
    string filename = fn;
    const char* f;
    f = filename.c_str();
    GetVizmo().SaveEnv(f);
  } else {
    statusBar()->message( "Saving aborted", 2000 );
  }
  m_GL->updateGL();

}

void VizmoMainWin::saveQryFile()
{
  QString fn = QFileDialog::getSaveFileName( QString::null, 
					     "Files (*.query)" ,
					     this,
					     "save file dialog",
					     "Choose a filename to save under" );
  
  QFileInfo fi(fn);
  if ( !fn.isEmpty() ) {
    string filename = fn;
    const char* f;
    f = filename.c_str();
    GetVizmo().SaveQry(f);
  } else {
    statusBar()->message( "Saving aborted", 2000 );
  }
  m_GL->updateGL();

}

void VizmoMainWin::saveQryStart(){

  GetVizmo().SaveQryCfg('s');
}

void VizmoMainWin::saveQryGoal(){

  GetVizmo().SaveQryCfg('g');
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
	roadmapGUI->reset();
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

void VizmoMainWin::showBBox()
{
    static bool show=true;  // The box is shown when the file is loaded
    show=!show;
    GetVizmo().ShowBBox(show);
    m_GL->updateGL();
}

void VizmoMainWin::obj_contexmenu()
{
    QPopupMenu cm(this);
    cm.insertItem("Solid",this,SLOT(setSolid()) );
    cm.insertItem("Wire", this,SLOT(setWire()));


    //Create submenu to set start and goal configs.
    //create it just if Robot has been selected
    string m_s;
    typedef vector<gliObj>::iterator GIT;
    
    for(GIT ig= GetVizmo().GetSelectedItem().begin();ig!=GetVizmo().GetSelectedItem().end();ig++)
    {
        CGLModel * gl=(CGLModel *)(*ig);
        list<string> info=gl->GetInfo();

	m_s = info.front();
	
    }

    if(m_s == "Robot"){
      QPopupMenu * cfgs = new QPopupMenu( this );
      cfgs->insertTearOffHandle();
      cfgs->insertItem( "Save start", this, SLOT(saveQryStart()) );
      cfgs->insertItem( "Save goal", this, SLOT(saveQryGoal()) );

      cm.insertItem( "Set query", cfgs);
    }
    else
      cm.insertItem("Delete",this,SLOT(setInvisible()));

	
    cm.insertItem(QIconSet(icon_pallet),"Color", this, SLOT(setNewColor()));
    cm.insertSeparator();
    //cm.insertItem("Edit...", this,SLOT(objectEdit()));

    if( cm.exec(QCursor::pos())!=-1 ) m_GL->updateGL();    
}

void VizmoMainWin::gen_contexmenu()
{

  QPopupMenu cm(this);
  changeBGcolorAction->addTo(&cm);
  cameraResetAction->addTo(&cm);
  showGridAction->addTo(&cm);
  showAxisAction->addTo(&cm);

  if( cm.exec(QCursor::pos())!=-1 ) m_GL->updateGL();    

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
    objectSelection->reset();
}

void VizmoMainWin::setNewColor()
{
    QColor color = QColorDialog::getColor( white, this, "color dialog" );
    if ( color.isValid() ){
      GetVizmo().mR = (double)(color.red()) / 255.0;
      GetVizmo().mG = (double)(color.green()) / 255.0;
      GetVizmo().mB = (double)(color.blue()) / 255.0;
    }
    GetVizmo().ChangeAppearance(3);
}

void VizmoMainWin::envObjsRandomColor()
{
	GetVizmo().envObjsRandomColor();
	m_GL->updateGL();
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

void VizmoMainWin::objectEdit()
{
	InvokeObjPropertyDialog(this);
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
        "This function will be implemented very soon!!!!",
		QMessageBox::Ok,QMessageBox::NoButton);
}

void VizmoMainWin::changeBGcolor(){
    
    QColor color = QColorDialog::getColor(white, this);
    if ( color.isValid() ){
		m_GL->setClearColor(
			(double)(color.red()) / 255.0,
			(double)(color.green()) / 255.0,
			(double)(color.blue()) / 255.0);
        m_GL->updateGL();
    }
}


void VizmoMainWin::resetRobotPosition(){

  GetVizmo().ResetRobot();
  m_GL->updateGL();
}

void VizmoMainWin::enablDisablCD(){
  if(CDButton->isOn()){
    m_GL->CDOn = true;
    GetVizmo().TurnOn_CD();
    m_GL->updateGL();
  }
  else
    m_GL->CDOn = false;

}

void VizmoMainWin::runCode(){

  obprmGUI * obprmWindow = new obprmGUI(this, "OBPRM Interface");
  if( obprmWindow->exec()!=QDialog::Accepted )
    return;
}

void VizmoMainWin::createQryFile(){
  QStringList command;
  QFileInfo fi(GetVizmo().getEnvFileName().c_str());
  QString qryPath;

  QFileDialog* fd = new QFileDialog( this, "obprm_dialog", TRUE );
  fd->setMode( QFileDialog::AnyFile );
  fd->setShowHiddenFiles ( TRUE ); 
  
  if ( fd->exec() == QDialog::Accepted )
    qryPath = fd->selectedFile();

  QString s;
  s = qryPath;
  s.append(" -f ");
  s.append(fi.baseName());
  
  vizmoEditor *vizEditor = new vizmoEditor(this, "VizmoEditor");
  vizEditor->setCaption( "Query Editor" );
  vizEditor->e->setText(s);
  vizEditor->show();


}


void VizmoMainWin::createEditor(){

  vizmoEditor *vizEditor = new vizmoEditor(this, "VizmoEditor");
  vizEditor->setCaption( "Vizmo++ Editor" );
  vizEditor->show();
}

void VizmoMainWin::addObstacle(){

  AddObjDial * addObj = new AddObjDial(this, "Add Object");
  bool create = addObj->create();
  if(create){
    int r;
    r =   addObj->exec();
    //if(!r) return;
   objectSelection->reset();
  }
  else
    return;
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
    fileOpenAction = new QAction("Open File", QPixmap(icon_folder), "&Open", CTRL+Key_O, this);
    connect(fileOpenAction,SIGNAL(activated()), this, SLOT(load()) );

    ///////////////////////////////////////////////////////////////////////////////
    // Update Files  
    fileUpdateAction=new QAction("Update File", QPixmap(icon_update), "&Update", CTRL+Key_R, this);
    connect(fileUpdateAction,SIGNAL(activated()), this, SLOT(updatefiles()) );

    ///////////////////////////////////////////////////////////////////////////////
    // Save File
    fileSaveAction=new QAction("Save Environment", QPixmap(disk_yellow), "&Save Env.", CTRL+Key_S,  this);
    connect(fileSaveAction,SIGNAL(activated()), this, SLOT(saveEnv()));

    fileSaveQryAction=new QAction("Save Query", QPixmap(disk_green), "&Save Query", CTRL+Key_Q,  this);
    connect(fileSaveQryAction,SIGNAL(activated()), this, SLOT(saveQryFile()));

    fileSaveMapAction=new QAction("Save Roadmap", QPixmap(disk_blue), "&Save Roadmap", CTRL+Key_D,  this);
    connect(fileSaveMapAction,SIGNAL(activated()), this, SLOT(saveRoadmap()));

    ///////////////////////////////////////////////////////////////////////////////
    // Show/Hide Roadmap
    showHideRoadmapAction = new QAction( "Show/Hide Roadmap", QPixmap(icon_navigate), "Show/Hide",  CTRL+Key_M, this, "",true);
    connect(showHideRoadmapAction, SIGNAL(activated()), this, SLOT(showmap()) );
    showHideRoadmapAction->setEnabled(false);

    ///////////////////////////////////////////////////////////////////////////////
    // Show/Hide Path
    showHidePathAction = new QAction("Show/Hide Path", QPixmap(icon_pen), "Show/Hide", CTRL+Key_P, this, "",true);
    connect(showHidePathAction, SIGNAL(activated()), this, SLOT(showpath()));
    showHidePathAction->setEnabled(false);

    ///////////////////////////////////////////////////////////////////////////////
    // Show/Hide Start and Goal
    showHideSGaction = new QAction("Show/Hide Start/Goal", QPixmap(icon_flag), "Show/Hide", CTRL+Key_G, this, "",true);
    connect(showHideSGaction, SIGNAL(activated()), this, SLOT(showstartgoal()));
    showHideSGaction->setEnabled(false);

    ///////////////////////////////////////////////////////////////////////////////
    // Reset Camera View
    cameraResetAction = new QAction( "Reset Camera", QPixmap( icon_camera ), "&Reset", CTRL+Key_R, this);
    connect( cameraResetAction, SIGNAL(activated()) , this, SLOT(resetCamera()));
    
    ///////////////////////////////////////////////////////////////////////////////
    // Quit
    quitAction = new QAction("Quit", "&Quit", CTRL+Key_Q, this, "quit");
    connect(quitAction, SIGNAL( activated() ) , qApp, SLOT( closeAllWindows() ) );

    ///////////////////////////////////////////////////////////////////////////////
    // Change background Color
    changeBGcolorAction = new QAction("ColorPalette", QPixmap( icon_pallet ), "Bg &Color", CTRL+Key_C, this);
    connect(changeBGcolorAction, SIGNAL(activated()), this, SLOT(changeBGcolor()));

    ///////////////////////////////////////////////////////////////////////////////
    // Change Color randomly
    randObjcolorAction = new QAction("Random Color", QPixmap( icon_pallet ), "Randomly Changes Obstacle Colors", ALT+Key_C, this);
    connect(randObjcolorAction,SIGNAL(activated()),this,SLOT(envObjsRandomColor()));

    ///////////////////////////////////////////////////////////////////////////////
    // Axis 
	showAxisAction = new QAction(this,"",true); showAxisAction->setText("Axis");
	showAxisAction->setOn(true);
	connect(showAxisAction, SIGNAL(activated()), m_GL, SLOT(showAxis()));

    ///////////////////////////////////////////////////////////////////////////////
    // Grid
    showGridAction = new QAction(this,"",true); showGridAction->setText("Grid");
    showGridAction->setOn(true);
    connect(showGridAction, SIGNAL(activated()), m_GL, SLOT(showGrid()));
    
    ///////////////////////////////////////////////////////////////////////////////
    // Robot: set robot to initial Cfg.
    resetRobotPosAction=new QAction("Reset", QPixmap(icon_folder), "Reset&X", CTRL+Key_X,  this);
    connect(resetRobotPosAction,SIGNAL(activated()), this, SLOT(resetRobotPosition()));

    /////////////////////////////////////////////////////////////////////////////
    // Add an obstacle to the environment
    addObstacleAction = new QAction("Add Obstacle","Add Obs&tacle",CTRL+Key_T, this);
    connect(addObstacleAction, SIGNAL(activated()), this, SLOT(addObstacle()));
    addObstacleAction->setEnabled(false);

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
    
    const char * changeBGcolorText = "<p>Click this button to change the background color";
    
    QWhatsThis::add( palletButton, changeBGcolorText );


    const char * envObjsColor = "<P>Changes randomly the colors of the environment's objects";

    QWhatsThis::add( envButton, envObjsColor );
    
    const char * resetCamText = "<p>Click this button to reset camera position";

    QWhatsThis::add( cameraButton, resetCamText );

    const char * pictureText = "Click this button to take a picture of the environment";

   QWhatsThis::add(screenShotGUI->takePicture, pictureText );
 
    const char * CDText = "Click this button to enable/disable Collision Detection";

    QWhatsThis::add( CDButton, CDText );

    const char * editorText = "Click this button to open a <em>file</em>. <br>"
      "You can also execute <em>file</em> clicking the <b>Execute</b> button"
      "from the toolbar.</p>";

    QWhatsThis::add( editorButton, editorText );
 
}

void VizmoMainWin::CreateToolbar(){
    
    QPixmap folderIcon, pathIcon, cameraIcon, roadmapIcon, strtGoalIcon, palletIcon, envIcon,
            CDIcon;
    
    vizmoTools = new QToolBar( this, "vizmo operations" );
    vizmoTools->setLabel( "Vizmo Operations" );
    
    //////////////////////////
    /// create buttons
    //////////////////////////
    folderIcon = QPixmap( icon_folder );
    folderButton = new QToolButton(folderIcon, "File", "Open File", this,
        SLOT(load()), vizmoTools, "file");
    //folderButton->setToggleButton(true);
    folderButton->setUsesTextLabel ( true );
    
    vizmoTools->addSeparator();
    
    //roadmapIcon = QPixmap(Board);
    roadmapIcon = QPixmap(icon_navigate);
    roadmapButton = new QToolButton(roadmapIcon, "Roadmap", "Load Roadmap",
        this, SLOT(showmap()), vizmoTools, "roadmap");
    roadmapButton->setToggleButton(true);
    roadmapButton->setUsesTextLabel ( true );
    roadmapButton->setEnabled(false);
    
    pathIcon = QPixmap(icon_pen);
    pathButton = new QToolButton( pathIcon, "Path", "Load Path",
        this, SLOT(showpath()), vizmoTools, "path" );
    pathButton->setToggleButton(true);
    pathButton->setUsesTextLabel ( true );
    pathButton->setEnabled(false);
    
    strtGoalIcon = QPixmap(icon_flag);
    strtGoalButton = new QToolButton(strtGoalIcon, "Start/Goal", "Load Start/Goal positions",this, SLOT(showstartgoal()), vizmoTools, "start goal" );
    strtGoalButton->setToggleButton(true);
    strtGoalButton->setUsesTextLabel ( true );
    strtGoalButton->setEnabled(false);
    
    vizmoTools->addSeparator(); 
    
    cameraIcon = QPixmap(icon_camera);
    cameraButton = new QToolButton(cameraIcon, "Camera", " Reset Camera", this,
        SLOT(resetCamera()), vizmoTools, "Camera");
    cameraButton->setUsesTextLabel ( true );
    cameraButton->setEnabled(false);

    palletIcon = QPixmap(icon_pallet);
    palletButton = new QToolButton(palletIcon, "BgColor", "Change Background Color", this,
				   SLOT(changeBGcolor()), vizmoTools, "background color");
    palletButton->setUsesTextLabel ( true );
    palletButton->setEnabled(false);

    envIcon = QPixmap(icon_pallet);
    envButton = new QToolButton(envIcon, "Random Color", "Changes randomly Env.obj. color", 
				this, SLOT(envObjsRandomColor()),vizmoTools, "Env.");
    envButton->setUsesTextLabel ( true );
    envButton->setEnabled(false);

    //////////////////////////////////////////////////////////////////////////////
    // Collision Detection: enable/disable
    
    CDIcon = QPixmap(icon_crash_burn);
    CDButton = new QToolButton(CDIcon, "CollisionDetection", "Enable-Disable CD", 
			       this, SLOT(enablDisablCD()), vizmoTools, "CD");
    CDButton->setUsesTextLabel ( true );
    CDButton->setToggleButton(true);
    CDButton->setEnabled(false);

    vizmoTools->addSeparator(); 

    //////////////////////////////////////////////////////////////////////////////
    // Editor

    editorButton = new QToolButton(QPixmap( icon_edit),
				   "Editor", "VizmoEditor", this,
				   SLOT( createEditor() ), vizmoTools, "VizEdt");
    
    editorButton->setUsesTextLabel ( true );
    editorButton->setEnabled(false);
                     

}

void VizmoMainWin::CreateMenubar()
{ 
    //create FILE menu
    
    QPopupMenu * file = new QPopupMenu( this );
    menuBar()->insertItem( "&File", file );
    
    fileOpenAction->addTo( file );
    fileUpdateAction->addTo( file );
    fileSaveAction->addTo( file );
    fileSaveQryAction->addTo( file );
    fileSaveMapAction->addTo( file );
    quitAction->addTo( file );
    
    ////////////////////////////////////////////// 
    // ROADMAP menu
    // first create submenus: param
    //////////////////////////////////////////////
    
    QPopupMenu * param = new QPopupMenu( this );
    param->insertTearOffHandle();
    //param->insertItem(  "Change size of nodes", this, SLOT(changeSize()));
    int numberId = param->insertItem( "Number of nodes", this, SLOT(notimp()) );
    int defId =param->insertItem( "Define start/goal", this, SLOT(notimp()) );
    param->setItemEnabled( numberId, FALSE );
    param->setItemEnabled( defId, FALSE );
    
    QPopupMenu * roadmapMenu = new QPopupMenu( this );
    menuBar()->insertItem( "Road&map", roadmapMenu );
    
    showHideRoadmapAction->addTo(roadmapMenu);
    roadmapMenu->insertSeparator();
    roadmapGUI->editAction->addTo(roadmapMenu);
    //roadmapGUI->addNodeAction->addTo(roadmapMenu);
    //roadmapGUI->addEdgeAction->addTo(roadmapMenu);
    roadmapMenu->insertSeparator();
    //roadmapMenu->insertItem( "Set parameters", param);
    //roadmapMenu->insertSeparator();
    
    /////////////////////////////////////////////////
    // PATH menu
    // first create submenus: opt
    ////////////////////////////////////////////////
    
    //QPopupMenu * opt = new QPopupMenu( this );
    //opt->insertTearOffHandle();
    //opt->insertItem( "Smooth", this, SLOT(notimp()) );
    
    QPopupMenu * pathMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&Path", pathMenu );
    
    showHidePathAction->addTo( pathMenu );
    pathMenu->insertSeparator();
    //pathMenu->insertItem( "Optimization", opt);
    //pathMenu->insertSeparator();
    
    ///////////////////////////////////////////////
    // ROBOT menu
    // first create submenus: robotRigid, robotArt
    ///////////////////////////////////////////////
    
    //QPopupMenu * robotRigid = new QPopupMenu( this );
    //robotRigid->insertTearOffHandle();
    //robotRigid->insertItem(  "Change start/goal position", this, SLOT(notimp()));
    // robotRigid->insertItem(  "Show/Hide", this, SLOT(notimp()));
    //showHideSGaction->addTo( robotRigid );
    
    //QPopupMenu * robotArt = new QPopupMenu( this );
    //robotArt->insertTearOffHandle();
    //robotArt->insertItem(  "Build", this, SLOT(notimp()) );
    //robotArt->insertItem(  "Change start/goal position", this, SLOT(notimp()));
    //robotArt->insertItem(  "Show/Hide", this, SLOT(notimp()));
    
    QPopupMenu * robotMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&Robot", robotMenu );
    
    //robotMenu->insertItem( "Rigid", robotRigid);
    //robotMenu->insertSeparator();
    //robotMenu->insertItem( "Articulated", robotArt);
    //robotMenu->insertSeparator();

    resetRobotPosAction->addTo(robotMenu);
    
    /////////////////////////////////////////////  
    // ENVIRONMENT menu
    /////////////////////////////////////////////
    
    QPopupMenu *envMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&Environment", envMenu );
    envMenu->insertItem( "Refresh", this, SLOT(refreshEnv()));  
    envMenu->insertSeparator();
    changeBGcolorAction->addTo( envMenu );
    randObjcolorAction->addTo( envMenu );
    envMenu->insertSeparator();
    envMenu->insertItem( "&Bounding Box", this, SLOT(showBBox()),  CTRL+Key_B );   
    addObstacleAction->addTo( envMenu );  

    /////////////////////////////////////////////  
    // Scene menu
    /////////////////////////////////////////////
    
    QPopupMenu * sceneMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&Scene", sceneMenu );
    cameraResetAction->addTo( sceneMenu );
	showGridAction->addTo( sceneMenu );
	showAxisAction->addTo( sceneMenu );

    /////////////////////////////////////////////  
    // OBPRM menu
    /////////////////////////////////////////////

    QPopupMenu * obprmMenu = new QPopupMenu( this);
    menuBar()->insertItem("OBPRM", obprmMenu );
    obprmMenu->insertSeparator();
    obprmMenu->insertItem("Generate mkmp", this, SLOT(runCode()));
    obprmMenu->insertItem("Generate qry", this, SLOT(createQryFile()));

    /////////////////////////////////////////////  
    //create HELP menu
    /////////////////////////////////////////////

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
	//connect(objectSelection,SIGNAL(itemSelected()), this, SLOT(obj_contexmenu()));
}

void VizmoMainWin::CreateAttributeSelection()
{
    //attributeSelection= new VizmoAttributeSelectionGUI(this,"AttributeSelection");
}

void VizmoMainWin::CreateRoadmapToolbar(){
  roadmapGUI = new VizmoRoadmapGUI (this, "MapSelection");
  connect(roadmapGUI,SIGNAL(callUpdate()),this,SLOT(updateScreen()));
}


