#include "FileListDialog.h"

#include <QDialog>
#include <QString>
#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

#include "Models/Vizmo.h"
#include "Models/CfgModel.h"
#include "Models/EdgeModel.h"
#include "Models/MapModel.h"

#include "Icons/Folder.xpm"
#include "Icons/Eye.xpm"

FileListDialog::FileListDialog(const string& _filename,
    QWidget* _parent, Qt::WFlags _f) : QDialog(_parent, _f) {

  setWindowIcon(QPixmap(eye));

  //1. Create subwidgets
  m_envCheckBox = new QCheckBox(this);
  m_envCheckBox->setEnabled(false);
  m_envLabel = new QLabel("<b>Env File</b>:", this);
  m_envFilename = new QLabel(this);
  m_envButton = new QPushButton(QIcon(folder), "Browse", this);

  m_mapCheckBox = new QCheckBox(this);
  m_mapLabel = new QLabel("<b>Map File</b>:", this);
  m_mapFilename = new QLabel(this);
  m_mapButton = new QPushButton(QIcon(folder),"Browse", this);

  m_queryCheckBox = new QCheckBox(this);
  m_queryLabel = new QLabel("<b>Query File</b>:", this);
  m_queryFilename = new QLabel(this);
  m_queryButton = new QPushButton(QIcon(folder), "Browse", this);

  m_pathCheckBox = new QCheckBox(this);
  m_pathLabel = new QLabel("<b>Path File</b>:", this);
  m_pathFilename = new QLabel(this);
  m_pathButton = new QPushButton(QIcon(folder), "Browse", this);

  m_debugCheckBox = new QCheckBox(this);
  m_debugLabel = new QLabel("<b>Debug File</b>:", this);
  m_debugFilename = new QLabel(this);
  m_debugButton = new QPushButton(QIcon(folder), "Browse", this);

  m_loadButton = new QPushButton("Load", this);
  m_cancelButton = new QPushButton("Cancel", this);

  //2. Make connections
  connect(m_mapButton, SIGNAL(clicked()), this, SLOT(ChangeMap()));
  connect(m_envButton, SIGNAL(clicked()), this, SLOT(ChangeEnv()));
  connect(m_pathButton, SIGNAL(clicked()), this, SLOT(ChangePath()));
  connect(m_debugButton, SIGNAL(clicked()), this, SLOT(ChangeDebug()));
  connect(m_queryButton, SIGNAL(clicked()), this, SLOT(ChangeQuery()));
  connect(m_loadButton, SIGNAL(clicked()), this, SLOT(Accept()));
  connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(m_pathCheckBox, SIGNAL(stateChanged(int)), this, SLOT(PathChecked()));
  connect(m_debugCheckBox, SIGNAL(stateChanged(int)), this, SLOT(DebugChecked()));

  //3. Set up layout
  SetUpLayout();

  //4. GetAssociated filenames to reset the labels appropriately
  GetAssociatedFiles(_filename);
}

void
FileListDialog::GetAssociatedFiles(const string& _filename) {
  //empty filename provided, so grab previous values
  if(_filename.empty()) {
    string envname = GetVizmo().getEnvFileName();
    m_envFilename->setText(envname.c_str());
    m_envCheckBox->setChecked(!envname.empty());

    string mapname = GetVizmo().getMapFileName();
    m_mapFilename->setText(mapname.c_str());
    m_mapCheckBox->setChecked(!mapname.empty());

    string queryname = GetVizmo().getQryFileName();
    m_queryFilename->setText(queryname.c_str());
    m_queryCheckBox->setChecked(!queryname.empty());

    string pathname = GetVizmo().getPathFileName();
    m_pathFilename->setText(pathname.c_str());
    m_pathCheckBox->setChecked(!pathname.empty());

    string debugname = GetVizmo().getDebugFileName();
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
      MapModel<CfgModel,EdgeModel> headerParser(mapname);
      envname = headerParser.GetEnvFileName();
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
FileListDialog::SetUpLayout(){

  m_layout = new QGridLayout;
  setLayout(m_layout);

  m_layout->addWidget(m_envCheckBox, 0, 0);
  m_layout->addWidget(m_envLabel, 0, 1);
  m_layout->addWidget(m_envFilename, 0, 2, 1, 3);
  m_layout->addWidget(m_envButton, 0, 5);

  m_layout->addWidget(m_mapCheckBox, 1, 0);
  m_layout->addWidget(m_mapLabel, 1, 1);
  m_layout->addWidget(m_mapFilename, 1, 2, 1, 3);
  m_layout->addWidget(m_mapButton, 1, 5);

  m_layout->addWidget(m_queryCheckBox, 2, 0);
  m_layout->addWidget(m_queryLabel, 2, 1);
  m_layout->addWidget(m_queryFilename, 2, 2, 1, 3);
  m_layout->addWidget(m_queryButton, 2, 5);

  m_layout->addWidget(m_pathCheckBox, 3, 0);
  m_layout->addWidget(m_pathLabel, 3, 1);
  m_layout->addWidget(m_pathFilename, 3, 2, 1, 3);
  m_layout->addWidget(m_pathButton, 3, 5);

  m_layout->addWidget(m_debugCheckBox, 4, 0);
  m_layout->addWidget(m_debugLabel, 4, 1);
  m_layout->addWidget(m_debugFilename, 4, 2, 1, 3);
  m_layout->addWidget(m_debugButton, 4, 5);

  m_loadButton->setFixedWidth(m_cancelButton->width());
  m_layout->addWidget(m_loadButton, 5, 4);
  m_layout->addWidget(m_cancelButton, 5, 5);
}

void
FileListDialog::ChangeEnv(){
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
    GetVizmo().setEnvFileName(m_envFilename->text().toStdString());

    GetVizmo().setMapFileName("");
    GetVizmo().setPathFileName("");
    GetVizmo().setDebugFileName("");
    GetVizmo().setQryFileName("");

    if(m_mapCheckBox->isChecked())
      GetVizmo().setMapFileName(m_mapFilename->text().toStdString());

    if(m_queryCheckBox->isChecked())
      GetVizmo().setQryFileName(m_queryFilename->text().toStdString());

    if(m_pathCheckBox->isChecked())
      GetVizmo().setPathFileName(m_pathFilename->text().toStdString());

    if(m_debugCheckBox->isChecked())
      GetVizmo().setDebugFileName(m_debugFilename->text().toStdString());

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
