#include "vizmo2.h"
#include "roadmap.h"

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
#include <qcolordialog.h> 

///////////////////////////////////////////////////////////////////////////////
//Icons
#include "icon/shapes1.xpm"

VizmoRoadmapGUI::VizmoRoadmapGUI(QMainWindow *parent,char *name)
:QToolBar(parent, name){

  this->setLabel("CC's features");
  createGUI();

}

void VizmoRoadmapGUI::createGUI()
{

  nodesize= new QToolButton
    (QPixmap(shapes1), "Node Size", "Change roadmap node size", this,
     SLOT(changeSize()), this, "node");
  nodesize->setUsesTextLabel ( true );
  nodesize->setEnabled(false);
  
  l =  new QListBox( this );
  l->setFocusPolicy( QWidget::StrongFocus ); 
  l->insertItem( QString::fromLatin1( "Robot" ));
  l->insertItem( QString::fromLatin1( "Box" ));
  l->insertItem( QString::fromLatin1( "Point" ));
  l->setSelected(2,true);
  connect(l,SIGNAL(clicked(QListBoxItem *)),this,SLOT(getSelectedItem()));
  l->setEnabled(false);
  size=0.5;
  
  nodesColor= new QToolButton
    (QPixmap(shapes1), "CC's color", "Change roadmap node's color", this,
     SLOT(changeColor()), this, "node");
  nodesColor->setUsesTextLabel ( true );
  nodesColor->setEnabled(false);
  
}

void VizmoRoadmapGUI::reset()
{
    if( GetVizmo().IsRoadMapLoaded() ){
      nodesize->setEnabled(true);
      l->setEnabled(true);
      nodesColor->setEnabled(true);
    }
    else{
      nodesize->setEnabled(false);
      l->setEnabled(false);
      nodesColor->setEnabled(false);
    }
}


void VizmoRoadmapGUI::getSelectedItem()
{
  string s;
    for ( unsigned int i = 0; i < l->count(); i++ ) {
        QListBoxItem *item = l->item( i );
        // if the item is selected...
        if ( item->selected() )
            s = (string)item->text().ascii();
    }
    m_shapeString = s;
//      cout<<"--------";
//      cout<<"S::"<<s<<endl;
//      cout<<"m_shapeString::"<<m_shapeString<<endl;
//      cout<<"--------";
    GetVizmo().ChangeNodesShape(s);

    emit callUpdate(); //set an update event
}


void VizmoRoadmapGUI::changeSize(){
    
    bool ok = false;
    size = QInputDialog::getDouble(tr("Change Roadmap Node Size"), 
                 tr("Enter a positive number to scale the nodes"),
                 size, 0, 1, 2,  &ok,  this);
    if(ok){
        string shape;
        for ( unsigned int i = 0; i < l->count(); i++ ){
            QListBoxItem *item = l->item( i );
            // if the item is selected...
            if ( item->selected() )
                shape = (string)item->text().ascii();
        }
        GetVizmo().ChangeNodesSize(size, shape);
        emit callUpdate(); //set an updat event
    }

}
void VizmoRoadmapGUI::changeColor(){

  double R, G, B;
  
 QColor color = QColorDialog::getColor( white, this, "color dialog" );
    if ( color.isValid() ){
        R = (double)(color.red()) / 255.0;
        G = (double)(color.green()) / 255.0;
        B = (double)(color.blue()) / 255.0;

	//string shape;
	//shape = "Robot";

        GetVizmo().ChangeNodesColor(R, G, B, m_shapeString);
        emit callUpdate(); //set an updat event
    }
}


