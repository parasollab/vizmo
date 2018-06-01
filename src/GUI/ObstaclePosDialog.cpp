#include "ObstaclePosDialog.h"

#include "GLWidget.h"
#include "MainWindow.h"
#include "Transformation.h"
#include "Models/BoundaryModel.h"
#include "Models/EnvModel.h"
#include "Models/PolyhedronModel.h"
#include "Models/StaticMultiBodyModel.h"
#include "Models/Vizmo.h"
#include "Utilities/IO.h"

ObstaclePosDialog::
ObstaclePosDialog(MainWindow* _mainWindow,
    const vector<StaticMultiBodyModel*>& _multiBody) : QDialog(_mainWindow),
    m_multiBody(_multiBody) {

  m_oneObst = m_multiBody.size() == 1;

  setWindowTitle("Obstacle Position");
  setFixedSize(200, 400);

  SetUpLayout();
  SetSlidersInit();

  //connect all sliders/posLines
  size_t num = m_oneObst ? 6 : 3;
  for(size_t i = 0; i < num; i++) {
    connect(m_sliders[i], SIGNAL(valueChanged(int)), this, SLOT(DisplaySlidersValues(int)));
    connect(m_posLines[i], SIGNAL(editingFinished()), this, SLOT(ChangeSlidersValues()));
  }

  connect(&m_transformTool, SIGNAL(TranslationChanged(const Vector3d&)),
      this, SLOT(ChangeTranslation(const Vector3d&)));
  connect(&m_transformTool, SIGNAL(RotationChanged(const Quaternion&)),
      this, SLOT(ChangeRotation(const Quaternion&)));
  connect(this, SIGNAL(TranslationChanged(const Vector3d&)),
      &m_transformTool, SLOT(ChangeTranslation(const Vector3d&)));
  connect(this, SIGNAL(RotationChanged(const Quaternion&)),
      &m_transformTool, SLOT(ChangeRotation(const Quaternion&)));

  //set delete on close
  setAttribute(Qt::WA_DeleteOnClose);

  //show dialog
  QDialog::show();
}

ObstaclePosDialog::
~ObstaclePosDialog() {
  GetMainWindow()->GetGLWidget()->SetTransformTool(NULL);
}

void
ObstaclePosDialog::
SetUpLayout() {
  QGridLayout* layout = new QGridLayout;
  setLayout(layout);
  setStyleSheet("QLabel { font:9pt } QLineEdit { font:8pt }");

  layout->setAlignment(Qt::AlignTop);

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
  for(size_t i = 0; i < num; ++i) {
    m_sliders[i] = new QSlider(Qt::Horizontal, this);
    m_sliders[i]->setFixedWidth(100);
    layout->addWidget(m_sliders[i], i < 3 ? i+1 : i+2, 1);

    m_posLines[i] = new QLineEdit("0", this);
    layout->addWidget(m_posLines[i], i < 3 ? i+1 : i+2, 2);
  }

}

void
ObstaclePosDialog::
SetSlidersInit() {
  //set validators for posLines
  vector<pair<double, double> > ranges = GetVizmo().GetEnv()->GetBoundary()->GetRanges();

  //set sliders min/max and initial values
  for(size_t i = 0; i < 3; ++i) {
    if(ranges[i].second == numeric_limits<double>::max()) {
      double rad = GetVizmo().GetEnv()->GetRadius();
      m_posLines[i]->setValidator(new QDoubleValidator(-rad, rad, 2, this));
      m_sliders[i]->setMinimum(-rad*100);
      m_sliders[i]->setMaximum(rad*100);
    }
    else {
      m_posLines[i]->setValidator(
          new QDoubleValidator(ranges[i].first, ranges[i].second, 2, this));
      m_sliders[i]->setMinimum(ranges[i].first*100);
      m_sliders[i]->setMaximum(ranges[i].second*100);
    }
  }
  if(m_oneObst) {
    for(size_t i = 3; i < 6; ++i) {
      m_posLines[i]->setValidator(new QDoubleValidator(-180, 180, 2, this));
      m_sliders[i]->setMinimum(-180*100);
      m_sliders[i]->setMaximum(180*100);
    }

    const Transformation& t = (*m_multiBody[0]->begin())->GetTransform();
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

    m_transformTool.SetTransform(t);
    GetMainWindow()->GetGLWidget()->SetTransformTool(&m_transformTool);
  }
  else {
    //compute center
    for(const auto& mb : m_multiBody)
      m_center += (*mb->begin())->GetTransform().translation();
    m_center /= m_multiBody.size();

    //set slider values
    for(size_t i = 0; i < 3; ++i) {
      m_sliders[i]->setValue(m_center[i]*100);
      m_posLines[i]->setText(QString::number(m_center[i]));
    }

    Transformation t(m_center, Orientation(EulerAngle()));
    m_transformTool.SetRotationsOn(false);
    m_transformTool.SetTransform(t);
    GetMainWindow()->GetGLWidget()->SetTransformTool(&m_transformTool);
  }
}

void
ObstaclePosDialog::
DisplaySlidersValues(int _i) {
  //static cast sender() to slider*
  //set position lines from slider

  QSlider* slider = static_cast<QSlider*>(sender());
  auto itr = std::find(begin(m_sliders), end(m_sliders), slider);
  size_t indx = distance(begin(m_sliders), itr);
  m_posLines[indx]->blockSignals(true);
  m_posLines[indx]->setText(QString::number(_i / 100.));
  m_posLines[indx]->blockSignals(false);
  RefreshPosition(true);
}

void
ObstaclePosDialog::
ChangeSlidersValues() {
  QLineEdit* posLine = static_cast<QLineEdit*>(sender());
  auto itr = std::find(begin(m_posLines), end(m_posLines), posLine);
  size_t indx = distance(begin(m_posLines), itr);
  double val = m_posLines[indx]->text().toDouble();
  m_sliders[indx]->blockSignals(true);
  m_sliders[indx]->setValue(val*100);
  m_sliders[indx]->blockSignals(false);
  RefreshPosition(true);
}

void
ObstaclePosDialog::
ChangeTranslation(const Vector3d& _t) {
  for(size_t i = 0; i < 3; ++i) {
    m_posLines[i]->blockSignals(true);
    m_posLines[i]->setText(QString::number(_t[i]));
    m_posLines[i]->blockSignals(false);
    m_sliders[i]->blockSignals(true);
    m_sliders[i]->setValue(_t[i]*100);
    m_sliders[i]->blockSignals(false);
  }
  RefreshPosition(false);
}

void
ObstaclePosDialog::
ChangeRotation(const Quaternion& _r) {
  EulerAngle e;
  convertFromQuaternion(e, _r);

  for(size_t i = 3; i < 6; ++i) {
    m_posLines[i]->blockSignals(true);
    m_sliders[i]->blockSignals(true);
  }

  double a = radToDeg(e.alpha()), b = radToDeg(e.beta()), g = radToDeg(e.gamma());
  m_sliders[3]->setValue(a*100);
  m_posLines[3]->setText(QString::number(a));
  m_sliders[4]->setValue(b*100);
  m_posLines[4]->setText(QString::number(b));
  m_sliders[5]->setValue(g*100);
  m_posLines[5]->setText(QString::number(g));

  for(size_t i = 3; i < 6; ++i) {
    m_posLines[i]->blockSignals(false);
    m_sliders[i]->blockSignals(false);
  }

  RefreshPosition(false);
}

void
ObstaclePosDialog::
RefreshPosition(bool _emit) {
  if(m_oneObst) {
    double x = m_posLines[0]->text().toDouble();
    double y = m_posLines[1]->text().toDouble();
    double z = m_posLines[2]->text().toDouble();
    double a = degToRad(m_posLines[3]->text().toDouble());
    double b = degToRad(m_posLines[4]->text().toDouble());
    double g = degToRad(m_posLines[5]->text().toDouble());
    Vector3d translation(x, y, z);
    EulerAngle e(a, b, g);
    Transformation t(translation, Orientation(e));
    m_multiBody[0]->SetTransform(t);
    if(_emit) {
      Quaternion q;
      convertFromEulerAngle(q, e);

      emit TranslationChanged(translation);
      emit RotationChanged(q);
    }
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
    for(const auto& mb : m_multiBody) {
      Transformation t = (*mb->begin())->GetTransform();
      t.translation() += diff;
      mb->SetTransform(t);
    }

    if(_emit)
      emit TranslationChanged(m_center);
  }
}

