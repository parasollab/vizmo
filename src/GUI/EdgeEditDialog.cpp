#include "EdgeEditDialog.h"

#include <sstream>
#include <algorithm>

#include "NodeEditDialog.h"
#include "GLWidget.h"
#include "Models/EdgeModel.h"
#include "Models/Vizmo.h"

CfgListWidget::CfgListWidget(QWidget* _parent)
  : QListWidget(_parent) {

  connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(SelectInMap()));
}

void
CfgListWidget::SelectInMap(){ //Display intermediate cfg selection on map

  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  for(IIT i = m_items.begin(); i != m_items.end(); i++){
    if((*i)->isSelected())
      sel.push_back((*i)->m_cfg);
    else
      sel.erase(remove(sel.begin(), sel.end(), (*i)->m_cfg), sel.end());
  }
  emit CallUpdateGL();
}

EdgeEditDialog::EdgeEditDialog(QWidget* _parent, EdgeModel* _edge, GLWidget* _scene)
: QDialog(_parent) {

  setWindowTitle("Modify Edge");
  setFixedWidth(480);
  setFixedHeight(200);

  m_glScene = _scene;
  m_currentEdge = _edge;

  m_intermediatesList = new CfgListWidget(this);

  QPushButton* editIntermediateButton = new QPushButton(this);
  editIntermediateButton->setFixedWidth(70);
  editIntermediateButton->setText("Edit...");

  QPushButton* addIntermediateButton = new QPushButton(this);
  addIntermediateButton->setFixedWidth(70);
  addIntermediateButton->setText("Add...");
  addIntermediateButton->setToolTip("Add an intermediate configuration after the one currently selected in the list.");

  QPushButton* removeIntermediateButton = new QPushButton(this);
  removeIntermediateButton->setFixedWidth(70);
  removeIntermediateButton->setText("Remove");
  removeIntermediateButton->setToolTip("Remove the currently selected configuration.");

  QPushButton* doneButton = new QPushButton(this);
  doneButton->setFixedWidth(90);
  doneButton->setText("Done");

  QPushButton* cancelButton = new QPushButton(this);
  cancelButton->setFixedWidth(90);
  cancelButton->setText("Cancel");

  QLabel* edgeLabel = new QLabel(this);
  edgeLabel->setText(_edge->Name().c_str());

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->setAlignment(Qt::AlignLeft);
  buttonLayout->addWidget(editIntermediateButton);
  buttonLayout->addWidget(addIntermediateButton);
  buttonLayout->addWidget(removeIntermediateButton);
  buttonLayout->insertSpacing(3, 50);
  buttonLayout->addWidget(doneButton);
  buttonLayout->addWidget(cancelButton);

  QVBoxLayout* overallLayout = new QVBoxLayout();
  overallLayout->addWidget(edgeLabel);
  overallLayout->addWidget(m_intermediatesList);
  overallLayout->addLayout(buttonLayout);

  this->setLayout(overallLayout);

  ResetIntermediates();

  connect(editIntermediateButton, SIGNAL(clicked()), this, SLOT(EditIntermediate()));
  connect(addIntermediateButton, SIGNAL(clicked()), this, SLOT(AddIntermediate()));
  connect(removeIntermediateButton, SIGNAL(clicked()), this, SLOT(RemoveIntermediate()));
  connect(doneButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_intermediatesList, SIGNAL(CallUpdateGL()), m_glScene, SLOT(updateGL()));
}

EdgeEditDialog::~EdgeEditDialog(){}

void
EdgeEditDialog::ClearIntermediates(){

  m_intermediatesList->clear();
  m_intermediatesList->GetListItems().clear();
}

void
EdgeEditDialog::ResetIntermediates(){

  ClearIntermediates();

  vector<CfgModel>& intermediates = m_currentEdge->GetIntermediates();
  if(intermediates.empty()){
    //Assign to start cfg so that new one can be added right after it
    CfgListItem* defaultItem = new CfgListItem(m_intermediatesList, m_currentEdge->GetStartCfg());
    defaultItem->setText("There are no intermediate configurations.\n");
    m_intermediatesList->addItem(defaultItem);
    m_intermediatesList->GetListItems().push_back(defaultItem);
  }
  else{
    //Add intermediates to list
    typedef vector<CfgModel>::iterator CIT;
    int i = 0;
    for(CIT cit = intermediates.begin(); cit != intermediates.end(); cit++){
      CfgListItem* interItem = new CfgListItem(m_intermediatesList, &(*cit));
      ostringstream oss;
      oss << "Intermediate cfg " << i;
      interItem->setText(QString::fromStdString(oss.str()));
      m_intermediatesList->addItem(interItem);
      m_intermediatesList->GetListItems().push_back(interItem);
      i++;
    }
  }
  m_glScene->updateGL();
}

void
EdgeEditDialog::EditIntermediate(){

  vector<CfgListItem*>& listItems = m_intermediatesList->GetListItems();
  vector<EdgeModel*> currentEdge; //For NodeEditDialog's validity checks

  for(IIT it = listItems.begin(); it != listItems.end(); it++){
    currentEdge.clear();
    //Default list item has start cfg as m_cfg, so check against that
    if((*it)->isSelected() && (*it)->m_cfg != m_currentEdge->GetStartCfg()){
      currentEdge.push_back(m_currentEdge);
      NodeEditDialog n(this, (*it)->m_cfg, m_glScene, "Intermediate Configuration");
      n.SetCurrentEdges(&currentEdge);
      n.exec();
      break;
    }
  }
}

void
EdgeEditDialog::AddIntermediate(){

  vector<CfgListItem*>& listItems = m_intermediatesList->GetListItems();
  int indexAhead = 1;

  for(IIT it = listItems.begin(); it != listItems.end(); it++){
    if((*it)->isSelected()){

      vector<CfgModel>& allInts = m_currentEdge->GetIntermediates();
      vector<CfgModel>::iterator pos = std::find(allInts.begin(), allInts.end(), *((*it)->m_cfg));
      allInts.insert(pos++, *((*it)->m_cfg)); //insertion is before, so must increment

      ResetIntermediates();

      if(m_intermediatesList->count() == 1) //There were no intermediates before, so the new first one is edited
        m_intermediatesList->item(0)->setSelected(true);
      else //There were intermediates before, so the newly inserted one AFTER selected one is edited
        m_intermediatesList->item(indexAhead)->setSelected(true);

      EditIntermediate();
      return;
    }
    indexAhead++;
  }
}

void
EdgeEditDialog::RemoveIntermediate(){

  vector<CfgListItem*>& listItems = m_intermediatesList->GetListItems();

  for(IIT it = listItems.begin(); it != listItems.end(); it++){
    if((*it)->isSelected()){

      vector<CfgModel>& allInts = m_currentEdge->GetIntermediates();
      vector<CfgModel>::iterator pos = std::find(allInts.begin(), allInts.end(), *(*it)->m_cfg);

      if(pos != allInts.end()){
        allInts.erase(pos);
        ResetIntermediates();
      }
      return;
    }
  }
}
