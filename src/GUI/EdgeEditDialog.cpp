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
  setFixedWidth(390);
  setFixedHeight(200);

  m_glScene = _scene;

  m_overallLayout = new QVBoxLayout();
  m_buttonLayout = new QHBoxLayout();
  m_edgeLabel = new QLabel(this);
  m_intermediatesList = new CfgListWidget(this);
  m_editIntermediateButton = new QPushButton(this);
  m_addIntermediateButton = new QPushButton(this);
  m_removeIntermediateButton = new QPushButton(this);
  m_doneButton = new QPushButton(this);

  SetUpWidgets();
  SetCurrentEdge(_edge);

  connect(m_intermediatesList, SIGNAL(CallUpdateGL()), m_glScene, SLOT(updateGL()));
}

EdgeEditDialog::~EdgeEditDialog(){}

void
EdgeEditDialog::SetUpWidgets(){

  m_editIntermediateButton->setFixedWidth(70);
  m_editIntermediateButton->setText("Edit...");
  m_addIntermediateButton->setFixedWidth(70);
  m_addIntermediateButton->setText("Add...");
  m_addIntermediateButton->setToolTip("Add an intermediate configuration after the one currently selected in the list.");
  m_removeIntermediateButton->setFixedWidth(70);
  m_removeIntermediateButton->setText("Remove");
  m_removeIntermediateButton->setToolTip("Remove the currently selected configuration.");
  m_doneButton->setFixedWidth(90);
  m_doneButton->setText("Done");

  m_buttonLayout->setAlignment(Qt::AlignLeft);
  m_buttonLayout->addWidget(m_editIntermediateButton);
  m_buttonLayout->addWidget(m_addIntermediateButton);
  m_buttonLayout->addWidget(m_removeIntermediateButton);
  m_buttonLayout->insertSpacing(3, 50);
  m_buttonLayout->addWidget(m_doneButton);

  m_overallLayout->addWidget(m_edgeLabel);
  m_overallLayout->addWidget(m_intermediatesList);
  m_overallLayout->addLayout(m_buttonLayout);

  this->setLayout(m_overallLayout);

  connect(m_editIntermediateButton, SIGNAL(clicked()), this, SLOT(EditIntermediate()));
  connect(m_addIntermediateButton, SIGNAL(clicked()), this, SLOT(AddIntermediate()));
  connect(m_removeIntermediateButton, SIGNAL(clicked()), this, SLOT(RemoveIntermediate()));
  connect(m_doneButton, SIGNAL(clicked()), this, SLOT(close()));
}

void
EdgeEditDialog::SetCurrentEdge(EdgeModel* _edge){

  m_currentEdge = _edge;
  m_edgeLabel->setText(_edge->Name().c_str());
  ResetIntermediates();
}

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

  for(IIT it = listItems.begin(); it != listItems.end(); it++){
    //Default list item has start cfg as m_cfg, so check against that
    if((*it)->isSelected() && (*it)->m_cfg != m_currentEdge->GetStartCfg()){
      NodeEditDialog n(this, (*it)->m_cfg, m_glScene);
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
