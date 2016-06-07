#include "CustomizePathDialog.h"

#include <iostream>

#include "MainWindow.h"
#include "Models/PathModel.h"
#include "Models/Vizmo.h"

CustomizePathDialog::
CustomizePathDialog(MainWindow* _mainWindow) : QDialog(_mainWindow) {
  RestoreDefault();
  SetUpDialog();
  setAttribute(Qt::WA_DeleteOnClose);
}

void
CustomizePathDialog::
paintEvent(QPaintEvent* _p) {

  QPainter painter(this);
  QLinearGradient grad(QPointF(m_gradientRect->x(), m_gradientRect->y()),
    QPointF(m_gradientRect->x(), m_gradientRect->y() + m_gradientRect->height()));

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
  painter.drawRect(m_gradientRect->x(), m_gradientRect->y(), m_gradientRect->width(),
    m_gradientRect->height());
}

void
CustomizePathDialog::
RestoreDefault() {

  m_colors.clear();
  m_colors.push_back(QColor(0, 255, 255));
  m_colors.push_back(QColor(0, 255, 0));
  m_colors.push_back(QColor(255, 255, 0));
  m_isDefault = true;

  update();
}

void
CustomizePathDialog::
AddColor() {

  QColor color = QColorDialog::getColor(Qt::white, this);
  if(color.isValid()){
    if(m_isDefault)
      m_colors.clear();
    m_colors.push_back(color);
    m_isDefault = false;
    update();
  }
}

void
CustomizePathDialog::
AcceptData() {
  PathModel* path = GetVizmo().GetPath();

  double width = (m_widthLineEdit->text()).toDouble();
  path->SetLineWidth(width);

  size_t disp = (m_modLineEdit->text()).toInt();
  if(disp < path->GetSize() && disp > 0) //if 0, floating point exception!
    path->SetDisplayInterval(disp);

  path->GetGradientVector().clear();
  for(auto& c : m_colors)
    path->GetGradientVector().push_back(Color4(c.redF(), c.greenF(), c.blueF(), 1.0));

  path->Build();
  accept();
}

void
CustomizePathDialog::
SetUpDialog() {
  PathModel* path = GetVizmo().GetPath();

  setFixedSize(200, 540);
  setWindowTitle("Customize Path");

  QGridLayout* layout = new QGridLayout();
  layout->setContentsMargins(20, 0, 20, 10);
  setLayout(layout);

  m_gradientRect = new QWidget(this);
  m_gradientRect->setFixedSize(30, 425);
  layout->addWidget(new QLabel("Path Gradient Fill", this), 1, 1, 1, 3);
  layout->addWidget(m_gradientRect, 2, 1, 7, 1);

  m_colors.clear();
  vector<Color4> tmp = path->GetGradientVector();
  for(auto& c : tmp)
    m_colors.push_back(QColor(c[0] * 255, c[1] * 255, c[2] * 255));
  update();

  QPushButton* addColorButton = new QPushButton(this);
  addColorButton->setText("Add Color...");
  layout->addWidget(addColorButton, 2, 3, 1, 1);

  QPushButton* clearGradButton = new QPushButton(this);
  clearGradButton->setText("Clear");
  clearGradButton->setToolTip("Restores the default gradient. When a new color is added, the default gradient is erased.");
  layout->addWidget(clearGradButton, 3, 3, 1, 1);
  layout->addItem(new QSpacerItem(0, 195), 4, 3, 1, 1);

  m_widthLineEdit = new QLineEdit(this);
  layout->addWidget(new QLabel("Outline width", this), 5, 3, 1, 1);
  layout->addWidget(m_widthLineEdit, 6, 3, 1, 1);

  QString widthString = QString::number(path->GetLineWidth());
  m_widthLineEdit->setText(widthString);

  m_modLineEdit = new QLineEdit(this);
  layout->addWidget(new QLabel("Cfg display\ninterval", this), 7, 3, 1, 1);
  layout->addWidget(m_modLineEdit, 8, 3, 1, 1);
  layout->addItem(new QSpacerItem(200, 12), 9, 1, 1, 3);

  QString dispString = QString::number(path->GetDisplayInterval());
  m_modLineEdit->setText(dispString);

  QDialogButtonBox* okayCancel = new QDialogButtonBox(this);
  okayCancel->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
  layout->addWidget(okayCancel, 10, 1, 1, 3);

  //Make connections
  connect(addColorButton, SIGNAL(pressed()), this, SLOT(AddColor()));
  connect(clearGradButton, SIGNAL(pressed()), this, SLOT(RestoreDefault()));
  connect(okayCancel, SIGNAL(accepted()), this, SLOT(AcceptData()));
  connect(okayCancel, SIGNAL(rejected()), this, SLOT(reject()));
}
