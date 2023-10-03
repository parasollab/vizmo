#include "ModelSelectionWidget.h"

#include "GLWidget.h"

#include "Models/CCModel.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

ModelSelectionWidget::
ModelSelectionWidget(GLWidget* _glWidget, QWidget* _parent) :
    QTreeWidget(_parent), m_glWidget(_glWidget) {
  setFixedWidth(160);
  m_maxNoModels = 0;
  setHeaderLabel("Environment Objects");
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(SelectionChanged()));
  connect(this, SIGNAL(ResetSignal()), this, SLOT(ResetLists()));
  setEnabled(false);
}

void
ModelSelectionWidget::
CallResetLists() {
  emit ResetSignal();
}

void
ModelSelectionWidget::
ResetLists() {
  blockSignals(true);
  vector<Model*>& objs = GetVizmo().GetLoadedModels();
  vector<Model*> sel = GetVizmo().GetSelectedModels();
  ClearLists();
  FillTree(objs);
  setEnabled(!objs.empty());
  GetVizmo().GetSelectedModels() = sel;
  Select();
}

void
ModelSelectionWidget::
FillTree(vector<Model*>& _obj) {
  typedef vector<Model*>::iterator MIT;
  for(MIT mit = _obj.begin(); mit != _obj.end(); ++mit)
    CreateItem(NULL, *mit);
}

ModelSelectionWidget::ListViewItem*
ModelSelectionWidget::
CreateItem(ListViewItem* _p, Model* _model) {
  ListViewItem* item = NULL;
  if(!_p){
    item = new ListViewItem(this);
    item->setExpanded(true);
  }
  else
    item = new ListViewItem(_p);

  item->m_model = _model;
  if(_model->IsSelectable() == false)
    item->setDisabled(true);

  if(_model->Name() == "Map") {
    QMutexLocker locker(&((MapModel<CfgModel, EdgeModel>*)_model)->AcquireMutex());
    QString qstr = QString::fromStdString(_model->Name());
    item->setText(0, qstr); //Set the text to column 0, which is the only column in this tree widget
    m_items.push_back(item);

    list<Model*> objlist;
    _model->GetChildren(objlist);
    if(objlist.empty()) {
      return item;
    }

    typedef list<Model*>::iterator OIT;
    for(OIT i = objlist.begin(); i != objlist.end(); i++)
      CreateItem(item, *i);

  }
  else {

    QString qstr = QString::fromStdString(_model->Name());
    item->setText(0, qstr); //Set the text to column 0, which is the only column in this tree widget
    m_items.push_back(item);

    list<Model*> objlist;
    _model->GetChildren(objlist);
    if(objlist.empty()) {
      return item;
    }

    typedef list<Model*>::iterator OIT;
    for(OIT i = objlist.begin(); i != objlist.end(); i++)
      CreateItem(item, *i);

  }

  return item;
}

void
ModelSelectionWidget::
SelectionChanged() {
  //Selects in MAP whatever has been selected in the tree widget
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
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
  emit UpdateTextWidget();
}

void
ModelSelectionWidget::
ClearLists() {
  clear();  //Qt call to clear the TreeWidget
  m_items.clear();
}

void
ModelSelectionWidget::
Select() {
  //Selects in the TREE WIDGET whatever has been selected in the map
  typedef vector<ListViewItem*>::iterator IIT;

  //Unselect everything
  blockSignals(true);
  for(IIT i = m_items.begin(); i != m_items.end(); i++)
    (*i)->setSelected(false);

  //Find selected
  vector<ListViewItem*> selected;
  m_glWidget->SetCurrentRegion();
  m_glWidget->SetCurrentUserPath(NULL);
  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  typedef vector<Model*>::iterator MIT;
  for(MIT mit = sel.begin(); mit != sel.end();/* increments conditionally below */) {
    bool found = false;
    for(IIT i = m_items.begin(); i != m_items.end(); i++){
      if(*mit == (*i)->m_model){
        found = true;
        (*i)->setSelected(true);
        if(m_glWidget->GetDoubleClickStatus() == true) {
          if((*i)->parent())
            (*i)->parent()->setSelected(true);
          (*i)->setSelected(false);
          m_glWidget->SetDoubleClickStatus(false);
        }
        m_glWidget->SetCurrentRegion(GetVizmo().GetEnv()->GetRegion(*mit));
        if((*mit)->Name() == "User Path")
          m_glWidget->SetCurrentUserPath((UserPathModel*)(*mit));
      }
    }
    if(!found)
      mit = sel.erase(mit);
    else
      ++mit;
  }
  blockSignals(false);
  emit itemSelectionChanged();
}
