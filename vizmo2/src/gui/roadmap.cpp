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
 
  nodesSameColor= new QToolButton
    (QPixmap(shapes1), "CC's one color", "Change roadmap node's to same color", this,
     SLOT(setSameColor()), this, "node");
  nodesSameColor->setUsesTextLabel ( true );
  nodesSameColor->setEnabled(false);

}

void VizmoRoadmapGUI::reset()
{
    if( GetVizmo().IsRoadMapLoaded() ){
      nodesize->setEnabled(true);
      l->setEnabled(true);
      nodesColor->setEnabled(true);
      nodesSameColor->setEnabled(true);
    }
    else{
      nodesize->setEnabled(false);
      l->setEnabled(false);
      nodesColor->setEnabled(false);
      nodesSameColor->setEnabled(false);
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

void VizmoRoadmapGUI::setSameColor(){
  double R, G, B;
  R=G=B=1;
  string s = "all";
  GetVizmo().oneColor = true;
  QColor color = QColorDialog::getColor( white, this, "color dialog" );
  if ( color.isValid() ){
    R = (double)(color.red()) / 255.0;
    G = (double)(color.green()) / 255.0;
    B = (double)(color.blue()) / 255.0;
  }

  string shape;
  for ( unsigned int i = 0; i < l->count(); i++ ){
    QListBoxItem *item = l->item( i );
    // if the item is selected...
    if ( item->selected() )
      shape = (string)item->text().ascii();
  }

  GetVizmo().ChangeNodesColor(R, G, B, shape);
  emit callUpdate(); //set an updat event
}

void VizmoRoadmapGUI::changeColor(){

    double R, G, B;
    R=G=B=1;
  
//Check first if there is a CC selected
 
    vector<gliObj>& sel = GetVizmo().GetSelectedItem();
    typedef vector<gliObj>::iterator SI;
    int m_i;
    string m_sO;
    for(SI i = sel.begin(); i!= sel.end(); i++){
      CGLModel *gl = (CGLModel*)(*i);
      m_sO = gl->GetName();
    }
    string m_s="NULL";
    int position = -10;
    position = m_sO.find("CC",0);
    if(position != string::npos){
      QColor color = QColorDialog::getColor( white, this, "color dialog" );
      if ( color.isValid() ){
	R = (double)(color.red()) / 255.0;
	G = (double)(color.green()) / 255.0;
	B = (double)(color.blue()) / 255.0;
      }
    }
    
    string s;
    for ( unsigned int i = 0; i < l->count(); i++ ) {
      QListBoxItem *item = l->item( i );
      // if the item is selected...
        if ( item->selected() )
	  s = (string)item->text().ascii();
    }
    
    GetVizmo().ChangeNodesColor(R, G, B, s);
    emit callUpdate(); //set an updat event
}


