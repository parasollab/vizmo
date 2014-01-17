#include "QueryEditDialog.h"

#include <iostream>

#include "MainWindow.h"
#include "GLWidget.h"
#include "ModelSelectionWidget.h"
#include "NodeEditDialog.h"
#include "Models/Vizmo.h"

QueryEditDialog::QueryEditDialog(QueryModel* _queryModel, MainWindow* _mainWindow,
    QWidget* _parent) : QDialog(_parent) {
  resize(235, 246);
  setWindowTitle("Edit Query");

  m_listWidget = new QListWidget(this);
  QPushButton* editButton = new QPushButton("Edit",this);
  QPushButton* addButton = new QPushButton("Add",this);
  QPushButton* deleteButton = new QPushButton("Delete",this);
  QPushButton* upButton = new QPushButton(QChar(0x2227),this);
  QPushButton* downButton = new QPushButton(QChar(0x2228),this);
  QPushButton* leaveButton = new QPushButton("Leave",this);

  connect(deleteButton, SIGNAL(clicked()), this, SLOT(Delete()));
  connect(addButton, SIGNAL(clicked()), this, SLOT(Add()));
  connect(editButton, SIGNAL(clicked()), this, SLOT(EditQuery()));
  connect(upButton, SIGNAL(clicked()), this, SLOT(SwapUp()));
  connect(downButton, SIGNAL(clicked()), this, SLOT(SwapDown()));
  connect(leaveButton, SIGNAL(clicked()), this, SLOT(accept()));

  m_queryModel=_queryModel;
  m_mainWindow= _mainWindow;

  ShowQuery();

  m_listWidget->setGeometry(QRect(10, 10, 131, 231));
  m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  editButton->setGeometry(QRect(160, 10, 61, 31));
  addButton->setGeometry(QRect(160, 50, 61, 31));
  deleteButton->setGeometry(QRect(160, 90, 61, 31));
  upButton->setGeometry(QRect(160, 130, 61, 31));
  downButton->setGeometry(QRect(160, 160, 61, 31));
  leaveButton->setGeometry(QRect(160, 210, 61, 31));
}

QueryEditDialog::~QueryEditDialog(){}

void
QueryEditDialog::ShowQuery(){
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
QueryEditDialog::Add(){
  RefreshEnv();
  if(m_listWidget->currentItem()!=NULL){
    int num=m_listWidget->currentRow();
    m_queryModel->AddCfg(num+1);
    m_listWidget->clear();
    ShowQuery();
    m_listWidget->setCurrentItem(m_listWidget->item(num+1));
  }
  size_t numQuery=m_listWidget->row(m_listWidget->currentItem());
  NodeEditDialog q(this, &m_queryModel->GetQueryCfg(numQuery), m_mainWindow->GetGLScene());
  q.exec();
  RefreshEnv();
}

void
QueryEditDialog::Delete(){
  if(m_listWidget->currentItem()!=NULL){
    size_t numQuery=m_listWidget->row(m_listWidget->currentItem());
    m_queryModel->DeleteQuery(numQuery);
    m_listWidget->clear();
    ShowQuery();
    RefreshEnv();
  }
}

void
QueryEditDialog::EditQuery(){
  if(m_listWidget->currentItem()!=NULL){
    size_t numQuery=m_listWidget->row(m_listWidget->currentItem());
    NodeEditDialog q(this, &m_queryModel->GetQueryCfg(numQuery), m_mainWindow->GetGLScene());
    q.exec();
    RefreshEnv();
  }
}

void
QueryEditDialog::SwapUp(){
  if((m_listWidget->currentItem()!=NULL)&&(m_listWidget->row(m_listWidget->currentItem())!=0)){
    size_t numQuery=m_listWidget->row(m_listWidget->currentItem());
    m_queryModel->SwapUp(numQuery);
    RefreshEnv();
    m_listWidget->setCurrentItem(m_listWidget->item(numQuery-1));
  }
}

void
QueryEditDialog::SwapDown(){
  if((m_listWidget->currentItem()!=NULL)&&(m_listWidget->row(m_listWidget->currentItem())!=m_listWidget->count()-1)){
    size_t numQuery=m_listWidget->row(m_listWidget->currentItem());
    m_queryModel->SwapDown(numQuery);
    RefreshEnv();
    m_listWidget->setCurrentItem(m_listWidget->item(numQuery+1));
  }
}

void
QueryEditDialog::RefreshEnv(){
  m_queryModel->BuildModels();
  GetVizmo().PlaceRobot();
  m_mainWindow->GetGLScene()->updateGL();
  m_mainWindow->GetModelSelectionWidget()->ResetLists();
}
