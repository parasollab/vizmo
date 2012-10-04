#include "ItemSelectionGUI.h"

///////////////////////////////////////////////////////////////////////////////// 
//// Include Qt Headers
#include <qlabel.h>
#include <qvalidator.h>
#include <qstring.h>
#include <QListView>  

#include "Plum/MapObj/CCModel.h"

VizmoItemSelectionGUI::VizmoItemSelectionGUI(QString _title, QWidget* _parent)
  :QToolBar("Item Selection", _parent)  
{
  setMinimumSize(205, 277);  
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding); 
  setLabel("Outline View");
  m_maxNoModels = 0;
  m_listview=new Q3ListView(this,"");
  m_listview->setMinimumSize(205, 277);  
  m_listview->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding); 

  m_listview->addColumn("Environment Objects");
  m_listview->setColumnWidthMode(0,Q3ListView::Manual);
  m_listview->setRootIsDecorated( TRUE );
  m_listview->setMultiSelection(true);
  m_listview->setSelectionMode(Q3ListView::Extended);
  m_listview->setColumnWidth (0,205);
  connect(m_listview,SIGNAL(selectionChanged()),this,SLOT(selectionChanged()));
  setEnabled(false);
}

void 
VizmoItemSelectionGUI::reset(){

  vector<PlumObject*>& objs=GetVizmo().GetPlumObjects();

  //remove everything
  clear();
  fillTree(objs);

  //disable/enable the toolbar
  if(objs.empty() ) setEnabled(false);
  else setEnabled(true);
}

void 
VizmoItemSelectionGUI::fillTree(vector<PlumObject*>& _obj){

  typedef vector<PlumObject*>::iterator PIT;
  for(PIT i=_obj.begin();i!=_obj.end();i++){
    CGLModel* m = (*i)->getModel();
    if(m == NULL) 
      continue;
    createItem(NULL,m);
  }//end for
//    trim();
}

VizmoListViewItem *
VizmoItemSelectionGUI::createItem(VizmoListViewItem* _p, CGLModel* _model)
{

  VizmoListViewItem* item=NULL;
  if(_p==NULL){
    item=new VizmoListViewItem(m_listview);
    item->setOpen(true);
  }
  else item=new VizmoListViewItem(_p);

  item->m_model=_model;
  item->setText(0,_model->GetName().c_str());
  items.push_back(item);

  list<CGLModel*> objlist;
  _model->GetChildren(objlist);
  if(objlist.empty()) 
      return item;
  typedef list<CGLModel*>::iterator OIT;
  for(OIT i=objlist.begin(); i != objlist.end(); i++)
    createItem(item,*i);
  return item;
}

void 
VizmoItemSelectionGUI::selectionChanged(){

  vector<gliObj>& sel=GetVizmo().GetSelectedItem();
  sel.clear();
  typedef vector<VizmoListViewItem*>::iterator IIT;
  for(IIT i=items.begin(); i!=items.end(); i++){
    if(!m_listview->isSelected((*i))) 
      continue;
    GetVizmo().GetSelectedItem().push_back((*i)->m_model);
  }
  emit callUpdate();
}

void 
VizmoItemSelectionGUI::clear(){
    
  if(m_listview!=NULL) 
    m_listview->clear();
  items.clear();
}

void 
VizmoItemSelectionGUI::select(){

  vector<gliObj> sel=GetVizmo().GetSelectedItem();
  int size=sel.size();
  typedef vector<VizmoListViewItem*>::iterator IIT;
  //unselect everything
  {for(IIT i=items.begin(); i!=items.end(); i++)
    m_listview->setSelected((*i), false);}
  //find select
  vector<VizmoListViewItem*> selected;
  for(int s=0 ;s<size ;s++){
    for(IIT i=items.begin(); i!=items.end(); i++){
      if(sel[s] == (*i)->m_model){
        selected.push_back(*i);
      }
    }
  }

  //select
  for(IIT i=selected.begin(); i!=selected.end(); i++)
  m_listview->setSelected((*i), true);
  GetVizmo().GetSelectedItem() = sel;
}

void 
VizmoItemSelectionGUI::trim(){

  typedef vector<VizmoListViewItem*>::iterator IIT;
  
  for(IIT i = items.begin(); i != items.end(); i++){
    if((*i)->text(0) == "MultiBody"){      
      if(((*i)->firstChild())->text(0) == "")
      (*i)->setVisible(FALSE);
      //m_listview->takeItem((*i));	   
    }
  }
}


