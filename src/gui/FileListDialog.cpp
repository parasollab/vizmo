#include "vizmo2.h"
#include "FileListDialog.h"
#include "icon/Folder.xpm"
#include "icon/Eye.xpm"

#include <QDialog> 
#include <QGridLayout>
#include <QLabel>
#include <QPushButton> 
#include <QFileDialog> 

FileListDialog::FileListDialog(QWidget* _parent, Qt::WFlags _f)
  :QDialog(_parent, _f){
   
    setWindowIcon(QPixmap(eye));
    
    //1. Create subwidgets 
    m_mapLabel = new QLabel("<b>Map File</b>:", this);
    m_mapFilename = new QLabel(GetVizmo().getMapFileName().c_str(), this);
    m_mapButton = new QPushButton(QIcon(folder),"Browse", this);  
    
    m_envLabel = new QLabel("<b>Env File</b>:", this);
    m_envFilename = new QLabel(GetVizmo().getEnvFileName().c_str(), this);
    m_envButton = new QPushButton(QIcon(folder), "Browse", this);  
    
    m_pathLabel = new QLabel("<b>Path File</b>:", this);
    m_pathFilename = new QLabel(GetVizmo().getPathFileName().c_str(), this);
    m_pathButton = new QPushButton(QIcon(folder), "Browse", this);  
    
    m_debugLabel = new QLabel("<b>Debug File</b>:", this);
    m_debugFilename = new QLabel(GetVizmo().getDebugFileName().c_str(), this);
    m_debugButton = new QPushButton(QIcon(folder), "Browse", this);  
    
    m_queryLabel = new QLabel("<b>Query File</b>:", this);
    m_queryFilename = new QLabel(GetVizmo().getQryFileName().c_str(), this); 
    m_queryButton = new QPushButton(QIcon(folder), "Browse", this);  
    
    m_doneButton = new QPushButton("Done", this);
    
    //2. Make connections 
    connect(m_mapButton, SIGNAL(clicked()), this, SLOT(ChangeMap()));
    connect(m_envButton, SIGNAL(clicked()), this, SLOT(ChangeEnv()));
    connect(m_pathButton, SIGNAL(clicked()), this, SLOT(ChangePath()));
    connect(m_debugButton, SIGNAL(clicked()), this, SLOT(ChangeDebug()));
    connect(m_queryButton, SIGNAL(clicked()), this, SLOT(ChangeQry()));
    connect(m_doneButton, SIGNAL(clicked()), this, SLOT(accept()));

    //3. Set up layout
    SetUpLayout(); 
}

void
FileListDialog::SetUpLayout(){

 m_layout = new QGridLayout;
 this->setLayout(m_layout);

 m_layout->addWidget(m_mapLabel, 1, 1);
 m_layout->addWidget(m_mapFilename, 1, 2); 
 m_layout->addWidget(m_mapButton, 1, 3); 
 
 m_layout->addWidget(m_envLabel, 2, 1);
 m_layout->addWidget(m_envFilename, 2, 2);
 m_layout->addWidget(m_envButton, 2, 3);
 
 m_layout->addWidget(m_pathLabel, 3, 1);
 m_layout->addWidget(m_pathFilename, 3, 2);
 m_layout->addWidget(m_pathButton, 3, 3); 
 
 m_layout->addWidget(m_debugLabel, 4, 1); 
 m_layout->addWidget(m_debugFilename, 4, 2); 
 m_layout->addWidget(m_debugButton, 4, 3); 
 
 m_layout->addWidget(m_queryLabel, 5, 1); 
 m_layout->addWidget(m_queryFilename, 5, 2); 
 m_layout->addWidget(m_queryButton, 5, 3); 
 
 m_layout->addWidget(m_doneButton, 6, 1); 
}

void 
FileListDialog::ChangeMap(){

  QString fn = QFileDialog::getOpenFileName(this, "Choose a map file", QString::null,"Map File (*.map)");
  if(!fn.isEmpty()){
    GetVizmo().setMapFileName(fn.toStdString());
    m_mapFilename->setText(GetVizmo().getMapFileName().c_str());
  }
}

void 
FileListDialog::ChangeEnv(){

  QString fn = QFileDialog::getOpenFileName(this, "Choose an environment file", QString::null,"Env File (*.env)");
  if(!fn.isEmpty()){
    GetVizmo().setEnvFileName(fn.toStdString());
    m_envFilename->setText(GetVizmo().getEnvFileName().c_str());
  }
}

void 
FileListDialog::ChangePath(){

  QString fn = QFileDialog::getOpenFileName(this, "Choose a path file", QString::null,"Path File (*.path)");
  if(!fn.isEmpty()){
    GetVizmo().setPathFileName(fn.toStdString());
    m_pathFilename->setText(GetVizmo().getPathFileName().c_str());
  }
}

void 
FileListDialog::ChangeDebug(){

  QString fn = QFileDialog::getOpenFileName(this, "Choose a debug file", QString::null,"Debug File (*.vd)");
  if(!fn.isEmpty()){
    GetVizmo().setDebugFileName(fn.toStdString());
    m_debugFilename->setText(GetVizmo().getDebugFileName().c_str());
  }
}

void 
FileListDialog::ChangeQry(){

  QString fn = QFileDialog::getOpenFileName(this, "Choose a query file", QString::null,"Query File (*.query)");
  if(!fn.isEmpty()){
    GetVizmo().setQryFileName(fn.toStdString());
    m_queryFilename->setText(GetVizmo().getQryFileName().c_str());
  }
}

