#include "BoundingBoxWidget.h"

#include "Models/BoundingBoxModel.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

BoundingBoxWidget::
BoundingBoxWidget(QWidget* _parent) : QWidget(_parent) {
  //construct objects
  setStyleSheet("QLineEdit { font: 9pt }");
  QLabel* labelX = new QLabel("<b>X<b>", this);
  QLabel* labelY = new QLabel("<b>Y<b>", this);
  QLabel* labelZ = new QLabel("<b>Z<b>", this);

  m_lineXMin = new QLineEdit("-1", this);
  m_lineXMax = new QLineEdit("1", this);
  m_lineYMin = new QLineEdit("-1", this);
  m_lineYMax = new QLineEdit("1", this);
  m_lineZMin = new QLineEdit("-1", this);
  m_lineZMax = new QLineEdit("1", this);

  m_is2D = false;
  m_checkIs2D = new QCheckBox("2D", this);
  connect(m_checkIs2D, SIGNAL(clicked()), this, SLOT(Toggle2D()));

  m_lineXMin->setValidator(new QDoubleValidator(this));
  m_lineXMax->setValidator(new QDoubleValidator(this));
  m_lineYMin->setValidator(new QDoubleValidator(this));
  m_lineYMax->setValidator(new QDoubleValidator(this));
  m_lineZMin->setValidator(new QDoubleValidator(this));
  m_lineZMax->setValidator(new QDoubleValidator(this));

  //set initial values
  ShowCurrentValues();

  //setup layout
  QGridLayout* layout = new QGridLayout;
  setLayout(layout);

  layout->addWidget(m_checkIs2D, 0, 0, 1, 6, Qt::AlignCenter);
  layout->addWidget(labelX, 1, 0, 1, 1);
  layout->addWidget(labelY, 2, 0, 1, 1);
  layout->addWidget(labelZ, 3, 0, 1, 1);
  layout->addWidget(m_lineXMin, 1, 1);
  layout->addWidget(m_lineXMax, 1, 2);
  layout->addWidget(m_lineYMin, 2, 1);
  layout->addWidget(m_lineYMax, 2, 2);
  layout->addWidget(m_lineZMin, 3, 1);
  layout->addWidget(m_lineZMax, 3, 2);
}


void
BoundingBoxWidget::
Toggle2D(){
  m_is2D = m_checkIs2D->isChecked();
  m_lineZMin->setEnabled(!m_is2D);
  m_lineZMax->setEnabled(!m_is2D);
}


void
BoundingBoxWidget::
SetBoundary() {
  double maxd = numeric_limits<double>::max();
  vector<pair<double, double> > ranges(3, make_pair(-maxd, maxd));
  ranges[0].first = m_lineXMin->text().toDouble();
  ranges[0].second = m_lineXMax->text().toDouble();
  ranges[1].first = m_lineYMin->text().toDouble();
  ranges[1].second = m_lineYMax->text().toDouble();
  if(!m_is2D) {
      ranges[2].first = m_lineZMin->text().toDouble();
      ranges[2].second = m_lineZMax->text().toDouble();
  }

  GetVizmo().GetEnv()->SetBoundary(shared_ptr<BoundingBoxModel>(
        new BoundingBoxModel(ranges[0], ranges[1], ranges[2])));
}


void
BoundingBoxWidget::
ShowCurrentValues() {
  const string& name = GetVizmo().GetEnv()->GetBoundary()->Name();
  if(name == "Bounding Box") {
    shared_ptr<BoundingBoxModel> bbx = static_pointer_cast<BoundingBoxModel>(
        GetVizmo().GetEnv()->GetBoundary());
    vector<pair<double, double> > ranges = bbx->GetRanges();

    m_lineXMin->setText(QString::number(ranges[0].first));
    m_lineXMax->setText(QString::number(ranges[0].second));
    m_lineYMin->setText(QString::number(ranges[1].first));
    m_lineYMax->setText(QString::number(ranges[1].second));
    if(ranges.size() > 2) {
      m_lineZMin->setText(QString::number(ranges[2].first));
      m_lineZMax->setText(QString::number(ranges[2].second));
    }
  }
}
