#include <sstream>

#include "SliderDialog.h"

SliderDialog::SliderDialog(QString _windowTitle, QString _instructions,
  int _rangeStart, int _rangeEnd, int _startValue,
  QWidget* _parent) : QDialog(_parent){

  QGridLayout* layout = new QGridLayout();
  setLayout(layout);
  setWindowTitle(_windowTitle);

  QLabel* instructions = new QLabel(this);
  instructions->setText(_instructions);
  layout->addWidget(instructions, 1, 1, 1, 4);

  m_value = new QLabel(this);
  m_value->setFixedWidth(70);
  m_value->setText("100%");
  layout->addWidget(m_value, 2, 1, 1, 1);

  m_slider = new QSlider(this);
  m_slider->setOrientation(Qt::Vertical);
  m_slider->setRange(_rangeStart, _rangeEnd);
  m_slider->setValue(_startValue);
  m_startValue = _startValue;
  m_oldValue = _startValue;
  layout->addWidget(m_slider, 2, 2, 1, 1);

  QDialogButtonBox* okayCancel = new QDialogButtonBox(this);
  okayCancel->setFixedSize(120, 32);
  okayCancel->setOrientation(Qt::Horizontal);
  okayCancel->setStandardButtons(QDialogButtonBox::Cancel
    |QDialogButtonBox::Ok);
  layout->addWidget(okayCancel, 3, 1, 1, 2);

  connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(UpdatePercentile()));
  connect(okayCancel, SIGNAL(accepted()), this, SLOT(accept()));
  connect(okayCancel, SIGNAL(rejected()), this, SLOT(reject()));

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
