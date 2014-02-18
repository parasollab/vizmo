#include "ObstaclePosDialog.h"

#include "GLWidget.h"
#include "MainWindow.h"
#include "Transformation.h"
#include "Models/PolyhedronModel.h"
#include "Models/Vizmo.h"
#include "Models/EnvModel.h"
#include "Utilities/IO.h"

ObstaclePosDialog::ObstaclePosDialog(const vector<MultiBodyModel*>& _multiBody, MainWindow* _mainWindow,
    QWidget* _parent) : QDialog(_parent), m_multiBody(_multiBody), m_mainWindow(_mainWindow) {

  m_oneObst = m_multiBody.size() == 1;

  setWindowTitle("Obstacle Position");

  SetUpLayout();
  SetSlidersInit();

  //connect all sliders/posLines
  size_t num = m_oneObst ? 6 : 3;
  for(size_t i = 0; i < num; i++) {
    connect(m_sliders[i], SIGNAL(valueChanged(int)), this, SLOT(DisplaySlidersValues(int)));
    connect(m_posLines[i], SIGNAL(editingFinished()), this, SLOT(ChangeSlidersValues()));
  }

  QDialog::show();
}

void
ObstaclePosDialog::SetUpLayout(){
  QGridLayout* layout = new QGridLayout;
  setLayout(layout);

  QPushButton* loadButton = new QPushButton("OK", this);
  connect(loadButton, SIGNAL(clicked()), this, SLOT(accept()));
  layout->addWidget(loadButton, 8, 0, 1, 3);

  layout->addWidget(new QLabel("<b>Coordinates<b>", this), 0, 0, 1, 3);
  layout->addWidget(new QLabel("x", this), 1, 0);
  layout->addWidget(new QLabel("y", this), 2, 0);
  layout->addWidget(new QLabel("z", this), 3, 0);
  if(m_oneObst) {
    layout->addWidget(new QLabel("<b>Rotation<b>", this), 4, 0, 1, 3);
    layout->addWidget(new QLabel(QChar(0x03B1), this), 5, 0);
    layout->addWidget(new QLabel(QChar(0x03B2), this), 6, 0);
    layout->addWidget(new QLabel(QChar(0x03B3), this), 7, 0);
  }
  size_t num = m_oneObst ? 6 : 3;
  for(size_t i = 0; i < num; i++) {
    m_sliders[i] = new QSlider(Qt::Horizontal, this);
    m_sliders[i]->setFixedWidth(200);
    layout->addWidget(m_sliders[i], i < 3 ? i+1 : i+2, 1);

    m_posLines[i] = new QLineEdit("0", this);
    layout->addWidget(m_posLines[i], i < 3 ? i+1 : i+2, 2);
  }

}

void
ObstaclePosDialog::SetSlidersInit(){
  //set validators for posLines
  vector<pair<double, double> > ranges = GetVizmo().GetEnv()->GetBoundary()->GetRanges();
  m_posLines[0]->setValidator(new QDoubleValidator(ranges[0].first, ranges[0].second, 2, this));
  m_posLines[1]->setValidator(new QDoubleValidator(ranges[1].first, ranges[1].second, 2, this));
  m_posLines[2]->setValidator(new QDoubleValidator(ranges[2].first, ranges[2].second, 2, this));
  if(m_oneObst) {
    m_posLines[3]->setValidator(new QDoubleValidator(-180, 180, 2, this));
    m_posLines[4]->setValidator(new QDoubleValidator(-180, 180, 2, this));
    m_posLines[5]->setValidator(new QDoubleValidator(-180, 180, 2, this));
  }

  //set sliders min/max and initial values
  m_valueEdited=false;
  for(size_t i = 0; i < 3; ++i) {
    m_sliders[i]->setMinimum(ranges[i].first*100);
    m_sliders[i]->setMaximum(ranges[i].second*100);
  }
  if(m_oneObst) {
    for(size_t i = 3; i < 6; ++i) {
      m_sliders[i]->setMinimum(-180*100);
      m_sliders[i]->setMaximum(180*100);
    }

    const Transformation& t = (*m_multiBody[0]->Begin())->GetTransform();
    const Vector3d& v = t.translation();
    EulerAngle e;
    convertFromMatrix(e, t.rotation().matrix());

    for(size_t i = 0; i < 3; ++i) {
      m_sliders[i]->setValue(v[i]*100);
      m_posLines[i]->setText(QString::number(v[i]));
    }
    double a = radToDeg(e.alpha()), b = radToDeg(e.beta()), g = radToDeg(e.gamma());
    m_sliders[3]->setValue(a*100);
    m_posLines[3]->setText(QString::number(a));
    m_sliders[4]->setValue(b*100);
    m_posLines[4]->setText(QString::number(b));
    m_sliders[5]->setValue(g*100);
    m_posLines[5]->setText(QString::number(g));
  }
  else {
    //compute center
    typedef vector<MultiBodyModel*>::iterator MIT;
    for(MIT mit = m_multiBody.begin(); mit != m_multiBody.end(); mit++)
      m_center += (*(*mit)->Begin())->GetTransform().translation();
    m_center /= m_multiBody.size();

    //set slider values
    for(size_t i=0; i<3; i++) {
      m_sliders[i]->setValue(m_center[i]*100);
      m_posLines[i]->setText(QString::number(m_center[i]));
    }
  }
}

void
ObstaclePosDialog::DisplaySlidersValues(int _i) {
  if(!m_valueEdited) {
    size_t num = m_oneObst ? 6 : 3;
    for(size_t i = 0; i < num; ++i)
      if(m_sliders[i]->value() == _i)
        m_posLines[i]->setText(QString::number(_i / 100.0));
    RefreshPosition();
  }
}

void
ObstaclePosDialog::ChangeSlidersValues(){
  m_valueEdited=true;
  size_t num = m_oneObst ? 6 : 3;
  for(size_t i = 0; i < num; i++) {
    double val = m_posLines[i]->text().toDouble();
    if(val != m_sliders[i]->value() / 100.)
      m_sliders[i]->setValue(val*100);
  }
  RefreshPosition();
  m_valueEdited=false;
}

void
ObstaclePosDialog::RefreshPosition() {
  if(m_oneObst) {
    double x = m_posLines[0]->text().toDouble();
    double y = m_posLines[1]->text().toDouble();
    double z = m_posLines[2]->text().toDouble();
    double a = degToRad(m_posLines[3]->text().toDouble());
    double b = degToRad(m_posLines[4]->text().toDouble());
    double g = degToRad(m_posLines[5]->text().toDouble());
    Transformation t(Vector3d(x, y, z), Orientation(EulerAngle(a, b, g)));
    (*m_multiBody[0]->Begin())->SetTransform(t);
  }
  else {
    //compute difference from center
    Vector3d diff;
    for(size_t i = 0; i < 3; ++i) {
      double val = m_posLines[i]->text().toDouble();
      diff[i] = val - m_center[i];
      m_center[i] = val;
    }

    //update transforms
    typedef vector<MultiBodyModel*>::iterator MIT;
    for(MIT mit = m_multiBody.begin(); mit != m_multiBody.end(); ++mit) {
      Transformation t = (*(*mit)->Begin())->GetTransform();
      t.translation() += diff;
      (*(*mit)->Begin())->SetTransform(t);
    }
  }
  m_mainWindow->GetGLScene()->updateGL();
}

