#include "vizmo2.h"
#include "attributeselectio_gui.h"

///////////////////////////////////////////////////////////////////////////////// 
//// Include Qt Headers
#include <qapplication.h>
#include <qpixmap.h>
#include <qaction.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qlistview.h>
#include <qgrid.h>



VizmoAttributeSelectionGUI::VizmoAttributeSelectionGUI(QMainWindow *parent,char *name)
:QToolBar("AttributeSelection",parent,QMainWindow::Right,true,name)
{

  this->setLabel("Attribute  Selection");
  list=new QListView(this,"blah");
  list->addColumn("Attribute");
  list->addColumn("Value");

  setEnabled(false);
  
}
