#include "vizmo2.h"
#include "filelistDialog.h"
#include "icon/folder.xpm"
#include "icon/eye.xpm"

#include <qlabel.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qgrid.h>
#include <qpushbutton.h>

FileListDialog::FileListDialog
(QWidget *parent, const char *name, WFlags f)
:QDialog(parent,name,true,f)
{
    QVBoxLayout* vbox = new QVBoxLayout(this,8);
    vbox->setAutoAdd(TRUE);
    QGrid* controls = new QGrid(3,QGrid::Horizontal,this);
    controls->setSpacing(1);
    QLabel* l;
	setIcon(QPixmap(icon_eye));

    ///////////////////////////////////////////////////////////////////////////
    l=new QLabel("<b>Map File</b>:",controls); //l->setAlignment(AlignCenter);
    Map_label=new QLabel(GetVizmo().getMapFileName().c_str(),controls);
    QPushButton * mapButton = new QPushButton(QIconSet(icon_folder),"Browse",controls);  
    connect(mapButton,SIGNAL(clicked()),this,SLOT(changeMap()));

    l=new QLabel("<b>Env File</b>:",controls); //l->setAlignment(Qt::SingleLine);
    Env_label = new QLabel(GetVizmo().getEnvFileName().c_str(),controls);
	QPushButton * envButton = new QPushButton(QIconSet(icon_folder),"Browse",controls);  
    connect(envButton,SIGNAL(clicked()),this,SLOT(changeEnv()));
  
    l=new QLabel("<b>Path File</b>:",controls); //l->setAlignment(AlignCenter);
    Path_label = new QLabel(GetVizmo().getPathFileName().c_str(),controls);
	QPushButton * pathButton = new QPushButton(QIconSet(icon_folder),"Browse",controls);  
    connect(pathButton,SIGNAL(clicked()),this,SLOT(changePath()));
 
    l=new QLabel("<b>Query File</b>:",controls); //l->setAlignment(AlignCenter);
    Query_label = new QLabel(GetVizmo().getQryFileName().c_str(),controls); 
	QPushButton * qryButton = new QPushButton(QIconSet(icon_folder),"Browse",controls);  
    connect(qryButton,SIGNAL(clicked()),this,SLOT(changeQry()));

	///////////////////////////////////////////////////////////////////////////
    QPushButton *go = new QPushButton("Done",controls);
    connect(go,SIGNAL(clicked()),this,SLOT(accept()));
}

void FileListDialog::changeMap()
{
    QString fn=QFileDialog::getOpenFileName(QString::null,"Env File (*.map)",this);
    if ( !fn.isEmpty() ){
		GetVizmo().setMapFileName(fn.data());
        Map_label->setText(GetVizmo().getMapFileName().c_str());
    }
}

void FileListDialog::changeEnv()
{
    QString fn=QFileDialog::getOpenFileName(QString::null,"Path File (*.env)",this);
    if ( !fn.isEmpty() ){
		GetVizmo().setEnvFileName(fn.data());
        Env_label->setText(GetVizmo().getEnvFileName().c_str());
    }
}

void FileListDialog::changePath()
{
    QString fn=QFileDialog::getOpenFileName(QString::null,"Path File (*.path)",this);
    if ( !fn.isEmpty() ){
		GetVizmo().setPathFileName(fn.data());
        Path_label->setText(GetVizmo().getPathFileName().c_str());
    }
}

void FileListDialog::changeQry()
{
    QString fn=QFileDialog::getOpenFileName(QString::null,"Query File (*.query)",this);
    if ( !fn.isEmpty() ){
		GetVizmo().setQryFileName(fn.data());
        Query_label->setText(GetVizmo().getQryFileName().c_str());
    }
}