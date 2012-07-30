#include "ItemSelectionGUI.h"

///////////////////////////////////////////////////////////////////////////////// 
//// Include Qt Headers
#include <qlabel.h>
#include <qvalidator.h>
#include <qstring.h>

#include "Plum/MapObj/CCModel.h"

VizmoItemSelectionGUI::VizmoItemSelectionGUI(Q3MainWindow *parent,char *name)
:Q3ToolBar("ItemSelection",parent,Qt::DockLeft,true,name)
//VizmoItemSelectionGUI::VizmoItemSelectionGUI(QWidget * parent, char * name)
//:QToolBar("ItemSelection",QMainWindow::Left, parent,true,name)
{


  //QSplitter *split = new QSplitter( parent);

    setLabel("Outline View");
    maxNoModels=0;
    listview=NULL;
    listview=new Q3ListView(this,"");

    //listview=new QListView(split, "");

    listview->addColumn("Environment Objects");
    listview->setColumnWidthMode(0,Q3ListView::Manual);
    listview->setRootIsDecorated( TRUE );
    listview->setMinimumHeight(parent->height()*11/24);
    listview->setMultiSelection(true);
    listview->setSelectionMode(Q3ListView::Extended);
    listview->setColumnWidth (0,200);
    connect(listview,SIGNAL(selectionChanged()),this,SLOT(selectionChanged()));
    setEnabled(false);
}

void VizmoItemSelectionGUI::reset()
{
    vector<PlumObject*>& objs=GetVizmo().GetPlumObjects();

    //remove everything
    clear();
    fillTree(objs);

    //disable/enable the toolbar
    if(objs.empty() ) setEnabled(false);
    else setEnabled(true);
}

void VizmoItemSelectionGUI::fillTree(vector<PlumObject*>& obj)
{

    typedef vector<PlumObject*>::iterator PIT;
    for(PIT i=obj.begin();i!=obj.end();i++){
        CGLModel * m=(*i)->getModel();
        if(m == NULL) continue;
	createItem(NULL,m);
	
    }//end for
//    trim();
}

VizmoListViewItem *
VizmoItemSelectionGUI::createItem(VizmoListViewItem * p, CGLModel * model)
{

    VizmoListViewItem * item=NULL;
    if( p==NULL ){
        item=new VizmoListViewItem(listview);
        item->setOpen(true);
    }
    else item=new VizmoListViewItem(p);


    item->model=model;
    item->setText(0,model->GetName().c_str());
    items.push_back(item);

    list<CGLModel *> objlist;
    model->GetChildren(objlist);
    if( objlist.empty() ) return item;
    typedef list<CGLModel *>::iterator OIT;
    for(OIT i=objlist.begin();i!=objlist.end();i++){
        createItem(item,*i);
   }
    return item;
}

void VizmoItemSelectionGUI::selectionChanged()
{
    vector<gliObj>& sel=GetVizmo().GetSelectedItem();
    sel.clear();
    typedef list<VizmoListViewItem*>::iterator IIT;
    for( IIT i=items.begin();i!=items.end();i++ ){
        if( !listview->isSelected((*i)) ) continue;
		GetVizmo().GetSelectedItem().push_back((*i)->model);
	}
    emit callUpdate();
}

void VizmoItemSelectionGUI::clear()
{
    if( listview!=NULL ) listview->clear();
    items.clear();
}

void VizmoItemSelectionGUI::select()
{
    vector<gliObj> sel=GetVizmo().GetSelectedItem();
    int size=sel.size();
    typedef list<VizmoListViewItem*>::iterator IIT;
    //unselect everything
    {for( IIT i=items.begin();i!=items.end();i++ )
        listview->setSelected((*i), false);}
    //find select
	list<VizmoListViewItem*> selected;
    for( int s=0;s<size;s++ ){
        for( IIT i=items.begin();i!=items.end();i++ ){
            if( sel[s]==(*i)->model ) {
                selected.push_back(*i);
            }
        }//end i
    }//end s

    //select
    for( IIT i=selected.begin();i!=selected.end();i++ )
      listview->setSelected((*i), true);
    GetVizmo().GetSelectedItem()=sel;
}

void VizmoItemSelectionGUI::trim(){

  typedef list<VizmoListViewItem*>::iterator IIT;
  
  for( IIT i=items.begin();i!=items.end();i++ ){
    if((*i)->text(0) == "MultiBody"){      
      if( ((*i)->firstChild())->text(0) == "")
	(*i)->setVisible(FALSE);
	//listview->takeItem((*i));	   
    }
  }
    
}


