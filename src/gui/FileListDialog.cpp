#include "vizmo2.h"
#include "FileListDialog.h"
#include "icon/Folder.xpm"
#include "icon/Eye.xpm"

//Added by qt3to4:
#include <q3grid.h>

#include <QtGui>

FileListDialog::FileListDialog
(QWidget *parent, const char *name, Qt::WFlags f)
:QDialog(parent,name,true,f)
{
    QVBoxLayout* vbox = new QVBoxLayout(this,8);
    vbox->setAutoAdd(TRUE);
    Q3Grid* controls = new Q3Grid(3,Qt::Horizontal,this);
    controls->setSpacing(1);
    setIcon(QPixmap(eye));

    ///////////////////////////////////////////////////////////////////////////
    new QLabel("<b>Map File</b>:",controls);
    Map_label=new QLabel(GetVizmo().getMapFileName().c_str(),controls);
    QPushButton * mapButton = new QPushButton(QIcon(folder),"Browse",controls);  
    connect(mapButton,SIGNAL(clicked()),this,SLOT(changeMap()));

    new QLabel("<b>Env File</b>:",controls);
    Env_label = new QLabel(GetVizmo().getEnvFileName().c_str(),controls);
	QPushButton * envButton = new QPushButton(QIcon(folder),"Browse",controls);  
    connect(envButton,SIGNAL(clicked()),this,SLOT(changeEnv()));
  
    new QLabel("<b>Path File</b>:",controls);
    Path_label = new QLabel(GetVizmo().getPathFileName().c_str(),controls);
	QPushButton * pathButton = new QPushButton(QIcon(folder),"Browse",controls);  
    connect(pathButton,SIGNAL(clicked()),this,SLOT(changePath()));
 
    new QLabel("<b>Debug File</b>:",controls);
    Debug_label = new QLabel(GetVizmo().getDebugFileName().c_str(),controls);
	QPushButton * debugButton = new QPushButton(QIcon(folder),"Browse",controls);  
    connect(debugButton,SIGNAL(clicked()),this,SLOT(changeDebug()));
 
    new QLabel("<b>Query File</b>:",controls);
    Query_label = new QLabel(GetVizmo().getQryFileName().c_str(),controls); 
	QPushButton * qryButton = new QPushButton(QIcon(folder),"Browse",controls);  
    connect(qryButton,SIGNAL(clicked()),this,SLOT(changeQry()));

	///////////////////////////////////////////////////////////////////////////
    QPushButton *go = new QPushButton("Done",controls);
    connect(go,SIGNAL(clicked()),this,SLOT(accept()));
}

void FileListDialog::changeMap()
{
    QString fn=QFileDialog::getOpenFileName(this, "Choose a map file" , 
					    QString::null,"Map File (*.map)");
    if ( !fn.isEmpty() ){
		GetVizmo().setMapFileName(fn.toStdString());
        Map_label->setText(GetVizmo().getMapFileName().c_str());
    }
}

void FileListDialog::changeEnv()
{
    QString fn=QFileDialog::getOpenFileName(this, "Choose an environment file", 
					    QString::null,"Env File (*.env)");
    if ( !fn.isEmpty() ){
		GetVizmo().setEnvFileName(fn.toStdString());
        Env_label->setText(GetVizmo().getEnvFileName().c_str());
    }
}

void FileListDialog::changePath()
{
    QString fn=QFileDialog::getOpenFileName(this, "Choose a path file", 
					    QString::null,"Path File (*.path)");
    if ( !fn.isEmpty() ){
		GetVizmo().setPathFileName(fn.toStdString());
        Path_label->setText(GetVizmo().getPathFileName().c_str());
    }
}

void FileListDialog::changeDebug()
{
    QString fn=QFileDialog::getOpenFileName(this, "Choose a debug file", 
					    QString::null,"Debug File (*.vd)");
    if ( !fn.isEmpty() ){
		GetVizmo().setDebugFileName(fn.toStdString());
        Debug_label->setText(GetVizmo().getDebugFileName().c_str());
    }
}

void FileListDialog::changeQry()
{
    QString fn=QFileDialog::getOpenFileName(this, "Choose a query file", 
					    QString::null,"Query File (*.query)");
    if ( !fn.isEmpty() ){
		GetVizmo().setQryFileName(fn.toStdString());
        Query_label->setText(GetVizmo().getQryFileName().c_str());
    }
}

