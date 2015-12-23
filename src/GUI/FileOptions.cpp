#include "FileOptions.h"

#include "AnimationWidget.h"
#include "FileListDialog.h"
#include "GLWidget.h"
#include "MainMenu.h"
#include "MainWindow.h"
#include "ModelSelectionWidget.h"

#include "Models/EnvModel.h"
#include "Models/MapModel.h"
#include "Models/PathModel.h"
#include "Models/QueryModel.h"
#include "Models/Vizmo.h"

#include "Icons/Folder.xpm"
#include "Icons/Quit.xpm"
#include "Icons/SaveEnv.xpm"
#include "Icons/SaveMap.xpm"
#include "Icons/SavePath.xpm"
#include "Icons/SaveQuery.xpm"
#include "Icons/Update.xpm"


FileOptions::
FileOptions(QWidget* _parent) : OptionsBase(_parent, "File") {
  CreateActions();
  SetUpSubmenu();
  SetUpToolbar();
  SetHelpTips();
}

/*---------------------------- GUI Management --------------------------------*/

void
FileOptions::
CreateActions() {
  //1. Create actions and add them to map
  m_actions["openFile"] = new QAction(QPixmap(folder),
      tr("&Open"), this);
  m_actions["updateFile"] = new QAction(QPixmap(updateicon),
      tr("Update File"), this);
  m_actions["saveFile"] = new QAction(QPixmap(saveenv),
      tr("Save Environment"), this);
  m_actions["saveQuery"] = new QAction(QPixmap(savequery),
      tr("Save Query"), this);
  m_actions["saveRoadmap"] = new QAction(QPixmap(savemap),
      tr("Save Roadmap"), this);
  m_actions["savePath"] = new QAction(QPixmap(savepath),
      tr("Save Path"), this);
  m_actions["quit"] = new QAction(QPixmap(quiticon),
      tr("Quit"), this);

  //2. Set other specifications as necessary
  m_actions["openFile"]->setShortcut(tr("CTRL+O")); //this one necessary?
  m_actions["openFile"]->setStatusTip(tr("Open a file"));
  m_actions["updateFile"]->setShortcut(tr("CTRL+U"));
  m_actions["quit"]->setShortcut(tr("CTRL+Q"));

  m_actions["updateFile"]->setEnabled(false);
  m_actions["saveFile"]->setEnabled(false);
  m_actions["saveQuery"]->setEnabled(false);
  m_actions["saveRoadmap"]->setEnabled(false);
  m_actions["savePath"]->setEnabled(false);

  //3. Make connections
  connect(m_actions["openFile"], SIGNAL(triggered()),
      this, SLOT(LoadFile()));
  connect(m_actions["updateFile"], SIGNAL(triggered()),
      this, SLOT(UpdateFiles()));
  connect(m_actions["saveFile"], SIGNAL(triggered()),
      this, SLOT(SaveEnv()));
  connect(m_actions["saveQuery"], SIGNAL(triggered()),
      this, SLOT(SaveQryFile()));
  connect(m_actions["saveRoadmap"], SIGNAL(triggered()),
      this, SLOT(SaveRoadmap()));
  connect(m_actions["savePath"], SIGNAL(triggered()),
      this, SLOT(SavePath()));
  connect(m_actions["quit"], SIGNAL(triggered()),
      qApp, SLOT(closeAllWindows()));
}


void
FileOptions::
SetHelpTips() {
  m_actions["openFile"]->setWhatsThis("Click this button to open a <b>File</b>."
      "<br>You can separately specify the preferred Map, Environment, Path, "
      "Debug, and Query files.");
}


void
FileOptions::
SetUpToolbar() {
  m_toolbar = new QToolBar(GetMainWindow());
  m_toolbar->addAction(m_actions["openFile"]);
}


void
FileOptions::
Reset() {
  m_actions["updateFile"]->setEnabled(true);
  m_actions["saveFile"]->setEnabled(true);
  m_actions["saveQuery"]->setEnabled(true);
  m_actions["saveRoadmap"]->setEnabled(true);
  m_actions["savePath"]->setEnabled(true);
}

/*---------------------------- File Functions --------------------------------*/

void
FileOptions::
LoadFile() {
  /// Handles all local data types, including:
  /// \arg \c .env
  /// \arg \c .map
  /// \arg \c .query
  /// \arg \c .path
  /// \arg \c .vd
  /// \arg \c .xml
  QString fn = QFileDialog::getOpenFileName(this,
      "Choose an environment to open", GetMainWindow()->GetLastDir(),
      "Files (*.env *.map *.query *.path *.vd *.xml)");

  if(!fn.isEmpty()){
    GetMainWindow()->ResetDialogs();
    GetMainWindow()->GetArgs().clear();
    GetMainWindow()->GetArgs().push_back(QString(fn.toLatin1()).toStdString());
    GetMainWindow()->SetVizmoInit(false);
    QFileInfo fi(fn);
    GetMainWindow()->setWindowTitle("Vizmo++ - " + fi.baseName() +
        " environment");
    GetMainWindow()->statusBar()->showMessage("File Loaded : "+fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
    GetMainWindow()->InitVizmo();
  }
  else
    GetMainWindow()->statusBar()->showMessage("Loading aborted");
}


void
FileOptions::
UpdateFiles() {
  vector<string> emptyFiles;
  FileListDialog flDialog(emptyFiles, this);
  if(flDialog.exec() != QDialog::Accepted)
    return;
  GetMainWindow()->ResetDialogs();

  if(!GetVizmo().InitModels())
    return;

  //reset guis
  GetMainWindow()->GetAnimationWidget()->Reset();
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
  GetMainWindow()->m_mainMenu->CallReset();
  GetMainWindow()->GetGLWidget()->ResetCamera();
  GetMainWindow()->GetGLWidget()->SetCurrentRegion();
}


void
FileOptions::
SaveEnv() {
  QString fn = QFileDialog::getSaveFileName(this,
      "Choose a file name for the environment", GetMainWindow()->GetLastDir(),
      "Files(*.env)");

  if(!fn.isEmpty()){
    string filename = fn.toStdString();
    GetVizmo().GetEnv()->SaveFile(filename);
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
  else
    GetMainWindow()->statusBar()->showMessage("Saving aborted", 2000);
}


void
FileOptions::
SaveQryFile() {
  QString fn = QFileDialog::getSaveFileName(this,
      "Choose a file name to save the query", GetMainWindow()->GetLastDir(),
      "Files (*.query)");

  if(!fn.isEmpty()) {
    GetVizmo().GetQry()->SaveQuery(fn.toStdString());
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
  else
    GetMainWindow()->statusBar()->showMessage("Saving aborted", 2000);
}


void
FileOptions::
SaveRoadmap() {
  QString fn = QFileDialog::getSaveFileName(this,
      "Choose a file name for the roadmap",
      GetMainWindow()->GetLastDir(), "Files (*.map)");

  if(!fn.isEmpty()){
    string filename = fn.toStdString();
    const char* f;
    f = filename.c_str();
    GetVizmo().GetMap()->Write(f);
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
  else
    GetMainWindow()->statusBar()->showMessage("Saving aborted", 2000);
}


void
FileOptions::
SavePath() {
  QString fn = QFileDialog::getSaveFileName(this,
      "Choose a file name for the path",
      GetMainWindow()->GetLastDir(), "Files(*.path)");

  if(!fn.isEmpty() && GetVizmo().GetPath()) {
    GetVizmo().GetPath()->SavePath(fn.toStdString());
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
  else
    GetMainWindow()->statusBar()->showMessage("Saving aborted", 2000);
}
