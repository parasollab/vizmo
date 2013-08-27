#include "ModelSelectionWidget.h"

#include <QString>

#include "Models/CCModel.h"
#include "Models/Vizmo.h"

ModelSelectionWidget::ModelSelectionWidget(QWidget* _parent) :
  QTreeWidget(_parent) {
    setMinimumSize(205, 277);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_maxNoModels = 0;
    setHeaderLabel("Environment Objects");
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(SelectionChanged()));
    setEnabled(false);
  }

void
ModelSelectionWidget::ResetLists(){
  vector<GLModel*>& objs = GetVizmo().GetLoadedModels();
  ClearLists();
  FillTree(objs);
  setEnabled(!objs.empty());
}

void
ModelSelectionWidget::FillTree(vector<GLModel*>& _obj){
  typedef vector<GLModel*>::iterator MIT;
  for(MIT mit = _obj.begin(); mit != _obj.end(); ++mit)
    CreateItem(NULL, *mit);
}

ModelSelectionWidget::ListViewItem*
ModelSelectionWidget::CreateItem(ListViewItem* _p, GLModel* _model){
  ListViewItem* item = NULL;
  if(!_p){
    item = new ListViewItem(this);
    item->setExpanded(true);
  }
  else
    item = new ListViewItem(_p);

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
    CreateItem(item, *i);
  return item;
}

void
ModelSelectionWidget::SelectionChanged(){
  //Selects in MAP whatever has been selected in the tree widget
  vector<GLModel*>& sel = GetVizmo().GetSelectedModels();
  sel.clear();
  typedef vector<ListViewItem*>::iterator IIT;
  for(IIT i = m_items.begin(); i != m_items.end(); i++){
    if((*i)->isSelected()){
      sel.push_back((*i)->m_model);
      for(int j = 0; j < (*i)->childCount(); j++){ //Select all subcomponents as well
        ListViewItem* child = (ListViewItem*)(*i)->child(j);
          sel.push_back(child->m_model);
      }
    }
  }
  emit CallUpdate();
  emit UpdateTextGUI();
}

void
ModelSelectionWidget::ClearLists(){

  clear();  //Qt call to clear the TreeWidget
  m_items.clear();
}

void
ModelSelectionWidget::Select(){
  //Selects in the TREE WIDGET whatever has been selected in the map
  vector<GLModel*>& sel = GetVizmo().GetSelectedModels();
  typedef vector<ListViewItem*>::iterator IIT;

  //Unselect everything
  blockSignals(true);
  for(IIT i = m_items.begin(); i != m_items.end(); i++)
    (*i)->setSelected(false);

  //Find selected
  vector<ListViewItem*> selected;
  for(size_t s = 0; s < sel.size(); ++s){
    for(IIT i = m_items.begin(); i != m_items.end(); i++){
      if(sel[s] == (*i)->m_model){
        (*i)->setSelected(true);
        if(GetVizmo().GetDoubleClickStatus() == true){
          (*i)->parent()->setSelected(true);
          (*i)->setSelected(false);
          GetVizmo().SetDoubleClickStatus(false);
        }
      }
    }
  }
  blockSignals(false);
  emit itemSelectionChanged();
}
