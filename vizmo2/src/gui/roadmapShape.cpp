#include "vizmo2.h"
#include "main_win.h"


#include "roadmapShape.h"

#include <qwidget.h>

///////////////////////////////////////////////////////////////////////////////// 
//// Include Qt Headers
#include <qapplication.h>
#include <qaction.h>
#include <qlabel.h>

#include <qlistbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qstring.h>


VizmoRoadmapNodesShapeGUI::VizmoRoadmapNodesShapeGUI(QMainWindow *parent,char *name)
:QToolBar("ChangeNodesShape",parent,QMainWindow::Left,true,name){
  //:QWidget(0, name){
  this->setLabel("Roadmap Nodes");
  l =  new QListBox( this );
  l->setFocusPolicy( QWidget::StrongFocus ); 
  l->insertItem( QString::fromLatin1( "Original" ));
  l->insertItem( QString::fromLatin1( "Box" ));
  l->insertItem( QString::fromLatin1( "Point" ));

  setEnabled(false);
}
