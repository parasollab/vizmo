#include "itemselection_gui.h"

///////////////////////////////////////////////////////////////////////////////// 
//// Include Qt Headers
#include <qlabel.h>
#include <qvalidator.h>
#include <qstring.h>

VizmoItemSelectionGUI::VizmoItemSelectionGUI(QMainWindow *parent,char *name)
:QToolBar("ItemSelection",parent,QMainWindow::Left,true,name)
//VizmoItemSelectionGUI::VizmoItemSelectionGUI(QWidget * parent, char * name)
//:QToolBar("ItemSelection",QMainWindow::Left, parent,true,name)
{


  //QSplitter *split = new QSplitter( parent);

    setLabel("Outline View");
    maxNoModels=0;
    listview=NULL;
    listview=new QListView(this,"");

    //listview=new QListView(split, "");

    listview->addColumn("Objects");
    listview->setColumnWidthMode(0,QListView::Maximum);
    listview->setRootIsDecorated( TRUE );
    listview->setMinimumHeight(parent->height()*2/3);

    listview->setResizeMode(QListView::AllColumns);

    connect(listview,SIGNAL(selectionChanged(QListViewItem *)),
            this,SLOT(selectionChanged(QListViewItem *)));
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
        if( m==NULL ) continue;
        createItem(NULL,m);
    }//end for
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
    for(OIT i=objlist.begin();i!=objlist.end();i++)
        createItem(item,*i);
    return item;
}

void VizmoItemSelectionGUI::selectionChanged(QListViewItem * item)
{
    VizmoListViewItem * myitem=(VizmoListViewItem *)item;
    vector<gliObj>& sel=GetVizmo().GetSelectedItem();
    sel.clear();
    sel.push_back(myitem->model);
    emit callUpdate();
}

void VizmoItemSelectionGUI::clear()
{
    if( listview!=NULL ) listview->clear();
    items.clear();
}

void VizmoItemSelectionGUI::select()
{
    vector<gliObj>& sel=GetVizmo().GetSelectedItem();
    int size=sel.size();
    typedef list<VizmoListViewItem*>::iterator IIT;
    //unselect everything
    for( IIT i=items.begin();i!=items.end();i++ )
        listview->setSelected((*i), false);
    //select
    for( int s=0;s<size;s++ ){
        for( IIT i=items.begin();i!=items.end();i++ ){
            if( sel[s]==(*i)->model ) {
                listview->setSelected( (*i), true );
                break;
            }
        }//end i
    }//end s
}


