#include <QApplication> 
#include <QAction>
#include <QToolBar>
#include <QPushButton> 
#include <QPixmap>
#include <QFileDialog>
#include <QTranslator>
#include <QStatusBar>  

#include "FileOptions.h"
#include "MainMenu.h" 
#include "FileListDialog.h"
#include "SceneWin.h"
#include "AnimationGUI.h"
#include "ItemSelectionGUI.h"
#include "OptionsBase.h" 
#include "MainWin.h"   
#include "vizmo2.h"  
#include "icon/Folder.xpm"
#include "icon/Update.xpm"

FileOptions::FileOptions(QWidget* _parent, VizmoMainWin* _mainWin)
  :OptionsBase(_parent, _mainWin)
{
  CreateActions();
  SetUpSubmenu("File"); 
  SetUpToolbar(); 
  SetHelpTips(); 
}

void
FileOptions::CreateActions(){

  //1. Create actions and add them to map
  QAction* openFile = new QAction(QPixmap(folder), tr("&Open"), this); 
  m_actions["openFile"] = openFile; 
  QAction* updateFile = new QAction(QPixmap(updateIcon), tr("Update File"), this); 
  m_actions["updateFile"] = updateFile; 
  QAction* saveFile = new QAction(tr("Save Environment"), this); 
  m_actions["saveFile"] = saveFile; 
  QAction* saveQuery = new QAction(tr("Save Query"), this); 
  m_actions["saveQuery"] = saveQuery; 
  QAction* saveRoadmap = new QAction(tr("Save Roadmap"), this);
  m_actions["saveRoadmap"] = saveRoadmap; 
  QAction* quit = new QAction(tr("Quit"), this); 
  m_actions["quit"] = quit;  

  //2. Set other specifications as necessary  
  m_actions["openFile"]->setShortcut(tr("CTRL+O")); //this one necessary?
  m_actions["openFile"]->setStatusTip(tr("Open a file"));
  m_actions["updateFile"]->setShortcut(tr("CTRL+R"));
  m_actions["quit"]->setShortcut(tr("CTRL-Q")); 
  m_actions["updateFile"]->setEnabled(false); 
  m_actions["saveFile"]->setEnabled(false); 
  m_actions["saveQuery"]->setEnabled(false); 	
  m_actions["saveRoadmap"]->setEnabled(false);

  //3. Make connections 
  connect(m_actions["openFile"], SIGNAL(activated()), this, SLOT(LoadFile()));  
  connect(m_actions["updateFile"], SIGNAL(activated()), this, SLOT(UpdateFiles())); 
  connect(m_actions["saveFile"], SIGNAL(activated()), this, SLOT(SaveEnv())); 
  connect(m_actions["saveQuery"], SIGNAL(activated()), this, SLOT(SaveQryFile())); 
  connect(m_actions["saveRoadmap"], SIGNAL(activated()), this, SLOT(SaveRoadmap())); 
  connect(m_actions["quit"], SIGNAL(activated()), qApp, SLOT(closeAllWindows())); 
}

void
FileOptions::SetUpToolbar(){ 

  m_toolbar = new QToolBar(GetMainWin());  
  m_toolbar->addAction(m_actions["openFile"]);  
}

void
FileOptions::Reset(){

  m_actions["updateFile"]->setEnabled(true);
  m_actions["saveFile"]->setEnabled(true); 	
  m_actions["saveQuery"]->setEnabled(true); 	
  m_actions["saveRoadmap"]->setEnabled(true); 	
}

void
FileOptions::SetHelpTips(){

  m_actions["openFile"]->setWhatsThis("Click this button to open a <b>File</b>.<br>" 
    "You can separately specify the preferred Map, Environment, Path, Debug, and Query files."); 
}  

//Slots 

void 
FileOptions::LoadFile(){

  QString fn = QFileDialog::getOpenFileName(this,  "Choose an environment to open",  
      QString::null,"Files (*.map *.env *.vd)");

  QFileInfo fi(fn);

  if (!fn.isEmpty()){
    GetMainWin()->GetArgs().push_back(QString(fn.toLatin1()).toStdString()); //access the actual main window 
    GetMainWin()->SetVizmoInit(false);
    GetMainWin()->setWindowTitle("Vizmo++ - "+fi.baseName()+ " environment");
    GetMainWin()->statusBar()->showMessage("File Loaded : "+fn);
  }
  else 
    GetMainWin()->statusBar()->showMessage("Loading aborted");

  GetMainWin()->GetGLScene()->resetTransTool();
  GetMainWin()->GetGLScene()->updateGL();
} 

void
FileOptions::UpdateFiles(){

  GetMainWin()->SetVizmoInit(false); 	
  m_flDialog = new FileListDialog(this); 

  if(m_flDialog->exec() != QDialog::Accepted)
    return; 	

  if(GetVizmo().InitVizmoObject() == false)
    return; 

  //reset guis
  GetMainWin()->GetAnimationGUI()->reset(); 
  GetMainWin()->GetAnimationDebugGUI()->reset(); 
  GetMainWin()->GetObjectSelection()->ResetLists(); 
  GetMainWin()->m_mainMenu->CallReset();  

  GetMainWin()->GetGLScene()->resetTransTool();
}

void
FileOptions::SaveEnv(){

  QString fn = QFileDialog::getSaveFileName(this, "Choose a file name for the environment", 
      QString::null, "Files(*.env)");  

  QFileInfo fi(fn);
  if (!fn.isEmpty()){
    string filename = fn.toStdString();
    const char* f;
    f = filename.c_str();
    GetVizmo().SaveEnv(f);
  } 

  else{ 
    GetMainWin()->statusBar()->showMessage("Saving aborted", 2000);
  }
  GetMainWin()->GetGLScene()->updateGL();
} 

void
FileOptions::SaveQryFile(){

  QString fn = QFileDialog::getSaveFileName(this, "Choose a file name to save the query",
      QString::null, "Files (*.query)");

  QFileInfo fi(fn);
  if (!fn.isEmpty()) {
    string filename = fn.toStdString();
    const char* f;
    f = filename.c_str();
    GetVizmo().SaveQry(f);
  } 

  else {
    GetMainWin()->statusBar()->showMessage("Saving aborted", 2000);
  }

  GetMainWin()->GetGLScene()->updateGL();
}

void
FileOptions::SaveRoadmap(){

  QString fn = QFileDialog::getSaveFileName(this, "Choose a file name for the roadmap",
      QString::null, "Files (*.map)");

  QFileInfo fi(fn);
  if (!fn.isEmpty()){
    string filename = fn.toStdString() ;
    const char* f;
    f = filename.c_str();
    //GetVizmo().SaveMap(f);
    SaveNewRoadmap(f); //moved from above. Should combine this..? 
  } 

  else{
    GetMainWin()->statusBar()->showMessage("Saving aborted", 2000);
  }
  GetMainWin()->GetGLScene()->updateGL();
}

void
FileOptions::SaveNewRoadmap(const char* _filename){

  PlumObject* m_Map;
  m_Map = GetVizmo().GetMap();

  CMapHeaderLoader* maploader = (CMapHeaderLoader*)m_Map->GetLoader();

  ofstream outfile (_filename);

  if(maploader->ParseHeader()!=false){ //return false;
    const string version = maploader->GetVersionNumber();
    const string preamble = maploader->GetPreamble();
    const string envFile = maploader->GetEnvFileName();
    const list<string> lps = maploader->GetLPs();
    const list<string> cds = maploader->GetCDs();
    const list<string> dms = maploader-> GetDMs();
    const string seed = maploader-> GetSeed();
    outfile<< "Roadmap Version Number "<< version<<"\n";
    outfile<< "#####PREAMBLESTART##### \n";
    outfile<<preamble<<"\n";
    outfile<< "#####PREAMBLESTOP##### \n";
    outfile<< "#####ENVFILESTART##### \n";
    outfile<<envFile<<"\n";
    outfile<< "#####ENVFILESTOP##### \n";
    outfile<< "#####LPSTART##### \n";
    outfile<<lps.size()<<endl;
    list<string>::const_iterator it;
    for(it=lps.begin(); it!=lps.end(); ++it){
      outfile << *it << endl; // each element on a separate line
    }
    outfile<< "#####LPSTOP##### \n";
    outfile<< "#####CDSTART##### \n";
    outfile<<cds.size()<<endl;
    for(it=cds.begin(); it!=cds.end(); ++it){
      outfile << *it << endl; // each element on a separate line
    }
    outfile<< "#####CDSTOP##### \n";
    outfile<< "#####DMSTART##### \n";
    outfile<<dms.size()<<endl;
    for(it=dms.begin(); it!=dms.end(); ++it){
      outfile << *it << endl; // each element on a separate line
    }
    outfile<< "#####DMSTOP#####";

    if(version == "041805"){
      outfile<< "\n#####RNGSEEDSTART##### \n";
      outfile<< seed <<"\n";
      outfile<< "#####RNGSEEDSTOP##### \n";
    }
  }

  typedef CMapLoader<CCfg,Edge>::Wg WG;
  WG* graph;
  CMapLoader<CCfg,Edge> *m_loader=(CMapLoader<CCfg,Edge>*)m_Map->GetLoader();
  graph = m_loader->GetGraph();

  write_graph(*graph, outfile);
  outfile.close();
  //return true;
}    












