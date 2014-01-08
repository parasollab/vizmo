#include "FileListDialog.h"

#include "Models/Vizmo.h"
#include "Models/CfgModel.h"
#include "Models/EdgeModel.h"
#include "Models/MapModel.h"

#include "Icons/Folder.xpm"
#include "Icons/Eye.xpm"

FileListDialog::FileListDialog(const string& _filename,
    QWidget* _parent, Qt::WFlags _f) : QDialog(_parent, _f) {

  setWindowIcon(QPixmap(eye));
  SetUpSubwidgets();
  GetAssociatedFiles(_filename);
}

void
FileListDialog::GetAssociatedFiles(const string& _filename) {
  //empty filename provided, so grab previous values
  if(_filename.empty()) {
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
    string name = _filename.substr(0, _filename.rfind('.'));

    string envname = name + ".env";
    string mapname = name + ".map";
    string queryname = name + ".query";
    string pathname = name + ".path";
    string debugname = name + ".vd";

    //test if files exist
    if(FileExists(mapname, false)){
      m_mapFilename->setText(mapname.c_str());
      m_mapCheckBox->setChecked(true);
      envname = ParseMapHeader(mapname);
      cout << "MapName::" << mapname << endl;
      cout << "EnvName::" << envname << endl;
    }

    if(FileExists(envname, false)){
      m_envFilename->setText(envname.c_str());
      m_envCheckBox->setChecked(true);
    }

    if(FileExists(queryname, false)){
      m_queryFilename->setText(queryname.c_str());
      m_queryCheckBox->setChecked(true);
    }

    bool p = FileExists(pathname, false);
    bool d = FileExists(debugname, false);
    if(p)
      m_pathFilename->setText(pathname.c_str());
    if(d)
      m_debugFilename->setText(debugname.c_str());
    m_pathCheckBox->setChecked(p && !d);
    m_debugCheckBox->setChecked(d && !p);
  }
}

void
FileListDialog::SetUpSubwidgets(){

  QGridLayout* layout = new QGridLayout;
  setLayout(layout);

  QPushButton* loadButton = new QPushButton("Load", this);
  QPushButton* cancelButton = new QPushButton("Cancel", this);

  m_envCheckBox = new QCheckBox(this);
  m_envCheckBox->setEnabled(false);
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
  connect(m_pathCheckBox, SIGNAL(stateChanged(int)), this, SLOT(PathChecked()));
  connect(m_debugCheckBox, SIGNAL(stateChanged(int)), this, SLOT(DebugChecked()));
}

void
FileListDialog::ChangeEnv() {
  QString fn = QFileDialog::getOpenFileName(this, "Choose an environment file", QString::null,"Env File (*.env)");
  if(!fn.isEmpty()) {
    m_envFilename->setText(fn);
    m_envCheckBox->setChecked(true);
  }
}

void
FileListDialog::ChangeMap() {
  QString fn = QFileDialog::getOpenFileName(this, "Choose a map file", QString::null,"Map File (*.map)");
  if(!fn.isEmpty()) {
    m_mapFilename->setText(fn);
    m_mapCheckBox->setChecked(true);
  }
}

void
FileListDialog::ChangeQuery() {
  QString fn = QFileDialog::getOpenFileName(this, "Choose a query file", QString::null,"Query File (*.query)");
  if(!fn.isEmpty()) {
    m_queryFilename->setText(fn);
    m_queryCheckBox->setChecked(true);
  }
}

void
FileListDialog::ChangePath() {
  QString fn = QFileDialog::getOpenFileName(this, "Choose a path file", QString::null,"Path File (*.path)");
  if(!fn.isEmpty()) {
    m_pathFilename->setText(fn);
    m_pathCheckBox->setChecked(true);
  }
}

void
FileListDialog::ChangeDebug(){
  QString fn = QFileDialog::getOpenFileName(this, "Choose a debug file", QString::null,"Debug File (*.vd)");
  if(!fn.isEmpty()) {
    m_debugFilename->setText(fn);
    m_debugCheckBox->setChecked(true);
  }
}

void
FileListDialog::Accept() {
  if(m_envCheckBox->isChecked()) {
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
  else {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Vizmo++");
    msgBox.setText("No Environment File Loaded.");
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.exec();
  }
}

void
FileListDialog::PathChecked() {
  if(m_pathCheckBox->isChecked())
    m_debugCheckBox->setChecked(false);
}

void
FileListDialog::DebugChecked() {
  if(m_debugCheckBox->isChecked())
    m_pathCheckBox->setChecked(false);
}
