#include <sstream>

#include <QLabel>
#include <QDialogButtonBox>

#include "SliderDialog.h"

SliderDialog::SliderDialog(QString _windowTitle, QString _instructions,
  int _rangeStart, int _rangeEnd, int _startValue,
  QWidget* _parent) : QDialog(_parent){

  resize(477, 139);
  setWindowTitle(_windowTitle);
  m_instructions = new QLabel(this);
  m_instructions->setGeometry(QRect(20, 20, 311, 17));
  m_instructions->setText(_instructions);

  m_okayCancel = new QDialogButtonBox(this);
  m_okayCancel->setGeometry(QRect(290, 100, 181, 32));
  m_okayCancel->setOrientation(Qt::Horizontal);
  m_okayCancel->setStandardButtons(QDialogButtonBox::Cancel
    |QDialogButtonBox::Ok);

  m_slider = new QSlider(this);
  m_slider->setGeometry(QRect(19, 60, 441, 20));
  m_slider->setOrientation(Qt::Horizontal);
  m_slider->setRange(_rangeStart, _rangeEnd);
  m_slider->setValue(_startValue);
  m_startValue = _startValue;
  m_oldValue = _startValue;
  connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(UpdatePercentile()));

  m_value = new QLabel(this);
  m_value->setGeometry(QRect(20, 90, 70, 17)); //long width for plenty of room
  m_value->setText("100%");

  connect(m_okayCancel, SIGNAL(accepted()), this, SLOT(accept()));
  connect(m_okayCancel, SIGNAL(rejected()), this, SLOT(reject()));

  QMetaObject::connectSlotsByName(this);
}

void
SliderDialog::show(){

  m_oldValue = m_slider->value();
  QDialog::show();
}

void
SliderDialog::reject(){

  m_slider->setSliderPosition(m_oldValue);
  QDialog::reject();
}

void
SliderDialog::UpdatePercentile(){

  double proportion = (double)m_slider->value() / (double)m_startValue;
  ostringstream oss;
  oss << proportion*100 << "%";
  QString qs((oss.str()).c_str());
  m_value->setText(qs);
}

void
SliderDialog::Reset(){

  m_slider->setSliderPosition(m_startValue);
}
