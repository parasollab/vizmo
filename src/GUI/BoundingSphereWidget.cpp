#include "BoundingSphereWidget.h"

#include "Models/BoundingSphereModel.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

BoundingSphereWidget::BoundingSphereWidget(QWidget* _parent) : QWidget(_parent) {
  //construct objects
  QLabel* labelX = new QLabel("<b>X<b>", this);
  QLabel* labelY = new QLabel("<b>Y<b>", this);
  QLabel* labelZ = new QLabel("<b>Z<b>", this);
  QLabel* labelR = new QLabel("<b>Radius<b>", this);

  m_lineX = new QLineEdit("0", this);
  m_lineY = new QLineEdit("0", this);
  m_lineZ = new QLineEdit("0", this);
  m_lineR = new QLineEdit("1", this);

  m_lineX->setValidator(new QDoubleValidator(this));
  m_lineY->setValidator(new QDoubleValidator(this));
  m_lineZ->setValidator(new QDoubleValidator(this));
  m_lineR->setValidator(new QDoubleValidator(this));

  //set initial values
  ShowCurrentValues();

  //set up layout
  QGridLayout* layout = new QGridLayout;
  setLayout(layout);

  layout->addWidget(labelX, 0, 0, Qt::AlignCenter);
  layout->addWidget(labelY, 0, 1, Qt::AlignCenter);
  layout->addWidget(labelZ, 0, 2, Qt::AlignCenter);
  layout->addWidget(labelR, 0, 3, Qt::AlignCenter);
  layout->addWidget(m_lineX, 1, 0);
  layout->addWidget(m_lineY, 1, 1);
  layout->addWidget(m_lineZ, 1, 2);
  layout->addWidget(m_lineR, 1, 3);
}

void
BoundingSphereWidget::SetBoundary() {
  EnvModel* env = GetVizmo().GetEnv();
  delete env->GetBoundary();

  Point3d center;
  double radius = 0;
  center[0] = m_lineX->text().toDouble();
  center[1] = m_lineY->text().toDouble();
  center[2] = m_lineZ->text().toDouble();
  radius = m_lineR->text().toDouble();

  env->SetBoundary(new BoundingSphereModel(center, radius));
}

void
BoundingSphereWidget::ShowCurrentValues() {
  const string& name = GetVizmo().GetEnv()->GetBoundary()->Name();
  if(name == "Bounding Sphere") {
    BoundingSphereModel* bs = (BoundingSphereModel*)GetVizmo().GetEnv()->GetBoundary();
    const Point3d& c = bs->GetCenter();
    double r = bs->GetRadius();

    m_lineX->setText(QString::number(c[0]));
    m_lineY->setText(QString::number(c[1]));
    m_lineZ->setText(QString::number(c[2]));
    m_lineR->setText(QString::number(r));
  }
}

