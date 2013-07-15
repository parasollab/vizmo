#include "ItemSelectionGUI.h"
#include "Plum/MapObj/CCModel.h"

#include <QString>

VizmoItemSelectionGUI::VizmoItemSelectionGUI(QWidget* _parent)
  :QTreeWidget(_parent) {
  
  setMinimumSize(205, 277);  
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding); 
  m_maxNoModels = 0;
  setHeaderLabel("Environment Objects"); 
  setSelectionMode(QAbstractItemView::ExtendedSelection);  
  connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(SelectionChanged()));
  setEnabled(false);   
}

void 
VizmoItemSelectionGUI::ResetLists(){
  vector<PlumObject*>& objs=GetVizmo().GetPlumObjects();
  ClearLists();  
  FillTree(objs);
  
  if(objs.empty()) 
    setEnabled(false);
  else 
    setEnabled(true);
}

void 
VizmoItemSelectionGUI::FillTree(vector<PlumObject*>& _obj){

  typedef vector<PlumObject*>::iterator PIT;
  for(PIT i = _obj.begin(); i != _obj.end(); i++){
    GLModel* m = (*i)->GetModel();
    if(m == NULL) 
      continue;
    CreateItem(NULL,m);
  }
}

VizmoListViewItem*
VizmoItemSelectionGUI::CreateItem(VizmoListViewItem* _p, GLModel* _model)
{
  VizmoListViewItem* item = NULL;
  if(_p == NULL){
    item = new VizmoListViewItem(this);
    item->setExpanded(true); 
  }
  else item = new VizmoListViewItem(_p);  

  item->m_model = _model;
  QString qstr = QString::fromStdString(_model->GetName()); 
  item->setText(0, qstr); //Set the text to column 0, which is the only column in this tree widget  
  m_items.push_back(item);

  list<GLModel*> objlist;
  _model->GetChildren(objlist);
  if(objlist.empty()) 
    return item;
  
  typedef list<GLModel*>::iterator OIT;
  for(OIT i = objlist.begin(); i != objlist.end(); i++)
    CreateItem(item,*i);  
  return item;
}

void 
VizmoItemSelectionGUI::SelectionChanged(){
  //Selects in MAP whatever has been selected in the tree widget 
  vector<gliObj>& sel=GetVizmo().GetSelectedItem();
  sel.clear();
  typedef vector<VizmoListViewItem*>::iterator IIT;
  for(IIT i = m_items.begin(); i != m_items.end(); i++){
    if(((*i)->isSelected()))  
      GetVizmo().GetSelectedItem().push_back((*i)->m_model);
  }
  emit CallUpdate();
  emit UpdateTextGUI(); 
}

void 
VizmoItemSelectionGUI::ClearLists(){
  
  clear();      //Qt call to clear the TreeWidget          
  m_items.clear();
}

void 
VizmoItemSelectionGUI::Select(){
  //Selects in the TREE WIDGET whatever has been selected in the map 
  vector<gliObj> sel = GetVizmo().GetSelectedItem();   
  int size = sel.size();
  typedef vector<VizmoListViewItem*>::iterator IIT;
  //unselect everything
  for(IIT i = m_items.begin(); i != m_items.end(); i++)
    (*i) -> setSelected(false); 
  //find selected
  vector<VizmoListViewItem*> selected;
  for(int s=0; s<size; s++){
    for(IIT i = m_items.begin(); i != m_items.end(); i++){
      if(sel[s] == (*i)->m_model){
        selected.push_back(*i);
      }
    }
  }
  //select
  for(IIT i=selected.begin(); i!=selected.end(); i++)  
    (*i)->setSelected(true); 
  GetVizmo().GetSelectedItem() = sel;
}


