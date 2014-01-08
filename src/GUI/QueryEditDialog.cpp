#include "QueryEditDialog.h"

#include <iostream>
#include <QDialog>
#include <QPushButton>
#include <QListWidget>

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
  m_editButton = new QPushButton("Edit",this);
  m_addButton = new QPushButton("Add",this);
  m_deleteButton = new QPushButton("Delete",this);
  m_upButton = new QPushButton(QChar(0x2227),this);
  m_downButton = new QPushButton(QChar(0x2228),this);
  m_leaveButton = new QPushButton("Leave",this);

  connect(m_leaveButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(Delete()));
  connect(m_addButton, SIGNAL(clicked()), this, SLOT(Add()));
  connect(m_editButton, SIGNAL(clicked()), this, SLOT(EditQuery()));
  connect(m_upButton, SIGNAL(clicked()), this, SLOT(SwapUp()));
  connect(m_downButton, SIGNAL(clicked()), this, SLOT(SwapDown()));

  m_queryModel=_queryModel;
  m_mainWindow= _mainWindow;

  ShowQuery();
  SetUpLayout();
}

QueryEditDialog::~QueryEditDialog(){}

void
QueryEditDialog::SetUpLayout(){
  m_listWidget->setGeometry(QRect(10, 10, 131, 231));
  m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  m_editButton->setGeometry(QRect(160, 10, 61, 31));
  m_addButton->setGeometry(QRect(160, 50, 61, 31));
  m_deleteButton->setGeometry(QRect(160, 90, 61, 31));
  m_upButton->setGeometry(QRect(160, 130, 61, 31));
  m_downButton->setGeometry(QRect(160, 160, 61, 31));
  m_leaveButton->setGeometry(QRect(160, 210, 61, 31));
  QDialog::show();
}

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
