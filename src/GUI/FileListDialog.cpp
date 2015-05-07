#include "FileListDialog.h"

#include "MainWindow.h"

#include "Models/Vizmo.h"
#include "Models/CfgModel.h"
#include "Models/EdgeModel.h"
#include "Models/MapModel.h"

#include "Icons/Folder.xpm"
#include "Icons/Vizmo.xpm"

FileListDialog::
FileListDialog(const vector<string>& _filename, QWidget* _parent, Qt::WFlags _f) :
    QDialog(_parent, _f), m_xmlMode(false), m_setupWidgets(false) {
  setWindowIcon(QPixmap(vizmoIcon));
  GetAssociatedFiles(_filename);
}

void
FileListDialog::
GetAssociatedFiles(const vector<string>& _filename) {
  //empty filename provided, so grab previous values
  if(_filename.empty()) {

    SetUpSubwidgets();

    string envname = GetVizmo().GetEnvFileName();
    m_envFilename->setText(envname.c_str());
    m_envCheckBox->setChecked(!envname.empty());

    string mapname = GetVizmo().GetMapFileName();
    m_mapFilename->setText(mapname.c_str());
    m_mapCheckBox->setChecked(!mapname.empty());

    string queryname = GetVizmo().GetQryFileName();
    m_queryFilename->setText(queryname.c_str());
    m_queryCheckBox->setChecked(!queryname.empty());

    string pathname = GetVizmo().GetPathFileName();
    m_pathFilename->setText(pathname.c_str());
    m_pathCheckBox->setChecked(!pathname.empty());

    string debugname = GetVizmo().GetDebugFileName();
    m_debugFilename->setText(debugname.c_str());
    m_debugCheckBox->setChecked(!debugname.empty());
  }
  //grab new files
  else {
    for(vector<string>::const_iterator myIter = _filename.begin();
        myIter != _filename.end(); ++myIter) {
      string name = (*myIter).substr(0, (*myIter).rfind('.'));
      string envname = name + ".env";
      string mapname = name + ".map";
      string queryname = name + ".query";
      string pathname = name + ".path";
      string debugname = name + ".vd";
      string xmlname = name + ".xml";

      //test if files exist
      //first, if xml file, load that only
      if(FileExists(xmlname)) {
        m_xmlMode = true;

        SetUpSubwidgets();

        m_xmlFilename->setText(xmlname.c_str());
        m_xmlCheckBox->setChecked(true);
        break;
      }
      else {

        if(!m_setupWidgets)
          SetUpSubwidgets();

        if(FileExists(mapname)) {
          m_mapFilename->setText(mapname.c_str());
          m_mapCheckBox->setChecked(true);
          envname = ParseMapHeader(mapname);
          cout << "MapName::" << mapname << endl;
          cout << "EnvName::" << envname << endl;
        }

        if(FileExists(envname)) {
          m_envFilename->setText(envname.c_str());
          m_envCheckBox->setChecked(true);
        }

        if(FileExists(queryname)) {
          m_queryFilename->setText(queryname.c_str());
          m_queryCheckBox->setChecked(true);
        }

        bool p = FileExists(pathname);
        bool d = FileExists(debugname);
        if(p)
          m_pathFilename->setText(pathname.c_str());
        if(d)
          m_debugFilename->setText(debugname.c_str());
        m_pathCheckBox->setChecked(p && !d);
        m_debugCheckBox->setChecked(d && !p);
      }
    }
  }
}

void
FileListDialog::
SetUpSubwidgets() {

  m_setupWidgets = true;

  QGridLayout* layout = new QGridLayout;
  setLayout(layout);

  QPushButton* loadButton = new QPushButton("Load", this);
  QPushButton* cancelButton = new QPushButton("Cancel", this);

  if(!m_xmlMode) {
    m_envCheckBox = new QCheckBox(this);
    m_envFilename = new QLabel(this);
    QLabel* envLabel = new QLabel("<b>Env File</b>:", this);
    QPushButton* envButton = new QPushButton(QIcon(folder), "Browse", this);

    m_mapCheckBox = new QCheckBox(this);
    m_mapFilename = new QLabel(this);
    QLabel* mapLabel = new QLabel("<b>Map File</b>:", this);
    QPushButton* mapButton = new QPushButton(QIcon(folder),"Browse", this);

    m_queryCheckBox = new QCheckBox(this);
    m_queryFilename = new QLabel(this);
    QLabel* queryLabel = new QLabel("<b>Query File</b>:", this);
    QPushButton* queryButton = new QPushButton(QIcon(folder), "Browse", this);

    m_pathCheckBox = new QCheckBox(this);
    m_pathFilename = new QLabel(this);
    QLabel* pathLabel = new QLabel("<b>Path File</b>:", this);
    QPushButton* pathButton = new QPushButton(QIcon(folder), "Browse", this);

    m_debugCheckBox = new QCheckBox(this);
    m_debugFilename = new QLabel(this);
    QLabel* debugLabel = new QLabel("<b>Debug File</b>:", this);
    QPushButton* debugButton = new QPushButton(QIcon(folder), "Browse", this);

    layout->addWidget(m_envCheckBox, 0, 0);
    layout->addWidget(m_envFilename, 0, 2, 1, 3);
    layout->addWidget(envLabel, 0, 1);
    layout->addWidget(envButton, 0, 5);

    layout->addWidget(m_mapCheckBox, 1, 0);
    layout->addWidget(m_mapFilename, 1, 2, 1, 3);
    layout->addWidget(mapLabel, 1, 1);
    layout->addWidget(mapButton, 1, 5);

    layout->addWidget(m_queryCheckBox, 2, 0);
    layout->addWidget(m_queryFilename, 2, 2, 1, 3);
    layout->addWidget(queryLabel, 2, 1);
    layout->addWidget(queryButton, 2, 5);

    layout->addWidget(m_pathCheckBox, 3, 0);
    layout->addWidget(m_pathFilename, 3, 2, 1, 3);
    layout->addWidget(pathLabel, 3, 1);
    layout->addWidget(pathButton, 3, 5);

    layout->addWidget(m_debugCheckBox, 4, 0);
    layout->addWidget(m_debugFilename, 4, 2, 1, 3);
    layout->addWidget(debugLabel, 4, 1);
    layout->addWidget(debugButton, 4, 5);

    loadButton->setFixedWidth(cancelButton->width());
    layout->addWidget(loadButton, 5, 4);
    layout->addWidget(cancelButton, 5, 5);

    connect(mapButton, SIGNAL(clicked()), this, SLOT(ChangeMap()));
    connect(envButton, SIGNAL(clicked()), this, SLOT(ChangeEnv()));
    connect(pathButton, SIGNAL(clicked()), this, SLOT(ChangePath()));
    connect(debugButton, SIGNAL(clicked()), this, SLOT(ChangeDebug()));
    connect(queryButton, SIGNAL(clicked()), this, SLOT(ChangeQuery()));
    connect(loadButton, SIGNAL(clicked()), this, SLOT(Accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(m_pathCheckBox, SIGNAL(stateChanged(int)),
        this, SLOT(PathChecked()));
    connect(m_debugCheckBox, SIGNAL(stateChanged(int)),
        this, SLOT(DebugChecked()));
  }
  else {
    m_xmlCheckBox = new QCheckBox(this);
    m_xmlFilename = new QLabel(this);
    QLabel* xmlLabel = new QLabel("<b>XML File</b>:", this);
    QPushButton* xmlButton = new QPushButton(QIcon(folder), "Browse", this);

    m_envFilename = new QLabel(this);
    m_queryFilename = new QLabel(this);

    layout->addWidget(m_xmlCheckBox, 0, 0);
    layout->addWidget(m_xmlFilename, 0, 2, 1, 3);
    layout->addWidget(xmlLabel, 0, 1);
    layout->addWidget(xmlButton, 0, 5);

    loadButton->setFixedWidth(cancelButton->width());
    layout->addWidget(loadButton, 1, 4);
    layout->addWidget(cancelButton, 1, 5);

    connect(xmlButton, SIGNAL(clicked()), this, SLOT(ChangeXML()));
    connect(loadButton, SIGNAL(clicked()), this, SLOT(Accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
  }
}

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

void FileListDialog::
ChangeXML() {
  QString fn = QFileDialog::getOpenFileName(this, "Choose a xml file",
      GetMainWindow()->GetLastDir(), "XML File (*.xml)");
  if(!fn.isEmpty()) {
    m_xmlFilename->setText(fn);
    m_xmlCheckBox->setChecked(true);
    QFileInfo fi(fn);
    GetMainWindow()->SetLastDir(fi.absolutePath());
  }
}

void
FileListDialog::
Accept() {
  if(m_xmlMode) {
    GetVizmo().SetXMLFileName(m_xmlFilename->text().toStdString());
    m_envFilename->setText(
        SearchXML(m_xmlFilename->text().toStdString(), "Environment").c_str());
    m_queryFilename->setText(
        SearchXML(m_xmlFilename->text().toStdString(), "Query").c_str());

    if(!m_envFilename->text().toStdString().empty())
      GetVizmo().SetEnvFileName(m_envFilename->text().toStdString());

    if(!m_queryFilename->text().toStdString().empty())
      GetVizmo().SetQryFileName(m_queryFilename->text().toStdString());

    // Pass list of sampler strategies read in from xml to Vizmo.
    GetVizmo().SetLoadedSamplers(LoadXMLSamplers(
        m_xmlFilename->text().toStdString()));

    accept();
  }
  else if(m_envCheckBox->isChecked()) {
    GetVizmo().SetEnvFileName(m_envFilename->text().toStdString());

    GetVizmo().SetMapFileName("");
    if(m_mapCheckBox->isChecked())
      GetVizmo().SetMapFileName(m_mapFilename->text().toStdString());

    GetVizmo().SetQryFileName("");
    if(m_queryCheckBox->isChecked())
      GetVizmo().SetQryFileName(m_queryFilename->text().toStdString());

    GetVizmo().SetPathFileName("");
    if(m_pathCheckBox->isChecked())
      GetVizmo().SetPathFileName(m_pathFilename->text().toStdString());

    GetVizmo().SetDebugFileName("");
    if(m_debugCheckBox->isChecked())
      GetVizmo().SetDebugFileName(m_debugFilename->text().toStdString());
    accept();
  }
  else
    GetMainWindow()->AlertUser("No Environment File Loaded.");
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

string
FileListDialog::
SearchXML(string _filename, string _key) {
  string filename = "";

  // read in the motion planning node
  XMLNode mpNode(_filename, "MotionPlanning");
  for(auto& child1 : mpNode) {
    // Read in the MPPRoblem node
    if(child1.Name() == "MPProblem") {
      for(auto& child2 : child1) {
        // If the child node is the key, something likd Environment
        if(child2.Name() == _key) {
          // Handle Environment case
          if(_key == "Environment") {
            filename = child2.Read("filename", false, "", "env filename");
          }
        }
        // Handle all other specific cases
        else if(_key == "Query") {
          if(child2.Name() == "MapEvaluators") {
            for(auto& child3 : child2) {
              if(child2.Name() == _key) {
                filename = child3.Read("queryFile", false, "",
                    "query filename");
              }
            }
          }
        }
      }
    }
  }
  return filename;
}

vector<string>
FileListDialog::
LoadXMLSamplers(string _filename) {
  vector<string> samplers;

  // Read in the motion planning node
  XMLNode mpNode(_filename, "MotionPlanning");
  for(auto& child : mpNode) {
    // Read in MPProblem node
    if(child.Name() == "MPProblem") {
      for(auto& child2 : child) {
        if(child2.Name() == "Samplers") {
          for(auto& child3 : child2) {
            samplers.push_back(
                child3.Read("label", "false", "", "sampler name"));
          }
        }
      }
    }
  }

  return samplers;
}
