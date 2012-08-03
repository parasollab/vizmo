///////////////////////////////////////////////////////////////////////////////
//Include Vizmo2 Headers
#include "vizmo2.h"
#include "MainWin.h"  
#include "SceneWin.h"
#include "AnimationGUI.h"
#include "SnapshotGUI.h"
#include "ItemSelectionGUI.h"
#include "Roadmap.h"
#include "TextGUI.h"     
#include "FileListDialog.h"
#include "ObjProperty.h"
#include "OBPRMGUI.h"
#include "AddObjDialog.h"
#include "VizmoEditor.h"
#include "QueryGUI.h"
#include <GL/gliCamera.h> 
#include <Point.h> 

//////////////////////////////////////////////////////////////////////
// Include Load headers
#include "EnvObj/PathLoader.h"
#include "EnvObj/QueryLoader.h"
#include "EnvObj/DebugModel.h"

///////////////////////////////////////////////////////////////////////////////
//Include Qt Headers

#include <q3whatsthis.h>

#include <q3popupmenu.h>

#include <q3listbox.h>

#include <q3vbox.h>
//Added by qt3to4:
#include <QKeyEvent>

#include <QtGui>
///////////////////////////////////////////////////////////////////////////////
//Icons

#include "icon/Eye.xpm"
#include "icon/Folder.xpm"
#include "icon/Pen.xpm"
#include "icon/TripodCamera.xpm"
#include "icon/Navigate.xpm"
#include "icon/Flag.xpm"
#include "icon/Pallet.xpm"
//#include "icon/tapes.xpm"
//#include "icon/CrashBurn.xpm"
//#include "icon/Edit.xpm"
//#include "icon/floppy2_green.xpm"
#include "icon/3DYellow.xpm"
#include "icon/3DBlue.xpm"
#include "icon/3DGreen.xpm"
#include "icon/Update.xpm"
#include "icon/FontEdit.xpm"
#include "icon/ResetCamera.xpm"
#include "icon/BgColor.xpm"
#include "icon/RandEnv.xpm" 
//#include "icon/Runner.xpm"

////////////////////////////////////////////////////////
/// CLASS
/// VizmoMainWin
////////////////////////////////////////////////////////

VizmoMainWin::VizmoMainWin(QWidget * parent, const char * name)
:Q3MainWindow(parent, name), m_bVizmoInit(false)
{ 
    setMinimumSize(850, 700);
    setCaption("Vizmo++"); 
    m_GL=NULL;
    animationGUI=animationDebugGUI=NULL;
    move(0,0);
    m_setQS = false;
    m_setQG = false;
    command = "";
}

bool VizmoMainWin::Init()
{
    this->setIcon(QPixmap(eye));
    //m_ModelName=modelName;
    
    //Create GLModel
    if((m_GL=new VizGLWin(this))==NULL) 
      return false;
    setCentralWidget(m_GL);
    
    //Create Other GUI
    if(CreateGUI()==false) 
      return false;
    
    statusBar()->message("Ready");
    return true;
}

bool VizmoMainWin::InitVizmo()
{
    if(m_bVizmoInit) 
      return true;

    m_bVizmoInit=true;

    if(m_Args.empty()) 
      return true; //nothing to init...
    /*
    Here we use the first argument, but in the future
    we should use all of them to load files.
    */
    GetVizmo().GetAccessFiles(m_Args[0]);
	FileListDialog * flDialog = new FileListDialog(this,"Vizmo File List");
	if(flDialog->exec()!=QDialog::Accepted)
		return false;


    if(GetVizmo().InitVizmoObject()==false){
        return false;
    }
    resize(width(),height());
    m_Args.clear();
    resetCamera(); //reset camera
    //reset guis
    animationGUI->reset();
    animationDebugGUI->reset();
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
    animationGUI=new VizmoAnimationGUI("Path", this);    
    connect(animationGUI,SIGNAL(callUpdate()),this,SLOT(updateScreen()));

    animationDebugGUI=new VizmoAnimationGUI("Debug", this);    
    connect(animationDebugGUI,SIGNAL(callUpdate()),this,SLOT(updateScreen()));

    ///////////////////////////////////////////////////////////////////////////
    CreateActions();
    CreateToolbar();
    //CreateRoadmapToolbar(); 
    CreateScreenCapture();
    CreateObjectSelection();
    CreateRoadmapToolbar();
    CreateMenubar();
    CreateTextOutbox();    
    SetTips();


    connect(animationDebugGUI,SIGNAL(callUpdate()), m_outbox, SLOT(SetText()));
    connect(m_GL, SIGNAL(selectByRMB()), this, SLOT(obj_contexmenu()));
    connect(m_GL, SIGNAL(clickByRMB()), this, SLOT(gen_contexmenu()));
    connect(m_GL, SIGNAL(selectByLMB()), objectSelection, SLOT(select()));
    connect(m_GL, SIGNAL(selectByLMB()), roadmapGUI, SLOT(handleSelect()));
    connect(m_GL, SIGNAL(clickByLMB()), m_outbox, SLOT(SetText()));
    connect(m_GL, SIGNAL(selectByLMB()), m_outbox, SLOT(SetText()));
    connect(m_GL, SIGNAL(MRbyGLI()), roadmapGUI, SLOT(printRobCfg()));
 
    return true;
}

void VizmoMainWin::keyPressEvent (QKeyEvent* e)
{
    switch(e->key()){
		case Qt::Key_Escape: qApp->quit();
    }
}

void VizmoMainWin::reset()
{
    if(showHideRoadmapAction!=NULL){
        if(!GetVizmo().IsRoadMapLoaded()){
            showHideRoadmapAction->setEnabled(false);
            roadmapButton->setEnabled(false);
        }
        else {
            showHideRoadmapAction->setEnabled(true);
            roadmapButton->setEnabled(true);
        }
    }

    if(showHidePathAction!=NULL){
        if(GetVizmo().GetPathSize()==0){
            showHidePathAction->setEnabled(false);
            pathButton->setEnabled(false);
        }
        else {
            showHidePathAction->setEnabled(true);
            pathButton->setEnabled(true);
            m_pathOptionsAction->setEnabled(true); 
        }
    }

    if(m_showHideSGAction!=NULL){
        if(!GetVizmo().IsQueryLoaded()){
            m_showHideSGAction->setEnabled(false);
            strtGoalButton->setEnabled(false);
        }
        else{
            m_showHideSGAction->setEnabled(true);
            strtGoalButton->setEnabled(true);
        }
    }

    addObstacleAction->setEnabled(true);
    
    cameraButton->setEnabled(true);
    palletButton->setEnabled(true);
    envButton->setEnabled(true);
    //CDButton->setEnabled(true);
    //CDButton->setOn (FALSE);
    //editorButton->setEnabled(true);
    roadmapGUI->l_robCfg->clear();
    //mkmpButton->setEnabled(true);
    //qryButton->setEnabled(true);

    changeBGcolorAction->setEnabled(true);
    randObjcolorAction->setEnabled(true);
    envMenu->setItemEnabled(refreshID , TRUE);
    envMenu->setItemEnabled(bboxID , TRUE);

    robotCfgAction->setEnabled(false);
    robotCfgAction->setOn(false);
    resetRobotPosAction->setEnabled(true);

    fileUpdateAction->setEnabled(true);
    fileSaveAction->setEnabled(true);
    fileSaveMapAction->setEnabled(true);
    fileSaveQryAction->setEnabled(true);

    setQrySAction->setEnabled(true);
    setQryGAction->setEnabled(true);

    GetVizmo().ChangeNodesRandomColor();

}

/////////////////////////////////////////////////////////////////////
//
//  Slots
//
/////////////////////////////////////////////////////////////////////
void VizmoMainWin::load()
{
//     QString fn = Q3FileDialog::getOpenFileName(QString::null, 
//         "Files (*.map *.env)" , this);

    QString fn = QFileDialog::getOpenFileName(this,  "Choose an environment to open",  
	     QString::null,"Files (*.map *.env *.vd)");

    QFileInfo fi(fn);

    if (!fn.isEmpty()){
        m_Args.push_back(fn.latin1());
        m_bVizmoInit=false;
        setCaption("Vizmo++ - "+fi.baseName()+ " environment");
        statusBar()->message("File Loaded : "+fn);
    }
    else statusBar()->message("Loading aborted");

    m_pathOptionsInput->m_colors.clear(); //reset path gradient   
    m_GL->resetTransTool();
    m_GL->updateGL();
}

void VizmoMainWin::updatefiles()
{
	m_bVizmoInit=false;
	FileListDialog * flDialog=new FileListDialog(this,"Vizmo File List");
	if(flDialog->exec()!=QDialog::Accepted)
		return;

    if(GetVizmo().InitVizmoObject()==false){
        return;
    }
    
    //reset guis
    animationGUI->reset();
    animationDebugGUI->reset();
    objectSelection->reset();
    screenShotGUI->reset();
    roadmapGUI->reset();
    reset();

    m_GL->resetTransTool();

}

void VizmoMainWin::saveRoadmap()
{
  QString fn = QFileDialog::getSaveFileName(this, "Choose a filename for the roadmap",
					     QString::null, 
					     "Files (*.map)");
  
  QFileInfo fi(fn);
  if (!fn.isEmpty()) {
    string filename = fn.toStdString() ;
    const char* f;
    f = filename.c_str();
    //GetVizmo().SaveMap(f);
    roadmapGUI->SaveNewRoadmap(f);
  } else {
    statusBar()->message("Saving aborted", 2000);
  }
  m_GL->updateGL();

}

void VizmoMainWin::saveEnv()
{
  
  QString fn = QFileDialog::getSaveFileName(this, "Choose a filename for the environment",
					     QString::null, 
					     "Files (*.env)");
  
  QFileInfo fi(fn);
  if (!fn.isEmpty()) {
    string filename = fn.toStdString();
    const char* f;
    f = filename.c_str();
    GetVizmo().SaveEnv(f);
  } else {
    statusBar()->message("Saving aborted", 2000);
  }
  m_GL->updateGL();

}

void VizmoMainWin::saveQryFile()
{
  QString fn = QFileDialog::getSaveFileName(this, "Choose a filename to save the query",
					     QString::null, 
					     "Files (*.query)");
  
  QFileInfo fi(fn);
  if (!fn.isEmpty()) {
    string filename = fn.toStdString();
    const char* f;
    f = filename.c_str();
    GetVizmo().SaveQry(f);
  } else {
    statusBar()->message("Saving aborted", 2000);
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

void  
VizmoMainWin::SetCameraPos(){
  
  Point3d p = gliGetCameraFactory().getCurrentCamera()->getCameraPos();
  //vector<double> ptVect; 
  //p.get(ptVect);
  //Unfortunately, points are defined backwards for x and y
  //We want "X=3" to mean that VIEWER has moved to X=3 
  if(p[0] != 0) //otherwise displays '-0' ! 
    p[0] = -1*p[0];
  if(p[1] != 0)
    p[1] = -1*p[1]; 
  
  double azim = gliGetCameraFactory().getCurrentCamera()->getCurrentAzim();  
  double elev = gliGetCameraFactory().getCurrentCamera()->getCurrentElev();

  ostringstream ossX, ossY, ossZ, ossAzim, ossElev;  
  ossX << p[0]; 
  ossY << p[1]; 
  ossZ << p[2]; 
  ossAzim << azim; 
  ossElev << elev; 

  QString QsX((ossX.str()).c_str()); 
  QString QsY((ossY.str()).c_str()); 
  QString QsZ((ossZ.str()).c_str()); 
  QString QsAzim((ossAzim.str()).c_str());
  QString QsElev((ossElev.str()).c_str()); 

  (m_cameraPosInput->m_xLineEdit)->setText(QsX); 
  (m_cameraPosInput->m_yLineEdit)->setText(QsY); 
  (m_cameraPosInput->m_zLineEdit)->setText(QsZ); 
  (m_cameraPosInput->m_azimLineEdit)->setText(QsAzim); 
  (m_cameraPosInput->m_elevLineEdit)->setText(QsElev); 
  
  m_cameraPosInput->show();
}

void VizmoMainWin::showmap()          //show roadmap
{
    static bool show=false;
    show=!show;
    GetVizmo().ShowRoadMap(show);
    roadmapGUI->reset();
    typedef CMapModel<CCfg,Edge> MM;
    typedef CCModel<CCfg,Edge> CC;
    typedef vector<CC*>::iterator CCIT;
    CMapModel<CCfg,Edge>* mmodel =(MM*)GetVizmo().GetMap()->getModel();
    vector<CC*>& cc=mmodel->GetCCModels();
      for(CCIT ic=cc.begin();ic!=cc.end();ic++){
            (*ic)->DrawSelect();
            (*ic)->newColor = true;
            (*ic)->ReBuildAll();            
            (*ic)->DrawRobotNodes(GL_RENDER);
            (*ic)->DrawSelect();
      }
    GetVizmo().UpdateSelection();
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

void 
VizmoMainWin::PathDisplayOptions(){//change path gradient colors, etc. 
  m_pathOptionsInput->show(); 
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
    Q3PopupMenu cm(this);
    cm.insertItem("Solid",this,SLOT(setSolid()));
    cm.insertItem("Wire", this,SLOT(setWire()));


    //Create submenu to set start and goal configs.
    //create it just if Robot has been selected
    string m_s;
    typedef vector<gliObj>::iterator GIT;
    
    for(GIT ig= GetVizmo().GetSelectedItem().begin();ig!=GetVizmo().GetSelectedItem().end();ig++)
    {
        CGLModel * gl=(CGLModel *)(*ig);
        vector<string> info=gl->GetInfo();

	m_s = info.front();
    }

    if(m_s == "Robot"){
      Q3PopupMenu * cfgs = new Q3PopupMenu(this);
      cfgs->insertTearOffHandle();
      cfgs->insertItem("Save start", this, SLOT(saveQryStart()));
      cfgs->insertItem("Save goal", this, SLOT(saveQryGoal()));

      cm.insertItem("Set query", cfgs);
    }
    else
      cm.insertItem("Delete",this,SLOT(setInvisible()));

	
    cm.insertItem(QIcon(pallet),"Color", this, SLOT(setNewColor()));
    cm.insertSeparator();
    //cm.insertItem("Edit...", this,SLOT(objectEdit()));

    if(cm.exec(QCursor::pos())!= -1) 
      m_GL->updateGL();    
}

void VizmoMainWin::gen_contexmenu()
{

  Q3PopupMenu cm(this);
  changeBGcolorAction->addTo(&cm);
  cameraResetAction->addTo(&cm);
  m_cameraPositionAction->addTo(&cm); 
  showGridAction->addTo(&cm);
  showAxisAction->addTo(&cm);

  if(cm.exec(QCursor::pos())!=-1) 
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
    objectSelection->reset();
}

void VizmoMainWin::setNewColor()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "color dialog");
    if (color.isValid()){
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
    objectSelection->reset();
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
    
    QColor color = QColorDialog::getColor(Qt::white, this);
    if (color.isValid()){
		m_GL->setClearColor(
			(double)(color.red()) / 255.0,
			(double)(color.green()) / 255.0,
			(double)(color.blue()) / 255.0);
        m_GL->updateGL();
    }
}


void VizmoMainWin::resetRobotPosition(){

  GetVizmo().ResetRobot();
  animationGUI->reset();
  animationDebugGUI->reset();
  m_GL->updateGL();
}

void VizmoMainWin::showCfg(){
    if(robotCfgAction->isOn()){
      roadmapGUI->robCfgOn = true;
      if(roadmapGUI->l_cfg->isHidden())
	roadmapGUI->l_cfg->show();
      if(roadmapGUI->l_robCfg->isHidden())
	roadmapGUI->l_robCfg->show();
    }
    else{
      roadmapGUI->robCfgOn = false;
      roadmapGUI->l_cfg->hide();
      roadmapGUI->l_robCfg->hide();
    }
}

/*void VizmoMainWin::enablDisablCD(){
  if(CDButton->isOn()){
    m_GL->CDOn = true;
    //GetVizmo().TurnOn_CD();
    //m_GL->updateGL();
  }
  else
    m_GL->CDOn = false;

}*/  

void VizmoMainWin::runCode(){

  obprmGUI * obprmWindow = new obprmGUI(this, "OBPRM Interface");
  obprmWindow->caller = "runCode";
  if(obprmWindow->exec()!= QDialog::Accepted)
    return;
}

void VizmoMainWin::createQryFile(){
  QStringList command;
  QFileInfo fi(GetVizmo().getEnvFileName().c_str());
  QString qryPath;

  //Users will give the name of the qry executable
  //we assume it is in the PATH

  QDialog *dial = new QDialog(this);
  QPushButton *button1 = new QPushButton("Generate");
  connect(button1, SIGNAL(clicked()), dial, SLOT(accept()));
  QWidget *main = new QWidget(dial);
  QLineEdit *l_edit = new QLineEdit;

  QVBoxLayout *lay = new QVBoxLayout;
  lay->addWidget(l_edit);
  lay->addWidget(button1);

  main->setLayout(lay);
  main->show();
  dial->exec();

  QString s;
  s = l_edit->text();
  s.append(" -f ");
  s.append(fi.baseName());
  
  vizmoEditor *vizEditor = new vizmoEditor(this, "VizmoEditor");
  vizEditor->setCaption("Query Editor");
  vizEditor->e->setText(s);
  vizEditor->show();


}


/*void VizmoMainWin::createEditor(){

  vizmoEditor *vizEditor = new vizmoEditor(this, "VizmoEditor");
  vizEditor->setCaption("Vizmo++ Editor");
  vizEditor->show();
}*/ 

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

/*void VizmoMainWin::autoMkmp(){

  bool changed = false;
  string sub, mapFile, randName, dir;
  //const char * fileName;
  QString newComm;
  obprmGUI * obprmLine;
  string envFile = GetVizmo().getEnvFileName(); 

  //Ask if the environment has changed
  //if so, then save that environment to a new
  //one with random name so it can be used as input
  //to generate new roadmap.

 
  if(GetVizmo().envChanged()){ 
    //find out dir name
    int p=envFile.rfind('/');
    dir=envFile.substr(0,p);
    //generate random number
    int randNum = rand(); 
    std::ostringstream Out;
    Out << randNum;
    //create env file name and save it
    randName = "Env"+Out.str()+".env";
    GetVizmo().SaveEnv(randName.c_str());
    changed = true;
  }

  if(!changed){ 
    //get env. file name to use it for map file name
    int pos = envFile.find(".", 0);
    sub = envFile.substr(0, pos);
  }
  else{ 
    string randSub;
    int sPos = randName.find(".", 0);
    randSub = randName.substr(0, sPos);
    //sub = dir+"/"+randSub;
    sub = randSub;

  }
  if (command == ""){
    obprmLine = new obprmGUI(this, "OBPRM Interface");
    obprmLine->caller = "mkmp";
    if(obprmLine->exec()!=QDialog::Accepted)
      return;
    
    obprm_comm = obprmLine->command;
    int id =0;
    for (QStringList::Iterator it = obprm_comm.begin(); it != obprm_comm.end(); ++it) {
      if(id == 2){ // file name 
	newComm += (sub.c_str());
      }
      else
	newComm+= *it;
      id ++;
    }
  }
  else{
    int id =0;

    QStringList m_commandList = GetVizmo().getCommLine();
    if (m_commandList.size() !=0){
      for (QStringList::Iterator it = obprm_comm.begin(); it != obprm_comm.end(); ++it) {
	if(id == 2) // file name 
	  newComm+= (sub.c_str());
	else
	  newComm+= *it;
	id ++;
      }
    }
    else{
      for (QStringList::Iterator it = obprm_comm.begin(); it != obprm_comm.end(); ++it) {
	if(id == 2) // file name 
	  newComm+= (sub.c_str());
	else
	  newComm+= *it;
	id ++;
      }
    }
  } 
  
  command = newComm.toStdString () ;
  
  PlumObject * m_Map;
  m_Map = GetVizmo().GetMap();
  if(m_Map != NULL){
    CMapHeaderLoader * maploader=(CMapHeaderLoader*)m_Map->getLoader();
    if(maploader->ParseHeader()){
       mapFile = dir+"/"+sub +"New.map";
       command += "\\\n -outmapFile " + mapFile;
    }
  }
  else{
    mapFile = sub+".map";
  }
  cout << "Command " << command << endl;

  int i = system(command.c_str());

  if (i != -1){
    cout<< "\n\nNew map name = " <<mapFile<<endl;
    if(changed)
      GetVizmo().setEnvFileName(dir+"/"+randName);

    GetVizmo().setMapFileName(mapFile);
    
    GetVizmo().InitVizmoObject();
    //showmap();
    showHideRoadmapAction->setEnabled(true);
    roadmapButton->setEnabled(true);
  }

}*/ //End of autoMkmp function 

/*void VizmoMainWin::autoQry(){

  string sub, envName, randName, dir;
  int querySaved=0;

  //find out dir and name of ENV  
  string envFile = GetVizmo().getEnvFileName(); 
  int p=envFile.rfind('/');
  dir=envFile.substr(0,p);
  
  string tmp;
  tmp = envFile.substr(p+1);
  int pos = tmp.rfind(".");
  envName = tmp.substr(0, pos);
  string command;

  //generate random number
  int randNum = rand(); 
  std::ostringstream Out;
  Out << randNum;

  // Is there a query file?
  if((GetVizmo().IsQueryLoaded())){
    PlumObject * m_qry;
    m_qry = GetVizmo().GetQry();
    firstQry_file = (m_qry->getLoader())->GetFileName();
  }


  if((GetVizmo().IsQueryLoaded()) && (GetVizmo().hasQryChanged() == false)){
    PlumObject * m_qry;
    m_qry = GetVizmo().GetQry();
    string qryFileName = (m_qry->getLoader())->GetFileName();
    p=qryFileName.rfind('/');
    tmp.clear();
    tmp = qryFileName.substr(p+1);
    pos = tmp.rfind(".");
    string nameQ= tmp.substr(0, pos);
    sub = nameQ;
    if(firstQry_file != qryFileName)
      GetVizmo().changeQryStatus(true);
  }

  else{
    //create query file name and save it
    randName = "Qry"+Out.str()+".query";
    querySaved = GetVizmo().SaveQry(randName.c_str());
    string randSub;
    int sPos = randName.find(".", 0);
    randSub = randName.substr(0, sPos);
    //sub = dir+"/"+randSub;
    sub = randSub;
  }    

  string opt_inMap, opt_pathFile, opt_qryFile, mapName, pathName;  
  mapName = GetVizmo().getMapFileName();
  p = mapName.rfind('/');
  tmp.clear();
  tmp = mapName.substr(p+1);
  mapName = tmp;

  pathName = ""; 
  if(GetVizmo().IsPathLoaded()){
    pathName = envName+Out.str()+".path";
    opt_pathFile = " -pathFile " + pathName;
  }
  else
    opt_pathFile = " -pathFile " + envName+".path";

  opt_inMap = " -inmapFile " + mapName; 
  opt_qryFile = " -queryFile " + sub+".query";
  
  int numJoints;
  PlumObject * m_Rob;
  m_Rob = GetVizmo().GetRobot();
  OBPRMView_Robot * r = (OBPRMView_Robot*)m_Rob->getModel();
  numJoints = r->getNumJoints();
  if(numJoints ==0){
    
    command += "query_rigid  -f " + envName + opt_inMap + opt_qryFile + opt_pathFile;
  }
  else{
    std::stringstream num_s;
    num_s  <<  numJoints;
    
      command += "query_serial -numofjoints " +num_s.str()+ " -f " +
	envName + opt_inMap + opt_qryFile + opt_pathFile;
  }
  cout << "Command " << command << endl;
  int i = system(command.c_str());
  
  if(i != -1){
    if(GetVizmo().getQryFileName() != "")
      GetVizmo().setQryFileName(dir+"/"+sub+".query");
    else{
      if(querySaved !=0){
      GetVizmo().setQryFileName(dir+"/"+randName);
      }
    }
    //check if planner found path
    if(GetVizmo().IsPathLoaded()){
      if(pathName == "")
	GetVizmo().setPathFileName(dir+"/"+envName+".path");
      else
	GetVizmo().setPathFileName(dir+"/"+pathName);
    }
 
    GetVizmo().InitVizmoObject();
    
    m_showHideSGAction->setEnabled(true);
    strtGoalButton->setEnabled(true);
    if(GetVizmo().IsPathLoaded()){
      showHidePathAction->setEnabled(true);
      pathButton->setEnabled(true);
    }
  }
}*/ //End of autoQry function 


void VizmoMainWin::setSQuery(){
  qrySGUI = new queryGUI(this);
  connect(qrySGUI,SIGNAL(callUpdate()),this,SLOT(updateScreen()));
  connect(m_GL, SIGNAL(MRbyGLI()), qrySGUI, SLOT(updateQryCfg()));
  qrySGUI->m_SorG = 's';

  ///////////////////////////////////////////
  //set original Query cfgs. to queryGUI
  //so they can be reset...
  ////////////////////////////////////////
  vector<double> cfg;

  if(GetVizmo().IsQueryLoaded()){ 
    PlumObject * m_qry;
    m_qry = GetVizmo().GetQry();
    CQueryLoader * q=(CQueryLoader*)m_qry->getLoader();
    cfg =q->GetStartGoal(0);
    qrySGUI->setQuery(cfg);
  }
  else if(GetVizmo().IsPathLoaded()) {
    PlumObject * m_path;
    m_path = GetVizmo().GetPath();
    CPathLoader* ploader=(CPathLoader*)m_path->getLoader();
    cfg = ploader->GetConfiguration(0);
  }
  else{
    int dof;
    PlumObject * m_Rob;
    m_Rob = GetVizmo().GetRobot();
    OBPRMView_Robot * r = (OBPRMView_Robot*)m_Rob->getModel();    
    dof = r->returnDOF();
    cfg = vector<double>(dof,0);
  }
  
  qrySGUI->show();

 
 }

 void VizmoMainWin::setGQuery(){

  qryGGUI = new queryGUI(this);
  connect(qryGGUI,SIGNAL(callUpdate()),this,SLOT(updateScreen()));
  connect(m_GL, SIGNAL(MRbyGLI()), qryGGUI, SLOT(updateQryCfg()));
  qrySGUI->m_SorG = 'g';

  ///////////////////////////////////////////
  //set original Query cfgs. to queryGUI
  //so they can be reset...
  ////////////////////////////////////////
  
  vector<double> cfg;

  if(GetVizmo().IsQueryLoaded()){
    PlumObject * m_qry;
    m_qry = GetVizmo().GetQry();
    CQueryLoader * q=(CQueryLoader*)m_qry->getLoader();
    cfg=q->GetStartGoal(1);
    qrySGUI->setQuery(cfg);
  }

  else if(GetVizmo().IsPathLoaded()){
    PlumObject * m_path;
    m_path = GetVizmo().GetPath();
    CPathLoader* ploader=(CPathLoader*)m_path->getLoader();
    int path_size = ploader->GetPathSize();
    cfg = ploader->GetConfiguration(path_size-1);

  }
  else{
    int dof;
    PlumObject * m_Rob;
    m_Rob = GetVizmo().GetRobot();
    OBPRMView_Robot * r = (OBPRMView_Robot*)m_Rob->getModel();    
    dof = r->returnDOF();
    cfg = vector<double>(dof,0);
  }
  
  qryGGUI->show();
  
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
    fileOpenAction = new QAction(QIcon(QPixmap(folder)), "&Open", this);
    fileOpenAction->setShortcut(tr("CTRL+O"));
    connect(fileOpenAction, SIGNAL(activated()), this, SLOT(load()));
    ///////////////////////////////////////////////////////////////////////////////
    // Update Files  
    fileUpdateAction=new QAction(QIcon(QPixmap(updateIcon)), tr("Update File"), this);
    fileUpdateAction->setShortcut(tr("CTRL+R"));
    connect(fileUpdateAction,SIGNAL(activated()), this, SLOT(updatefiles()));
    fileUpdateAction->setEnabled(false);
    ///////////////////////////////////////////////////////////////////////////////
    // Save File
    fileSaveAction=new QAction(QIcon(QPixmap(diskYellow)), tr("Save Environment"),  this);
    connect(fileSaveAction,SIGNAL(activated()), this, SLOT(saveEnv()));
    fileSaveAction->setEnabled(false);

    fileSaveQryAction=new QAction(QIcon(QPixmap(diskGreen)), tr("Save Query"), this);
    connect(fileSaveQryAction,SIGNAL(activated()), this, SLOT(saveQryFile()));
    fileSaveQryAction->setEnabled(false);

    fileSaveMapAction=new QAction(QIcon(QPixmap(diskBlue)), tr("Save Roadmap"), this);
    connect(fileSaveMapAction,SIGNAL(activated()), this, SLOT(saveRoadmap()));
    fileSaveMapAction->setEnabled(false);

    ///////////////////////////////////////////////////////////////////////////////
    // Show/Hide Roadmap
    showHideRoadmapAction = new QAction(QIcon(QPixmap(navigate)), tr("Show/Hide Roadmap"), this);
    //showHideRoadmapAction->setCheckable(true); 
    showHideRoadmapAction->setIconText("Roadmap");
    connect(showHideRoadmapAction, SIGNAL(activated()), this, SLOT(showmap()));
    showHideRoadmapAction->setEnabled(false);

    ///////////////////////////////////////////////////////////////////////////////
    // Show/Hide Path
    showHidePathAction = new QAction(QIcon(QPixmap(pen)), tr("Show/Hide Path"), this);
    //showHidePathAction->setCheckable(true);
    showHidePathAction->setIconText("Path");
    connect(showHidePathAction, SIGNAL(activated()), this, SLOT(showpath()));
    showHidePathAction->setEnabled(false);

    ///////////////////////////////////////////////////////////////////////////////
    // Show/Hide Start and Goal
    m_showHideSGAction = new QAction(QIcon(QPixmap(flag)), tr("Show/Hide Start/Goal"),this);
    //m_showHideSGAction->setCheckable(true);
    m_showHideSGAction->setIconText("Start/Goal");
    connect(m_showHideSGAction, SIGNAL(activated()), this, SLOT(showstartgoal()));
    m_showHideSGAction->setEnabled(false);

    /********Buttons for unimplemented operations temporarily removed***********/ 

    ///////////////////////////////////////////////////////////////////////////////
    // Reset Camera View
    cameraResetAction = new QAction(QIcon(QPixmap(resetCameraIcon)), tr("Reset Camera"), this);
    cameraResetAction->setText("Reset Camera");   
    connect(cameraResetAction, SIGNAL(activated()) , this, SLOT(resetCamera()));
    
    // Quit
    quitAction = new QAction("Quit", this);
    quitAction->setShortcut(tr("CTRL+Q"));
    connect(quitAction, SIGNAL(activated()), qApp, SLOT(closeAllWindows()));

    ///////////////////////////////////////////////////////////////////////////////
    // Change background Color
    changeBGcolorAction = new QAction(QIcon(QPixmap(bgColor)), tr("Change BG color"), this);
    changeBGcolorAction->setText("Change BG color");
    connect(changeBGcolorAction, SIGNAL(activated()), this, SLOT(changeBGcolor()));
    changeBGcolorAction->setEnabled(false);
 
   ///////////////////////////////////////////////////////////////////////////////
    // Change Color randomly
    randObjcolorAction = new QAction(QIcon(QPixmap(pallet)), tr("Randomize Environment Colors"), this);
    randObjcolorAction->setText("Randomize Colors");    
    connect(randObjcolorAction,SIGNAL(activated()),this,SLOT(envObjsRandomColor()));
    randObjcolorAction->setEnabled(false);

    ///////////////////////////////////////////////////////////////////////////////
    // Axis 
    showAxisAction = new QAction(tr("Axis"), this); 
    showAxisAction->setCheckable(true);showAxisAction->setText("Axis");
    showAxisAction->setOn(true);
    connect(showAxisAction, SIGNAL(activated()), m_GL, SLOT(showAxis()));

    ///////////////////////////////////////////////////////////////////////////////
    // Grid
    showGridAction = new QAction(tr("Grid"), this);
    showGridAction->setCheckable(true); showGridAction->setText("Grid");
    showGridAction->setOn(false);
    connect(showGridAction, SIGNAL(activated()), m_GL, SLOT(showGrid()));
    
    ///////////////////////////////////////////////////////////////////////////////
    // Robot: set robot to initial Cfg.
    resetRobotPosAction=new QAction(QIcon(QPixmap(folder)), tr("Reset"), this);
    connect(resetRobotPosAction,SIGNAL(activated()), this, SLOT(resetRobotPosition()));
    resetRobotPosAction->setEnabled(false);

    ///////////////////////////////////////////////////////////////////////////////
    // Robot: show or hide configuration
    robotCfgAction = new QAction(tr("Show Cfg."), this); robotCfgAction->setCheckable(true);
    robotCfgAction->setText("Show Cfg.");
    robotCfgAction->setEnabled(false);
    robotCfgAction->setOn(false);
    connect(robotCfgAction, SIGNAL(activated()), this, SLOT(showCfg()));

    /////////////////////////////////////////////////////////////////////////////
    // Add an obstacle to the environment
    addObstacleAction = new QAction("Add Obstacle", this);
    connect(addObstacleAction, SIGNAL(activated()), this, SLOT(addObstacle()));
    addObstacleAction->setEnabled(false);
 
    /////////////////////////////////////////////////////////////////////////////
    // Set query
    setQrySAction = new QAction("Start", this);
    connect(setQrySAction, SIGNAL(activated()), this, SLOT(setSQuery()));
    setQrySAction->setStatusTip(tr("Set Query Start"));
    setQrySAction->setEnabled(false);
   
    setQryGAction = new QAction("Goal", this);
    connect(setQryGAction, SIGNAL(activated()), this, SLOT(setGQuery()));
    setQryGAction->setStatusTip(tr("Set Query Goal"));
    setQryGAction->setEnabled(false);

   return true;
}

void VizmoMainWin::SetTips(){
    
    (void)Q3WhatsThis::whatsThisButton(vizmoTools);
    
    const char * roadmapText = "<p>Click this button to visualize the <em>Roadmap</em>. <br>"
        "You can also select the <b>Show/Hide</b> option "
        "from the <b>Roadmap</b> menu.</p>";
    Q3WhatsThis::add(roadmapButton, roadmapText);   
    
    const char * pathText = "<p>Click this button to visualize the <em>Path</em>. <br>"
        "You can also select the <b>Show/Hide</b> option  "
        "from the <b>Path</b> menu.</p>";
    Q3WhatsThis::add(pathButton, pathText);
    
    const char * startGoalText = "<p>Click this button to visualize the <em>Start and Goal cfgs.</em><br> "
        "You can also select the <b>Show/Hide</b> option "
        "from the <b>Path</b> menu.";
    Q3WhatsThis::add(strtGoalButton, startGoalText);
    
    const char * changeBGcolorText = "<p>Click this button to change the background color";
    Q3WhatsThis::add(palletButton, changeBGcolorText);


    const char * envObjsColor = "<P>Changes randomly the colors of the environment's objects";
    Q3WhatsThis::add(envButton, envObjsColor);
    
    const char * resetCamText = "<p>Click this button to reset camera position";
    Q3WhatsThis::add(cameraButton, resetCamText);

    const char * pictureText = "Click this button to take a picture of the environment";
    Q3WhatsThis::add(screenShotGUI->takePicture, pictureText);
 
    //const char * CDText = "Click this button to enable/disable Collision Detection";
    //Q3WhatsThis::add(CDButton, CDText);

    //const char * editorText = "Click this button to open a <em>file</em>. <br>"
    //  "You can also execute <em>file</em> clicking the <b>Execute</b> button"
    //  "from the toolbar.</p>";
    //Q3WhatsThis::add(editorButton, editorText);
 
}

void VizmoMainWin::CreateToolbar(){
    
    QPixmap folderIcon, pathIcon, roadmapIcon, strtGoalIcon, palletIcon, envIcon,
            mksolidIcon, cameraIcon;
    //Removed from above list: CDIcon 
    
    vizmoTools = new Q3ToolBar(this, "Main Operations");
    vizmoTools->setLabel("Vizmo Operations");

    //////////////////////////
    /// create buttons
    //////////////////////////
    folderIcon = QPixmap(folder);
    folderButton = new QToolButton(folderIcon, "Open", "Open File", this,
    SLOT(load()), vizmoTools, "file");
    folderButton->setUsesTextLabel (true);
  
 
    vizmoTools->addSeparator();
    
    roadmapIcon = QPixmap(navigate);
    roadmapButton = new QToolButton(roadmapIcon, "Roadmap", "Load Roadmap",
        this, SLOT(showmap()), vizmoTools, "roadmap");
    roadmapButton->setToggleButton(true);
    roadmapButton->setUsesTextLabel (true);
    roadmapButton->setEnabled(false);
    
    pathIcon = QPixmap(pen);
    pathButton = new QToolButton(pathIcon, "Path", "Load Path",
        this, SLOT(showpath()), vizmoTools, "path");
    pathButton->setToggleButton(true);
    pathButton->setUsesTextLabel (true);
    pathButton->setEnabled(false);
    
    strtGoalIcon = QPixmap(flag);
    strtGoalButton = new QToolButton(strtGoalIcon, "Start/Goal", "Load Start/Goal positions",this, SLOT(showstartgoal()), vizmoTools, "start goal" );
    strtGoalButton->setToggleButton(true);
    strtGoalButton->setUsesTextLabel (true);
    strtGoalButton->setEnabled(false);
    
    vizmoTools->addSeparator(); 
   
    cameraIcon = QPixmap(resetCameraIcon);
    cameraButton = new QToolButton(cameraIcon, "Camera", " Reset Camera", this,
        SLOT(resetCamera()), vizmoTools, "Camera");
    cameraButton->setUsesTextLabel(true);
    cameraButton->setEnabled(false);
    
    palletIcon = QPixmap(bgColor);
    palletButton = new QToolButton(palletIcon, "BgColor", "Change Background Color",
        this, SLOT(changeBGcolor()), vizmoTools, "background color");
    palletButton->setUsesTextLabel(true);
    palletButton->setEnabled(false);

    //Buttons for non-implemented features temporarily removed

    envIcon = QPixmap(randEnvIcon);
    envButton = new QToolButton(envIcon, "Randomize Environment Colors", "Randomly change environment object colors", 
				this, SLOT(envObjsRandomColor()),vizmoTools, "Env.");
    envButton->setUsesTextLabel(true);
    envButton->setEnabled(false);

    //////////////////////////////////////////////////////////////////////////////
    // Collision Detection: enable/disable
    
    //CDIcon = QPixmap(crashBurn);
    //CDButton = new QToolButton(CDIcon, "CollisionDetection", "Enable-Disable CD", 
    //			       this, SLOT(enablDisablCD()), vizmoTools, "CD");
    //CDButton->setUsesTextLabel (true);
    //CDButton->setToggleButton(true);
    //CDButton->setEnabled(false);

    vizmoTools->addSeparator(); 

    //////////////////////////////////////////////////////////////////////////////
    // Editor

    //editorButton = new QToolButton(QPixmap(edit),
    //				   "Editor", "VizmoEditor", this,
    //				   SLOT(createEditor()), vizmoTools, "VizEdt");
    
    //editorButton->setUsesTextLabel (true);
    //editorButton->setEnabled(false);

    /////////////////////////////////////////////////////////////////////////////
    // Automatic generation of command line for obprm and query

    //mkmpButton = new QToolButton(QPixmap(font_edit),
    //				   "Automatic obprm", "CommandLine1", this,
    //				   SLOT(autoMkmp()), vizmoTools, "AutoMkmp");
    //mkmpButton->setUsesTextLabel (true);
    //mkmpButton->setEnabled(false);

    //qryButton = new QToolButton(QPixmap(runner),
    //				   "Run Query", "CommandLine2", this,
    //				   SLOT(autoQry()), vizmoTools, "AutoQry");
    //qryButton->setUsesTextLabel (true);
    //qryButton->setEnabled(false);

}

void VizmoMainWin::CreateMenubar()
{ 
    //create FILE menu
    QMenu* file = new QMenu(this); 
    menuBar()->insertItem("&File", file);
    file->addAction(fileOpenAction); 
    file->addAction(fileUpdateAction); 
    file->addAction(fileSaveAction); 
    file->addAction(fileSaveQryAction); 
    file->addAction(fileSaveMapAction); 
    file->addAction(quitAction); 

    //////////////////////////////////////////////
    //Scene Menu
    //////////////////////////////////////////////
    QMenu* sceneMenu = new QMenu(this); 
    menuBar()->insertItem("&Scene", sceneMenu);
    
    sceneMenu->addAction(showGridAction); 
    sceneMenu->addAction(showAxisAction); 
    sceneMenu->addAction(cameraResetAction);
     
    //Camera position does not have its own button 
    m_cameraPositionAction = new QAction("Set Camera Position", sceneMenu);
    m_cameraPosInput = new CameraPosDialog(this); 
    connect(m_cameraPositionAction, SIGNAL(activated()), this, SLOT(SetCameraPos())); 

    sceneMenu->addAction(m_cameraPositionAction); 
    sceneMenu->addAction(changeBGcolorAction); 

    ///////////////////////////////////////////////
    // ROBOT menu
    // first create submenus: robotRigid, robotArt
    ///////////////////////////////////////////////
    QMenu* robotMenu = new QMenu(this); 
    menuBar()->insertItem("&Robot", robotMenu);
    robotMenu->addAction(resetRobotPosAction); 
    robotMenu->addAction(robotCfgAction); 

    /////////////////////////////////////////////  
    // ENVIRONMENT menu. Probably want a QMenu* like others eventually, but this
    // one is odd. Actually declared in MainWin.h and used earlier in this file. 
    /////////////////////////////////////////////
    envMenu = new Q3PopupMenu(this);
    menuBar()->insertItem("&Environment", envMenu);
    refreshID = envMenu->insertItem("Refresh", this, SLOT(refreshEnv())); 
    envMenu->setItemEnabled(refreshID, FALSE);
    envMenu->insertSeparator();
    //changeBGcolorAction->addTo(envMenu);
    randObjcolorAction->addTo(envMenu);
    envMenu->insertSeparator();
    bboxID = envMenu->insertItem("&Bounding Box", this, SLOT(showBBox()),  Qt::CTRL+Qt::Key_B);   
    envMenu->setItemEnabled(bboxID , FALSE);
    addObstacleAction->addTo(envMenu);  
    
    ////////////////////////////////////////////// 
    // ROADMAP menu
    // first create submenus: param
    //////////////////////////////////////////////
    Q3PopupMenu * param = new Q3PopupMenu(this);
    param->insertTearOffHandle();
    //param->insertItem("Change size of nodes", this, SLOT(changeSize()));
    int numberId = param->insertItem("Number of nodes", this, SLOT(notimp()));
    int defId =param->insertItem("Define start/goal", this, SLOT(notimp()));
    param->setItemEnabled(numberId, FALSE);
    param->setItemEnabled(defId, FALSE);
    
    QMenu* roadmapMenu = new QMenu(this);
    menuBar()->insertItem("Road&map", roadmapMenu);
    roadmapMenu->addAction(showHideRoadmapAction);
    roadmapMenu->insertSeparator();
  
    QMenu* nodeView = new QMenu("Change Node Shape", roadmapMenu);
    QAction* robotView = new QAction("Robot", nodeView);
    QAction* boxView = new QAction("Box", nodeView); 
    QAction* pointView = new QAction("Point", nodeView); 
    nodeView->addAction(robotView);
    nodeView->addAction(boxView); 
    nodeView->addAction(pointView); 
    connect(nodeView, SIGNAL(triggered(QAction*)), roadmapGUI, SLOT(changeNodeShape(QAction*)));  
    roadmapMenu->addMenu(nodeView);

    QMenu* modifySelected = new QMenu("Modify Selected Item", roadmapMenu); 
    modifySelected->addAction(roadmapGUI->solidSelectNodeAction);
    modifySelected->addAction(roadmapGUI->wireSelectNodeAction); 
    modifySelected->addAction(roadmapGUI->invisibleSelectNodeAction);
    modifySelected->addAction(roadmapGUI->colorSelectNodeAction); 
    roadmapMenu->addMenu(modifySelected);  

    roadmapMenu->addAction(roadmapGUI->sizeAction);
    roadmapMenu->addAction(roadmapGUI->m_edgeSizeAction); 

    QMenu* connectedComps = new QMenu("Modify CCs", roadmapMenu); 
    connectedComps->addAction(roadmapGUI->colorSelectAction); 
    connectedComps->addAction(roadmapGUI->colorAction); 
    connectedComps->addAction(roadmapGUI->m_ccsOneColor); 
    roadmapMenu->addMenu(connectedComps); 

    //roadmapMenu->addAction(roadmapGUI->editAction);
    //roadmapMenu->addAction(roadmapGUI->addNodeAction);
    //roadmapMenu->addAction(roadmapGUI->addEdgeAction);

    /////////////////////////////////////////////////
    // PATH menu
    // first create submenus: opt
    ////////////////////////////////////////////////
    QMenu* pathMenu = new QMenu(this);
    pathMenu = menuBar()->addMenu(tr("&Path"));
    pathMenu->addAction(showHidePathAction);
    pathMenu->addAction(m_showHideSGAction);

    //Path display options does not have its own button in toolbar
    m_pathOptionsAction = new QAction("Path Display Options", pathMenu); 
    m_pathOptionsInput = new CustomizePathDialog(this); 
    connect(m_pathOptionsAction, SIGNAL(activated()), this, SLOT(PathDisplayOptions()));
    pathMenu->addAction(m_pathOptionsAction);
    m_pathOptionsAction->setEnabled(false); 

    pathMenu->addSeparator();

    QMenu* opt =  pathMenu->addMenu(tr("Set Query"));
    opt->insertTearOffHandle();
    opt->addAction(setQrySAction);
    opt->addAction(setQryGAction);
       
    /////////////////////////////////////////////  
    // OBPRM menu
    /////////////////////////////////////////////
    //Q3PopupMenu * obprmMenu = new Q3PopupMenu(this);
    //menuBar()->insertItem("MP Library", obprmMenu);
    //obprmMenu->insertSeparator();
    //obprmMenu->insertItem("Generate mkmp", this, SLOT(runCode()));
    //obprmMenu->insertItem("Generate qry", this, SLOT(createQryFile()));

    /////////////////////////////////////////////
    //SCREEN CAPTURE menu
    /////////////////////////////////////////////
    QMenu* captureMenu = new QMenu(this); 
    captureMenu = menuBar()->addMenu(tr("&Capture"));
    captureMenu->addAction("Crop", screenShotGUI, SLOT(takeBoxSnapshot())); 
    captureMenu->addAction("Picture", screenShotGUI, SLOT(takeSnapshot())); 
    captureMenu->addAction("Movie", screenShotGUI, SLOT(takeMoviePictures())); 

    /////////////////////////////////////////////  
    //create HELP menu
    /////////////////////////////////////////////
    Q3PopupMenu* help = new Q3PopupMenu(this);
    menuBar()->insertItem("&Help", help);
    help->insertItem("&About", this, SLOT(about()), Qt::Key_F1);
    help->insertItem("What's &This", this, SLOT(whatsThis()), Qt::SHIFT+Qt::Key_F1);  
}



/*void VizmoMainWin::CreateScreenCapture()
{
    screenShotGUI=new VizmoScreenShotGUI(this,(char*)"Screen Shot");
    connect(screenShotGUI,SIGNAL(getScreenSize(int *,int *)),this,SLOT(getOpenglSize(int *,int *)));
    connect(screenShotGUI,SIGNAL(togleSelectionSignal()),m_GL,SLOT(togleSlectionSlot()));
    connect(screenShotGUI,SIGNAL(getBoxDimensions(int *,int *,int *,int *)),m_GL,SLOT(getBoxDimensions(int *,int *,int *,int *)));
    connect(screenShotGUI,SIGNAL(callUpdate()),this,SLOT(updateScreen()));
    connect(screenShotGUI,SIGNAL(goToFrame(int)),animationGUI,SLOT(goToFrame(int)));
    connect(screenShotGUI,SIGNAL(goToFrame(int)),animationDebugGUI,SLOT(goToFrame(int)));
}*/ 

void VizmoMainWin::CreateObjectSelection()
{
    objectSelection= new VizmoItemSelectionGUI(this,(char*)"ObjectSelection");
    connect(objectSelection,SIGNAL(callUpdate()),this,SLOT(updateScreen()));
	//connect(objectSelection,SIGNAL(itemSelected()), this, SLOT(obj_contexmenu()));
}

void VizmoMainWin::CreateAttributeSelection()
{
    //attributeSelection= new VizmoAttributeSelectionGUI(this,"AttributeSelection");
}

void VizmoMainWin::CreateRoadmapToolbar(){
  roadmapGUI = new VizmoRoadmapGUI (this, (char*)"MapSelection");
  connect(roadmapGUI,SIGNAL(callUpdate()),this,SLOT(updateScreen()));
}

void VizmoMainWin::CreateScreenCapture()
{
    screenShotGUI=new VizmoScreenShotGUI(this,(char*)"Screen Shot");
    connect(screenShotGUI,SIGNAL(getScreenSize(int *,int *)),this,SLOT(getOpenglSize(int *,int *)));
    connect(screenShotGUI,SIGNAL(togleSelectionSignal()),m_GL,SLOT(togleSlectionSlot()));
    connect(screenShotGUI,SIGNAL(getBoxDimensions(int *,int *,int *,int *)),m_GL,SLOT(getBoxDimensions(int *,int *,int *,int *)));
    connect(screenShotGUI,SIGNAL(callUpdate()),this,SLOT(updateScreen()));
    connect(screenShotGUI,SIGNAL(goToFrame(int)),animationGUI,SLOT(goToFrame(int)));
    connect(screenShotGUI,SIGNAL(goToFrame(int)),animationDebugGUI,SLOT(goToFrame(int)));
}

void 
VizmoMainWin::CreateTextOutbox(){
  m_outbox = new TextGUI (this, (char*)"Vizmo Text Output");  
  m_outbox->setGeometry(0, 375, 206, 220);
}


