#include "QueryEditDialog.h"

#include <iostream>

#include "GLWidget.h"
#include "MainWindow.h"
#include "ModelSelectionWidget.h"
#include "NodeEditDialog.h"

#include "Models/Vizmo.h"

QueryEditDialog::
QueryEditDialog(MainWindow* _mainWindow, QueryModel* _queryModel) :
    QDialog(_mainWindow), m_queryModel(_queryModel), m_mainWindow(_mainWindow) {
  setFixedSize(200, 310);
  setWindowTitle("Edit Query");
  SetUpDialog();
  setAttribute(Qt::WA_DeleteOnClose);
  ShowQuery();
}

QueryEditDialog::
~QueryEditDialog() {}

void
QueryEditDialog::
SetUpDialog() {
  m_listWidget = new QListWidget(this);
  QPushButton* editButton = new QPushButton("Edit",this);
  QPushButton* addButton = new QPushButton("Add",this);
  QPushButton* deleteButton = new QPushButton("Delete",this);
  QPushButton* upButton = new QPushButton(QChar(0x2227),this);
  QPushButton* downButton = new QPushButton(QChar(0x2228),this);
  QPushButton* leaveButton = new QPushButton("Leave",this);

  QGridLayout* layout = new QGridLayout();
  setLayout(layout);

  QVBoxLayout* upDown = new QVBoxLayout();
  upDown->setSpacing(0);
  upDown->addWidget(upButton);
  upDown->addWidget(downButton);

  layout->addWidget(m_listWidget, 1, 1, 5, 1);
  m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  layout->addWidget(editButton, 1, 2, 1, 1);
  layout->addWidget(addButton, 2, 2, 1, 1);
  layout->addWidget(deleteButton, 3, 2, 1, 1);
  layout->addLayout(upDown, 4, 2, 1, 1);
  layout->addWidget(leaveButton, 5, 2, 1, 1);

  connect(deleteButton, SIGNAL(clicked()), this, SLOT(Delete()));
  connect(addButton, SIGNAL(clicked()), this, SLOT(Add()));
  connect(editButton, SIGNAL(clicked()), this, SLOT(EditQuery()));
  connect(upButton, SIGNAL(clicked()), this, SLOT(SwapUp()));
  connect(downButton, SIGNAL(clicked()), this, SLOT(SwapDown()));
  connect(leaveButton, SIGNAL(clicked()), this, SLOT(accept()));
}

void
QueryEditDialog::
ShowQuery() {
  for(size_t i=0; i<m_queryModel->GetQuerySize(); i++){
    if(i==0)
      m_listWidget->addItem("start");
    else{
      stringstream gNum;
      gNum<<"goal num:"<<i;
      m_listWidget->addItem(gNum.str().c_str());
    }
  }
}

void
QueryEditDialog::
Add() {
  RefreshEnv();
  if(m_listWidget->currentItem()!=NULL){
    int num=m_listWidget->currentRow();
    m_queryModel->AddCfg(num+1);
    m_listWidget->clear();
    ShowQuery();
    m_listWidget->setCurrentItem(m_listWidget->item(num+1));
  }
  size_t numQuery=m_listWidget->row(m_listWidget->currentItem());
  CfgModel& qryCfg = m_queryModel->GetQueryCfg(numQuery);
  NodeEditDialog* ned = new NodeEditDialog(m_mainWindow, qryCfg.Name(), &qryCfg);
  m_mainWindow->ShowDialog(ned);
  RefreshEnv();
}

void
QueryEditDialog::
Delete() {
  if(m_listWidget->currentItem()!=NULL){
    size_t numQuery=m_listWidget->row(m_listWidget->currentItem());
    m_queryModel->DeleteQuery(numQuery);
    m_listWidget->clear();
    ShowQuery();
    RefreshEnv();
  }
}

void
QueryEditDialog::
EditQuery() {
  if(m_listWidget->currentItem()!=NULL){
    size_t numQuery = m_listWidget->row(m_listWidget->currentItem());
    CfgModel& qryCfg = m_queryModel->GetQueryCfg(numQuery);
    NodeEditDialog* ned = new NodeEditDialog(m_mainWindow, qryCfg.Name(), &qryCfg);
    m_mainWindow->ShowDialog(ned);
    //RefreshEnv();
  }
}

void
QueryEditDialog::
SwapUp() {
  if((m_listWidget->currentItem()!=NULL)&&(m_listWidget->row(m_listWidget->currentItem())!=0)){
    size_t numQuery=m_listWidget->row(m_listWidget->currentItem());
    m_queryModel->SwapUp(numQuery);
    RefreshEnv();
    m_listWidget->setCurrentItem(m_listWidget->item(numQuery-1));
  }
}

void
QueryEditDialog::
SwapDown() {
  if((m_listWidget->currentItem()!=NULL)&&(m_listWidget->row(m_listWidget->currentItem())!=m_listWidget->count()-1)){
    size_t numQuery=m_listWidget->row(m_listWidget->currentItem());
    m_queryModel->SwapDown(numQuery);
    RefreshEnv();
    m_listWidget->setCurrentItem(m_listWidget->item(numQuery+1));
  }
}

void
QueryEditDialog::
RefreshEnv() {
  m_queryModel->Build();
  GetVizmo().PlaceRobots();
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
}
