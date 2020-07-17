#include "EdgeEditDialog.h"

#include <sstream>
#include <algorithm>

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>

#include "NodeEditDialog.h"
#include "GLWidget.h"
#include "MainWindow.h"
#include "ModelSelectionWidget.h"
#include "Models/EnvModel.h"
#include "Models/EdgeModel.h"
#include "Models/MapModel.h"
#include "Models/Vizmo.h"

CfgListWidget::
CfgListWidget(QWidget* _parent) : QListWidget(_parent) {

  connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(SelectInMap()));
}

void
CfgListWidget::
SelectInMap() { //Display intermediate cfg selection on map

  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  for(IIT i = m_items.begin(); i != m_items.end(); i++){
    if((*i)->isSelected())
      sel.push_back((*i)->m_cfg);
    else
      sel.erase(remove(sel.begin(), sel.end(), (*i)->m_cfg), sel.end());
  }
}

EdgeEditDialog::
EdgeEditDialog(MainWindow* _mainWindow, EdgeModel* _originalEdge)
    : QDialog(_mainWindow), m_mainWindow(_mainWindow),
    m_originalEdge(_originalEdge), m_glScene(_mainWindow->GetGLWidget()),
    m_tempObjs(), m_nodeEditDialog(NULL) {

    Init("Map", _mainWindow, _originalEdge);
}
//Revision
EdgeEditDialog::
EdgeEditDialog(string _type , MainWindow* _mainWindow, EdgeModel* _originalEdge)
    : QDialog(_mainWindow), m_mainWindow(_mainWindow),
    m_originalEdge(_originalEdge), m_glScene(_mainWindow->GetGLWidget()),
    m_tempObjs(), m_nodeEditDialog(NULL) {
      Init(_type, _mainWindow, _originalEdge);
    }

EdgeEditDialog::~EdgeEditDialog(){}

void
EdgeEditDialog::
Init(string _type, MainWindow* _mainWindow, EdgeModel* _originalEdge)  {
  m_title = _type;
  //make a working copy of the edge to be modified
  m_tempEdge = new EdgeModel(*_originalEdge);
  m_tempEdge->Set(m_originalEdge->GetStartCfg(), m_originalEdge->GetEndCfg());
  m_tempObjs.AddModel(m_tempEdge);

  setWindowTitle("Modify Edge");
  setFixedSize(200, 350);
  setStyleSheet("QListWidget { font:8pt } QPushButton { font:8pt }");

  m_intermediatesList = new CfgListWidget(this);

  QPushButton* editIntermediateButton = new QPushButton("Edit...", this);
  QPushButton* addIntermediateButton = new QPushButton("Add...", this);
  addIntermediateButton->setToolTip("Add an intermediate configuration after the one currently selected in the list.");
  QPushButton* removeIntermediateButton = new QPushButton("Remove", this);
  removeIntermediateButton->setToolTip("Remove the currently selected configuration.");
  QPushButton* doneButton = new QPushButton("Done", this);
  QPushButton* cancelButton = new QPushButton("Cancel", this);

  QLabel* edgeLabel = new QLabel(m_tempEdge->Name().c_str(), this);

  QHBoxLayout* topButtonLayout = new QHBoxLayout();
  topButtonLayout->addWidget(editIntermediateButton);
  topButtonLayout->addWidget(addIntermediateButton);
  topButtonLayout->addWidget(removeIntermediateButton);

  QHBoxLayout* bottomButtonLayout = new QHBoxLayout();
  bottomButtonLayout->addItem(new QSpacerItem(40, 0));
  bottomButtonLayout->addWidget(doneButton);
  bottomButtonLayout->addWidget(cancelButton);

  QVBoxLayout* overallLayout = new QVBoxLayout();
  overallLayout->addWidget(edgeLabel);
  overallLayout->addWidget(m_intermediatesList);
  overallLayout->addLayout(topButtonLayout);
  overallLayout->addItem(new QSpacerItem(200, 40));
  overallLayout->addLayout(bottomButtonLayout);

  this->setLayout(overallLayout);

  ResetIntermediates();
  connect(editIntermediateButton, SIGNAL(clicked()), this, SLOT(EditIntermediate()));
  connect(addIntermediateButton, SIGNAL(clicked()), this, SLOT(AddIntermediate()));
  connect(removeIntermediateButton, SIGNAL(clicked()), this, SLOT(RemoveIntermediate()));
  connect(doneButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  //Set end behavior
  connect(this, SIGNAL(finished(int)), this, SLOT(FinalizeEdgeEdit(int)));
  setAttribute(Qt::WA_DeleteOnClose);
}
void
EdgeEditDialog::
ClearIntermediates() {

  m_intermediatesList->clear();
  m_intermediatesList->GetListItems().clear();
}

void
EdgeEditDialog::
ResetIntermediates() {

  ClearIntermediates();

  vector<CfgModel>& intermediates = m_tempEdge->GetIntermediates();
  if(intermediates.empty()){
    //Assign to start cfg so that new one can be added right after it
    CfgListItem* defaultItem = new CfgListItem(m_intermediatesList, m_tempEdge->GetStartCfg());
    defaultItem->setText("There are no intermediates.\n");
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
}

void
EdgeEditDialog::
EditIntermediate() {

  if(m_nodeEditDialog == NULL) {
    vector<CfgListItem*>& listItems = m_intermediatesList->GetListItems();

    for(IIT it = listItems.begin(); it != listItems.end(); it++){
      if((*it)->isSelected() && (*it)->m_cfg != m_tempEdge->GetStartCfg()){
        if(m_title == "Skeleton")
          m_nodeEditDialog = new NodeEditDialog(m_mainWindow,
              "Intermediate Vertex", (*it)->m_cfg, m_tempEdge);
        else
          m_nodeEditDialog = new NodeEditDialog(m_mainWindow,
              "Intermediate Configuration", (*it)->m_cfg, m_tempEdge);
        m_mainWindow->ShowDialog(m_nodeEditDialog);
        break;
      }
    }
  }
}

void
EdgeEditDialog::
AddIntermediate() {


  vector<CfgListItem*>& listItems = m_intermediatesList->GetListItems();
  int indexAhead = 1;

  for(IIT it = listItems.begin(); it != listItems.end(); it++){
    if((*it)->isSelected()){

      vector<CfgModel>& allInts = m_tempEdge->GetIntermediates();
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
EdgeEditDialog::
RemoveIntermediate() {

  vector<CfgListItem*>& listItems = m_intermediatesList->GetListItems();

  for(IIT it = listItems.begin(); it != listItems.end(); it++){
    if((*it)->isSelected()){

      vector<CfgModel>& allInts = m_tempEdge->GetIntermediates();
      vector<CfgModel>::iterator pos = std::find(allInts.begin(), allInts.end(), *(*it)->m_cfg);

      if(pos != allInts.end()){
        allInts.erase(pos);
        ResetIntermediates();
      }
      return;
    }
  }
}

void
EdgeEditDialog::
FinalizeEdgeEdit(int _accepted) {
  if(_accepted == 1) {
    if(m_tempEdge->IsValid() || m_title == "Skeleton"){
      m_originalEdge->SetIntermediates(m_tempEdge->GetIntermediates());
      if(m_title == "Skeleton")
          GetVizmo().GetEnv()->GetGraphModel()->Refresh();
      }
    else
      //For now, user must start all over again in this case
      QMessageBox::about(this, "", "Invalid edge!");
  }
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
}


