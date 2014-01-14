#include "BoundingBoxWidget.h"

#include "Models/BoundingBoxModel.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

BoundingBoxWidget::BoundingBoxWidget(QWidget* _parent) : QWidget(_parent) {
  //construct objects
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
  connect(m_checkIs2D, SIGNAL(clicked()), this, SLOT(ChangeBoxTo2D()));

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
  layout->addWidget(labelX, 1, 0, 1, 2, Qt::AlignCenter);
  layout->addWidget(labelY, 1, 2, 1, 2, Qt::AlignCenter);
  layout->addWidget(labelZ, 1, 4, 1, 2, Qt::AlignCenter);
  layout->addWidget(m_lineXMin, 2, 0);
  layout->addWidget(m_lineXMax, 2, 1);
  layout->addWidget(m_lineYMin, 2, 2);
  layout->addWidget(m_lineYMax, 2, 3);
  layout->addWidget(m_lineZMin, 2, 4);
  layout->addWidget(m_lineZMax, 2, 5);
}

void
BoundingBoxWidget::ChangeBoxTo2D(){
  m_is2D = m_checkIs2D->isChecked();
  m_lineZMin->setEnabled(!m_is2D);
  m_lineZMax->setEnabled(!m_is2D);
}

void
BoundingBoxWidget::SetBoundary() {
  EnvModel* env = GetVizmo().GetEnv();
  string line="";
  string type="";
  string boxCoord[6]={"0","0","0","0","0","0"};
  if(m_lineXMin->text().toStdString() !="")
    boxCoord[0]=m_lineXMin->text().toStdString();
  if(m_lineXMax->text().toStdString() !="")
    boxCoord[1]=m_lineXMax->text().toStdString();
  if(m_lineYMin->text().toStdString() !="")
    boxCoord[2]=m_lineYMin->text().toStdString();
  if(m_lineYMax->text().toStdString() !="")
    boxCoord[3]=m_lineYMax->text().toStdString();
  if(m_lineZMin->text().toStdString() !="")
    boxCoord[4]=m_lineZMin->text().toStdString();
  if(m_lineZMax->text().toStdString() !="")
    boxCoord[5]=m_lineZMax->text().toStdString();
  int size = 2+1*(!m_is2D);
  for(int i=0;i<size;i++)
    line += (boxCoord[2*i]+":"+boxCoord[2*i+1]+" ");
  type="BOX";
  istringstream coord(line);
  env->ChangeBoundary(type, coord);
}

void
BoundingBoxWidget::ShowCurrentValues() {
  string type = GetVizmo().GetEnv()->GetBoundaryType();
  if(type=="BOX") {
    BoundingBoxModel* bbx = (BoundingBoxModel*)GetVizmo().GetEnv()->GetBoundary();
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

