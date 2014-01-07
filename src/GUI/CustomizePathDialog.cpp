#include "CustomizePathDialog.h"

#include <iostream>

#include "Models/PathModel.h"
#include "Models/Vizmo.h"

CustomizePathDialog::CustomizePathDialog(QWidget* _parent)
  : QDialog(_parent){

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

  QColor color = QColorDialog::getColor(Qt::white, this);
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
  if(disp < path->GetSize() && disp > 0) //if 0, floating point exception!
    path->SetDisplayInterval(disp);

  path->GetGradientVector().clear();
  typedef vector<QColor>::iterator QIT;
  for(QIT qit = m_colors.begin(); qit != m_colors.end(); qit++){
    path->GetGradientVector().push_back(Color4(qit->redF(), qit->greenF(), qit->blueF(), 1.0));
  }

  path->BuildModels();
  accept();
}

void
CustomizePathDialog::SetUpDialog(QDialog* _dialog){

  _dialog->resize(550, 312);
  _dialog->setWindowTitle("Customize Path");

  QDialogButtonBox* okayCancel = new QDialogButtonBox(_dialog);
  okayCancel->setGeometry(QRect(350, 270, 176, 27));
  okayCancel->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

  QLabel* gradientLabel = new QLabel(_dialog);
  gradientLabel->setGeometry(QRect(20, 120, 121, 17));
  gradientLabel->setText("Path Gradient Fill");

  m_widthLineEdit = new QLineEdit(_dialog);
  m_widthLineEdit->setGeometry(20, 210, 91, 27);

  QLabel* widthLabel = new QLabel(_dialog);
  widthLabel->setGeometry(20, 190, 101, 20);
  widthLabel->setText("Outline width");

  m_modLineEdit = new QLineEdit(_dialog);
  m_modLineEdit->setGeometry(20, 270, 91, 27);

  QLabel* modLabel = new QLabel(_dialog);
  modLabel->setGeometry(20, 250, 161, 17);
  modLabel->setText("Display cfgs at interval");

  QPushButton* addColorButton = new QPushButton(_dialog);
  addColorButton->setGeometry(QRect(360, 110, 101, 27));
  addColorButton->setText("Add Color...");
  connect(addColorButton, SIGNAL(pressed()), this, SLOT(AddColor()));

  QPushButton* clearGradButton = new QPushButton(_dialog);
  clearGradButton->setGeometry(470, 110, 51, 27);
  clearGradButton->setText("Clear");
  clearGradButton->setToolTip("Restores the default gradient. When a new color is added, the default gradient is erased.");
  connect(clearGradButton, SIGNAL(pressed()), this, SLOT(RestoreDefault()));

  QLabel* instructions1 = new QLabel(_dialog);
  instructions1->setGeometry(20, 20, 521, 17);
  instructions1->setText("Customize the path display with <b>Add Color</b>, or select <b>Clear</b> to start over.");

  QLabel* instructions2 = new QLabel(_dialog);
  instructions2->setGeometry(20, 40, 521, 17);
  instructions2->setText("The default gradient is cyan-green-yellow.");

  QFrame* spacerLine = new QFrame(_dialog);
  spacerLine->setGeometry(20, 70, 501, 16);
  spacerLine->setFrameShape(QFrame::HLine);
  spacerLine->setFrameShadow(QFrame::Sunken);

  connect(okayCancel, SIGNAL(accepted()), _dialog, SLOT(AcceptData()));
  connect(okayCancel, SIGNAL(rejected()), _dialog, SLOT(reject()));

  QMetaObject::connectSlotsByName(_dialog);
}
