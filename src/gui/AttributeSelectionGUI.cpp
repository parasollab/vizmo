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
#include <q3listview.h>
#include <q3grid.h>



VizmoAttributeSelectionGUI::VizmoAttributeSelectionGUI(Q3MainWindow *parent,char *name)
:Q3ToolBar("AttributeSelection",parent,Qt::DockRight,true,name)
{

  this->setLabel("Attribute  Selection");
  list=new Q3ListView(this,"blah");
  list->addColumn("Attribute");
  list->addColumn("Value");

  setEnabled(false);
  
}
