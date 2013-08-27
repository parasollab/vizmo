#include "FileOptions.h"

#include <QApplication>
#include <QAction>
#include <QToolBar>
#include <QPushButton>
#include <QPixmap>
#include <QFileDialog>
#include <QTranslator>
#include <QStatusBar>

#include "MainMenu.h"
#include "FileListDialog.h"
#include "GLWidget.h"
#include "AnimationGUI.h"
#include "ItemSelectionGUI.h"
#include "OptionsBase.h"
#include "MainWindow.h"
#include "Models/Vizmo.h"
#include "Models/MapModel.h"

#include "Icons/Folder.xpm"
#include "Icons/Update.xpm"

FileOptions::FileOptions(QWidget* _parent, MainWindow* _mainWindow) : OptionsBase(_parent, _mainWindow) {
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
  m_actions["quit"]->setShortcut(tr("CTRL+Q"));
  m_actions["updateFile"]->setEnabled(false);
  m_actions["saveFile"]->setEnabled(false);
  m_actions["saveQuery"]->setEnabled(false);
  m_actions["saveRoadmap"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["openFile"], SIGNAL(triggered()), this, SLOT(LoadFile()));
  connect(m_actions["updateFile"], SIGNAL(triggered()), this, SLOT(UpdateFiles()));
  connect(m_actions["saveFile"], SIGNAL(triggered()), this, SLOT(SaveEnv()));
  connect(m_actions["saveQuery"], SIGNAL(triggered()), this, SLOT(SaveQryFile()));
  connect(m_actions["saveRoadmap"], SIGNAL(triggered()), this, SLOT(SaveRoadmap()));
  connect(m_actions["quit"], SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
}

void
FileOptions::SetUpToolbar(){
  m_toolbar = new QToolBar(m_mainWindow);
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
      QString::null, "Files (*.env *.map *.query *.path *.vd)");

  QFileInfo fi(fn);

  if(!fn.isEmpty()){
    m_mainWindow->GetArgs().clear();
    m_mainWindow->GetArgs().push_back(QString(fn.toLatin1()).toStdString()); //access the actual main window
    m_mainWindow->SetVizmoInit(false);
    m_mainWindow->setWindowTitle("Vizmo++ - "+fi.baseName()+ " environment");
    m_mainWindow->statusBar()->showMessage("File Loaded : "+fn);
  }
  else
    m_mainWindow->statusBar()->showMessage("Loading aborted");

  m_mainWindow->GetGLScene()->resetTransTool();
  m_mainWindow->GetGLScene()->updateGL();
}

void
FileOptions::UpdateFiles(){
  FileListDialog flDialog("", this);

  if(flDialog.exec() != QDialog::Accepted)
    return;

  if(!GetVizmo().InitModels())
    return;

  //reset guis
  m_mainWindow->GetAnimationGUI()->reset();
  m_mainWindow->GetObjectSelection()->ResetLists();
  m_mainWindow->m_mainMenu->CallReset();
  m_mainWindow->GetGLScene()->resetTransTool();
}

void
FileOptions::SaveEnv(){

  QString fn = QFileDialog::getSaveFileName(this, "Choose a file name for the environment",
      QString::null, "Files(*.env)");

  QFileInfo fi(fn);
  if(!fn.isEmpty()){
    string filename = fn.toStdString();
    const char* f;
    f = filename.c_str();
    GetVizmo().SaveEnv(f);
  }
  else{
    m_mainWindow->statusBar()->showMessage("Saving aborted", 2000);
  }
  m_mainWindow->GetGLScene()->updateGL();
}

void
FileOptions::SaveQryFile(){

  QString fn = QFileDialog::getSaveFileName(this, "Choose a file name to save the query",
      QString::null, "Files (*.query)");

  QFileInfo fi(fn);
  if(!fn.isEmpty()){
    string filename = fn.toStdString();
    const char* f;
    f = filename.c_str();
    GetVizmo().SaveQry(f);
  }
  else{
    m_mainWindow->statusBar()->showMessage("Saving aborted", 2000);
  }
  m_mainWindow->GetGLScene()->updateGL();
}

void
FileOptions::SaveRoadmap(){

  QString fn = QFileDialog::getSaveFileName(this, "Choose a file name for the roadmap",
    QString::null, "Files (*.map)");

  QFileInfo fi(fn);
  if(!fn.isEmpty()){
    string filename = fn.toStdString();
    const char* f;
    f = filename.c_str();
    GetVizmo().GetMap()->WriteMapFile(f);
  }
  else
    m_mainWindow->statusBar()->showMessage("Saving aborted", 2000);

  m_mainWindow->GetGLScene()->updateGL();
}
