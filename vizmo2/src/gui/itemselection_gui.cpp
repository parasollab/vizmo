#include "vizmo2.h"
#include "main_win.h"


#include "itemselection_gui.h"

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
//#include <qchar.h>



VizmoItemSelectionGUI::VizmoItemSelectionGUI(QMainWindow *parent,char *name)
:QToolBar("ItemSelection",parent,QMainWindow::Left,true,name)
{

  this->setLabel("Item Selection");
  list=new QListView(this,"blah");
  list->addColumn("Object");
  list->addColumn("tempNo");

  setEnabled(false);


  maxNoModels=0;
  

}

void VizmoItemSelectionGUI::reset()
{
  maxNoModels=GetVizmo().GetNoEnvObjects();
  
  fillTree();
  
  //disable/enable the toolbar
  if(maxNoModels==0) setEnabled(false);
  else setEnabled(true);
}

void VizmoItemSelectionGUI::fillTree()
{
  const CMultiBodyInfo *multiBodyInfo;
  string dirstring;
  int dirstringlen;

  multiBodyInfo=GetVizmo().GetMultiBodyInfo(dirstring);
  dirstringlen=dirstring.length();
  char *name2 = new char[100];

  char index[1];
  
   QChar c;
  
  for(int i=0;i<maxNoModels;i++)
    {
      QListViewItem *parent;
      string name =multiBodyInfo[i].m_pBodyInfo[0].m_strModelDataFileName;
      cout<<name.length();
      for(int j=0;j<name.length()-dirstringlen;j++)
	name2[j]=name[j+dirstringlen+1];
      name2[name.length()-dirstringlen-1]='\0';
      
      //      index[0]=(char)i;
      c='1';
      QString *temp = new QString(c);
      parent=new QListViewItem(list,name2,c);
    }

}
