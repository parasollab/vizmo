#include "FileListDialog.h"

#include <QFileDialog>
#include <QGridLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

#include "MainWindow.h"

#include "Models/Vizmo.h"
#include "Models/CfgModel.h"
#include "Models/EdgeModel.h"
#include "Models/MapModel.h"

#include "Icons/Folder.xpm"
#include "Icons/Vizmo.xpm"

/*------------------------------- Construction -------------------------------*/

FileListDialog::
FileListDialog(const vector<string>& _filename, QWidget* _parent, Qt::WindowFlags _f) :
    QDialog(_parent, _f) {
  setWindowIcon(QPixmap(vizmoIcon));

  QGridLayout* layout = new QGridLayout;
  setLayout(layout);

  QPushButton* loadButton = new QPushButton("Load", this);
  QPushButton* cancelButton = new QPushButton("Cancel", this);

  m_envCheckBox = new QCheckBox(this);
  m_envFilename = new QLabel(this);
  QLabel* envLabel = new QLabel("<b>Env File</b>:", this);
  QPushButton* envButton = new QPushButton(QIcon(QString((const char*)(folder))), "Browse", this);

  m_mapCheckBox = new QCheckBox(this);
  m_mapFilename = new QLabel(this);
  QLabel* mapLabel = new QLabel("<b>Map File</b>:", this);
  QPushButton* mapButton = new QPushButton(QIcon(QString((const char*)(folder))),"Browse", this);

  m_queryCheckBox = new QCheckBox(this);
  m_queryFilename = new QLabel(this);
  QLabel* queryLabel = new QLabel("<b>Query File</b>:", this);
  QPushButton* queryButton = new QPushButton(QIcon(QString((const char*)(folder))), "Browse", this);

  m_pathCheckBox = new QCheckBox(this);
  m_pathFilename = new QLabel(this);
  QLabel* pathLabel = new QLabel("<b>Path File</b>:", this);
  QPushButton* pathButton = new QPushButton(QIcon(QString((const char*)(folder))), "Browse", this);

  m_rvCheckBox = new QCheckBox(this);
  m_rvFilename = new QLabel(this);
  QLabel* rvLabel = new QLabel("<b>RV File</b>:", this);
  QPushButton* rvButton = new QPushButton(QIcon(QString((const char*)(folder))), "Browse", this);

  m_debugCheckBox = new QCheckBox(this);
  m_debugFilename = new QLabel(this);
  QLabel* debugLabel = new QLabel("<b>Debug File</b>:", this);
  QPushButton* debugButton = new QPushButton(QIcon(QString((const char*)(folder))), "Browse", this);

  m_xmlFilename = new QLabel(this);
  QLabel* xmlLabel = new QLabel("<b>XML File</b>:", this);
  QPushButton* xmlButton = new QPushButton(QIcon(QString((const char*)(folder))), "Browse", this);

  layout->addWidget(m_envCheckBox, 0, 0);
  layout->addWidget(m_envFilename, 0, 2, 1, 3);
  layout->addWidget(envLabel, 0, 1);
  layout->addWidget(envButton, 0, 6);

  layout->addWidget(m_mapCheckBox, 1, 0);
  layout->addWidget(m_mapFilename, 1, 2, 1, 3);
  layout->addWidget(mapLabel, 1, 1);
  layout->addWidget(mapButton, 1, 6);

  layout->addWidget(m_queryCheckBox, 2, 0);
  layout->addWidget(m_queryFilename, 2, 2, 1, 3);
  layout->addWidget(queryLabel, 2, 1);
  layout->addWidget(queryButton, 2, 6);

  layout->addWidget(m_pathCheckBox, 3, 0);
  layout->addWidget(m_pathFilename, 3, 2, 1, 3);
  layout->addWidget(pathLabel, 3, 1);
  layout->addWidget(pathButton, 3, 6);

  layout->addWidget(m_rvCheckBox, 5, 0);
  layout->addWidget(m_rvFilename, 5, 2, 1, 3);
  layout->addWidget(rvLabel, 5, 1);
  layout->addWidget(rvButton, 5, 6);

  layout->addWidget(m_debugCheckBox, 4, 0);
  layout->addWidget(m_debugFilename, 4, 2, 1, 3);
  layout->addWidget(debugLabel, 4, 1);
  layout->addWidget(debugButton, 4, 6);

  layout->addWidget(m_xmlFilename, 6, 2, 1, 3);
  layout->addWidget(xmlLabel, 6, 1);
  layout->addWidget(xmlButton, 6, 6);

  loadButton->setFixedWidth(cancelButton->width());
  layout->addWidget(loadButton, 7, 5);
  layout->addWidget(cancelButton, 7, 6);

  connect(mapButton, SIGNAL(clicked()), this, SLOT(ChangeMap()));
  connect(envButton, SIGNAL(clicked()), this, SLOT(ChangeEnv()));
  connect(pathButton, SIGNAL(clicked()), this, SLOT(ChangePath()));
  connect(rvButton, SIGNAL(clicked()), this, SLOT(ChangeRV()));
  connect(debugButton, SIGNAL(clicked()), this, SLOT(ChangeDebug()));
  connect(queryButton, SIGNAL(clicked()), this, SLOT(ChangeQuery()));
  connect(xmlButton, SIGNAL(clicked()), this, SLOT(ChangeXML()));
  connect(loadButton, SIGNAL(clicked()), this, SLOT(Accept()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(m_pathCheckBox, SIGNAL(stateChanged(int)),
      this, SLOT(PathChecked()));
  connect(m_debugCheckBox, SIGNAL(stateChanged(int)),
      this, SLOT(DebugChecked()));

  GetAssociatedFiles(_filename);
}

/*------------------------------- Slots --------------------------------------*/

void
FileListDialog::
ChangeEnv() {
  QString fn = QFileDialog::getOpenFileName(this, "Choose an environment file",
      GetMainWindow()->GetLastDir(), "Env File (*.env)");
  if(!fn.isEmpty()) {
    m_envFilename->setText(fn);
    m_envCheckBox->setChecked(true);
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
}


void
FileListDialog::
ChangeMap() {
  QString fn = QFileDialog::getOpenFileName(this, "Choose a map file",
      GetMainWindow()->GetLastDir(), "Map File (*.map)");
  if(!fn.isEmpty()) {
    m_mapFilename->setText(fn);
    m_mapCheckBox->setChecked(true);
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
}


void
FileListDialog::
ChangeQuery() {
  QString fn = QFileDialog::getOpenFileName(this, "Choose a query file",
      GetMainWindow()->GetLastDir(),"Query File (*.query)");
  if(!fn.isEmpty()) {
    m_queryFilename->setText(fn);
    m_queryCheckBox->setChecked(true);
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
}


void
FileListDialog::
ChangePath() {
  QString fn = QFileDialog::getOpenFileName(this, "Choose a path file",
      GetMainWindow()->GetLastDir(), "Path File (*.path)");
  if(!fn.isEmpty()) {
    m_pathFilename->setText(fn);
    m_pathCheckBox->setChecked(true);
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
}


void
FileListDialog::
ChangeRV() {
  QString fn = QFileDialog::getOpenFileName(this, "Choose a rv file",
      GetMainWindow()->GetLastDir(), "RV File (*.rv)");
  if(!fn.isEmpty()) {
    m_rvFilename->setText(fn);
    m_rvCheckBox->setChecked(true);
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
}


void
FileListDialog::
ChangeDebug() {
  QString fn = QFileDialog::getOpenFileName(this, "Choose a debug file",
      GetMainWindow()->GetLastDir(), "Debug File (*.vd)");
  if(!fn.isEmpty()) {
    m_debugFilename->setText(fn);
    m_debugCheckBox->setChecked(true);
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
}


void
FileListDialog::
ChangeXML() {
  QString fn = QFileDialog::getOpenFileName(this, "Choose a xml file",
      GetMainWindow()->GetLastDir(), "XML File (*.xml)");
  if(!fn.isEmpty()) {
    m_xmlFilename->setText(fn);
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
}


void
FileListDialog::
Accept() {
  if(!m_envCheckBox->isChecked())
    GetMainWindow()->AlertUser("No Environment File Loaded.");

  //Set vizmo filenames
  GetVizmo().SetEnvFileName(m_envCheckBox->isChecked() ?
      m_envFilename->text().toStdString() : "");
  GetVizmo().SetMapFileName(m_mapCheckBox->isChecked() ?
      m_mapFilename->text().toStdString() : "");
  GetVizmo().SetQryFileName(m_queryCheckBox->isChecked() ?
      m_queryFilename->text().toStdString() : "");
  GetVizmo().SetPathFileName(m_pathCheckBox->isChecked() ?
      m_pathFilename->text().toStdString() : "");
  GetVizmo().SetRVFileName(m_rvCheckBox->isChecked() ?
      m_rvFilename->text().toStdString() : "");
  GetVizmo().SetDebugFileName(m_debugCheckBox->isChecked() ?
      m_debugFilename->text().toStdString() : "");
  GetVizmo().SetXMLFileName(m_xmlFilename->text().toStdString());
  accept();
}


void
FileListDialog::
PathChecked() {
  if(m_pathCheckBox->isChecked())
    m_debugCheckBox->setChecked(false);
}


void
FileListDialog::
DebugChecked() {
  if(m_debugCheckBox->isChecked())
    m_pathCheckBox->setChecked(false);
}


void
FileListDialog::
ParseXML(const string& _xmlfile, string& _envfile, string& _queryfile) {
  // Read in the motion planning node.
  XMLNode mpNode(_xmlfile, "MotionPlanning");

  for(auto& child1 : mpNode) {
    // Find the Input node.
    if(child1.Name() != "Input")
      continue;

    for(auto& child2 : child1) {
      // Find the environment node.
      if(child2.Name() == "Environment")
        _envfile = GetPathName(_xmlfile) +
          child2.Read("filename", false, "", "env filename");
      // Find the query node.
      else if(child2.Name() == "Query")
        _queryfile = GetPathName(_xmlfile) +
          child2.Read("filename", false, "", "query filename");
    }
  }
}

/*----------------------------- Helpers --------------------------------------*/

void
FileListDialog::
GetAssociatedFiles(const vector<string>& _filename) {
  // First get XML file and associated env and query.
  string envname, queryname, xmlname = GetVizmo().GetXMLFileName();
  if(xmlname.empty()) {
    // If no XML file is specified, load the default.
    const string path = QCoreApplication::applicationDirPath().toStdString();
    if(path == "/usr/bin")
      // This is a system-installed version.
#ifdef PMPState
      xmlname = "/usr/share/vizmo/StateExamples.xml";
#else
      xmlname = "/usr/share/vizmo/VizmoExamples.xml";
#endif
    else
      // This is a working copy.
#ifdef PMPState
      xmlname = path + "/Examples/StateExamples.xml";
#else
      xmlname = path + "/Examples/VizmoExamples.xml";
#endif
  }
  ParseXML(xmlname, envname, queryname);


  // Next get vizmo's env and query, overwriting xml's if different.
  if(!GetVizmo().GetEnvFileName().empty())
    envname = GetVizmo().GetEnvFileName();
  if(!GetVizmo().GetQryFileName().empty())
    queryname = GetVizmo().GetQryFileName();

  string mapname = GetVizmo().GetMapFileName();
  string pathname = GetVizmo().GetPathFileName();
  string rvname = GetVizmo().GetRVFileName();
  string debugname = GetVizmo().GetDebugFileName();

  // Grab new files
  for(const auto& s : _filename) {
    // Extract base file name.
    size_t pos = s.rfind('.');
    string name = s.substr(0, pos);
    string ext = s.substr(pos);
    if(s.substr(pos, s.length()) == ".path") {
      size_t pos2 = name.rfind('.');
      string subname = name.substr(pos2, name.length());
      if(subname == ".full" || subname == ".rdmp")
          name = name.substr(0, pos2);
    }

    // Only pick up XML files explicitly.
    if(ext == ".xml" && FileExists(s))
      xmlname = s;

    if(FileExists(name + ".env"))
      envname = name + ".env";

    if(FileExists(name + ".map")) {
      mapname = name + ".map";
      envname = ParseMapHeader(mapname);
    }

    if(FileExists(name + ".query"))
      queryname = name + ".query";

    if(FileExists(name + ".path"))
      pathname = name + ".path";
    else if(FileExists(name + ".full.path"))
      pathname = name + ".full.path";
    else if(FileExists(name + ".rdmp.path"))
      pathname = name + ".rdmp.path";

    if(FileExists(name + ".vd"))
      debugname = name + ".vd";
  }

  m_envFilename->setText(envname.c_str());
  m_envCheckBox->setChecked(!envname.empty());

  m_mapFilename->setText(mapname.c_str());
  m_mapCheckBox->setChecked(!mapname.empty());

  m_queryFilename->setText(queryname.c_str());
  m_queryCheckBox->setChecked(!queryname.empty());

  m_pathFilename->setText(pathname.c_str());
  m_pathCheckBox->setChecked(!pathname.empty());

  m_debugFilename->setText(debugname.c_str());
  m_debugCheckBox->setChecked(pathname.empty() && !debugname.empty());

  m_xmlFilename->setText(xmlname.c_str());
}

/*----------------------------------------------------------------------------*/
