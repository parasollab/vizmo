#include "vizmo2.h"
#include "roadmapColor.h"

///////////////////////////////////////////////////////////////////////////////// 
//// Include Qt Headers
#include <qapplication.h>
#include <qaction.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qtoolbutton.h>
#include <qwidget.h>
#include <qcolordialog.h> 

///////////////////////////////////////////////////////////////////////////////
//Icons
#include "icon/shapes1.xpm"

VizmoRoadmapColorGUI::VizmoRoadmapColorGUI(QMainWindow *parent,char *name)
:QToolBar("ChangeNodesColor",parent,QMainWindow::Right,true,name){

    QToolButton * nodesColor= new QToolButton
    (QPixmap(shapes1), "CC's color", "Change roadmap node's color", this,
     SLOT(changeColor()), this, "node");
    nodesColor->setUsesTextLabel ( true );

    setLabel("CC's color");

    setEnabled(false);
    size=0.5;
}

void VizmoRoadmapColorGUI::reset()
{
    if( GetVizmo().IsRoadMapLoaded() ) setEnabled(true);
    else setEnabled(false);
}


void VizmoRoadmapColorGUI::changeColor(){

  double R, G, B;
  
  QColor color = QColorDialog::getColor( white, this, "color dialog" );
    if ( color.isValid() ){
        R = (double)(color.red()) / 255.0;
        G = (double)(color.green()) / 255.0;
        B = (double)(color.blue()) / 255.0;

	string shape;
	shape = "Robot";

        GetVizmo().ChangeNodesColor(R, G, B, shape);
        emit callUpdate(); //set an updat event
    }
}

