#include "vizmo2.h"
#include "roadmapShape.h"

///////////////////////////////////////////////////////////////////////////////// 
//// Include Qt Headers
#include <qapplication.h>
#include <qaction.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qtoolbutton.h>
#include <qwidget.h>
#include <qinputdialog.h>

///////////////////////////////////////////////////////////////////////////////
//Icons
#include "icon/tapes.xpm"

VizmoRoadmapNodesShapeGUI::VizmoRoadmapNodesShapeGUI(QMainWindow *parent,char *name)
:QToolBar("ChangeNodesShape",parent,QMainWindow::Right,true,name){

	QToolButton * nodesize= new QToolButton
	(QPixmap(tapes), "Node Size", "Change roadmap node size", this,
     SLOT(changeSize()), this, "node");
    nodesize->setUsesTextLabel ( true );

    setLabel("Roadmap Nodes");
    l =  new QListBox( this );
    l->setFocusPolicy( QWidget::StrongFocus ); 
    l->insertItem( QString::fromLatin1( "Robot" ));
    l->insertItem( QString::fromLatin1( "Box" ));
    l->insertItem( QString::fromLatin1( "Point" ));
	l->setSelected(2,true);
    connect(l,SIGNAL(clicked(QListBoxItem *)),this,SLOT(getSelectedItem()));
    setEnabled(false);
	size=0.5;
}

void VizmoRoadmapNodesShapeGUI::reset()
{
    if( GetVizmo().IsRoadMapLoaded() ) setEnabled(true);
    else setEnabled(false);
}


void VizmoRoadmapNodesShapeGUI::getSelectedItem()
{
    string s;
    for ( unsigned int i = 0; i < l->count(); i++ ) {
        QListBoxItem *item = l->item( i );
        // if the item is selected...
        if ( item->selected() )
            s = (string)item->text();
    }
    GetVizmo().ChangeNodesShape(s);
    emit callUpdate(); //set an updat event
}


void VizmoRoadmapNodesShapeGUI::changeSize(){
    
    bool ok = false;
    size = QInputDialog::getDouble(tr("Change Roadmap Node Size"), 
                                       tr("Enter a positive number scale the nodes"),
                                       size, 0, 1, 1e10,  &ok,  this);
    if(ok){
        string shape;
        for ( unsigned int i = 0; i < l->count(); i++ ){
            QListBoxItem *item = l->item( i );
            // if the item is selected...
            if ( item->selected() )
                shape = (string)item->text();
        }
        GetVizmo().ChangeNodesSize(size, shape);
	    emit callUpdate(); //set an updat event
    }
}