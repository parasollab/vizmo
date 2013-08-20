#include "CustomizePathDialog.h"

#include <iostream>

#include <QPainter>

#include "Models/PathModel.h"
#include "Models/Vizmo.h"

CustomizePathDialog::CustomizePathDialog(QWidget* _parent)
  : QDialog(_parent), m_addColorDialog(NULL), m_brush(NULL), m_gradient(NULL) {

  RestoreDefault();
  SetUpDialog(this);
}

void
CustomizePathDialog::paintEvent(QPaintEvent* _p){

  QPainter painter(this);
  QLinearGradient grad(QPointF(20, 140), QPointF(501, 30));

  if(m_colors.size() == 0){
    RestoreDefault();
    return;
  }

  if(m_colors.size() > 1){
   typedef vector<QColor>::iterator QIT;
    for(QIT qit = m_colors.begin(); qit!=m_colors.end(); qit++)
      grad.setColorAt((double)(qit-m_colors.begin())/(m_colors.size()-1), *qit);
  }
  //handle single-color path
  else
    grad.setColorAt(0, m_colors[0]);

  painter.setBrush(grad);
  painter.drawRect(20, 140, 501, 30);
}

void
CustomizePathDialog::RestoreDefault(){

  m_colors.clear();
  m_colors.push_back(QColor(0, 255, 255));
  m_colors.push_back(QColor(0, 255, 0));
  m_colors.push_back(QColor(255, 255, 0));
  m_isDefault = true;

  update();
}

void
CustomizePathDialog::AddColor(){
  QColor color = m_addColorDialog->getColor();
  if(m_isDefault)
    m_colors.clear();
  m_colors.push_back(color);
  m_isDefault = false;
  update();
}

void
CustomizePathDialog::AcceptData(){

  PathModel* path = GetVizmo().GetPath();

  double width = (m_widthLineEdit->text()).toDouble();
  path->SetLineWidth(width);

  size_t disp = (m_modLineEdit->text()).toInt();
  if(disp < path->GetPathSize() && disp > 0) //if 0, floating point exception!
    path->SetDisplayInterval(disp);

  path->GetGradientVector().clear();
  typedef vector<QColor>::iterator QIT;
  for(QIT qit = m_colors.begin(); qit != m_colors.end(); qit++){
    path->GetGradientVector().push_back(PathModel::Color4(qit->redF(), qit->greenF(), qit->blueF(), 1.0));
  }

  path->BuildModels();
  accept();
}

void
CustomizePathDialog::SetUpDialog(QDialog* _dialog){

  _dialog->resize(550, 312);
  _dialog->setWindowTitle("Customize Path");

  m_okayCancel = new QDialogButtonBox(_dialog);
  m_okayCancel->setGeometry(QRect(350, 270, 176, 27));
  m_okayCancel->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

  m_gradientLabel = new QLabel(_dialog);
  m_gradientLabel->setGeometry(QRect(20, 120, 121, 17));
  m_gradientLabel->setText("Path Gradient Fill");

  m_widthLineEdit = new QLineEdit(_dialog);
  m_widthLineEdit->setGeometry(20, 210, 91, 27);

  m_widthLabel = new QLabel(_dialog);
  m_widthLabel->setGeometry(20, 190, 101, 20);
  m_widthLabel->setText("Outline width");

  m_modLineEdit = new QLineEdit(_dialog);
  m_modLineEdit->setGeometry(20, 270, 91, 27);

  m_modLabel = new QLabel(_dialog);
  m_modLabel->setGeometry(20, 250, 161, 17);
  m_modLabel->setText("Display cfgs at interval");

  m_addColorButton = new QPushButton(_dialog);
  m_addColorButton->setGeometry(QRect(360, 110, 101, 27));
  m_addColorButton->setText("Add Color...");
  connect(m_addColorButton, SIGNAL(pressed()), this, SLOT(AddColor()));

  m_clearGradButton = new QPushButton(_dialog);
  m_clearGradButton->setGeometry(470, 110, 51, 27);
  m_clearGradButton->setText("Clear");
  m_clearGradButton->setToolTip("Restores the default gradient. When a new color is added, the default gradient is erased.");
  connect(m_clearGradButton, SIGNAL(pressed()), this, SLOT(RestoreDefault()));

  m_instructions1 = new QLabel(_dialog);
  m_instructions1->setGeometry(20, 20, 521, 17);
  m_instructions1->setText("Customize the path display with <b>Add Color</b>, or select <b>Clear</b> to start over.");

  m_instructions2 = new QLabel(_dialog);
  m_instructions2->setGeometry(20, 40, 521, 17);
  m_instructions2->setText("The default gradient is cyan-green-yellow.");

  m_line = new QFrame(_dialog);
  m_line->setGeometry(20, 70, 501, 16);
  m_line->setFrameShape(QFrame::HLine);
  m_line->setFrameShadow(QFrame::Sunken);

  connect(m_okayCancel, SIGNAL(accepted()), _dialog, SLOT(AcceptData()));
  connect(m_okayCancel, SIGNAL(rejected()), _dialog, SLOT(reject()));

  QMetaObject::connectSlotsByName(_dialog);
}


