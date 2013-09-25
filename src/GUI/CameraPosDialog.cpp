#include "CameraPosDialog.h"

#include <sstream>

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>

using namespace std;

#include "Utilities/GL/Camera.h"

CameraPosDialog::CameraPosDialog(QWidget* _parent) :QDialog(_parent) {
  resize(450, 245);
  setWindowTitle("Set Camera Position and Rotation");

  m_buttonBox = new QDialogButtonBox(this);
  m_buttonBox->setGeometry(QRect(100, 210, 341, 32));
  m_buttonBox->setOrientation(Qt::Horizontal);
  m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

  m_label = new QLabel(this);
  m_label->setText("Enter a 3-D point and rotation angle to position the camera");
  m_label->setGeometry(QRect(10, 30, 421, 16));

  m_formLayoutWidget = new QWidget(this);
  m_formLayoutWidget->setGeometry(QRect(30, 70, 172, 95));
  m_formLayout = new QFormLayout(m_formLayoutWidget);
  m_formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  m_formLayout->setContentsMargins(0, 0, 0, 0);

  m_xLabel = new QLabel(m_formLayoutWidget);
  m_xLabel->setText("X:");
  m_formLayout->setWidget(0, QFormLayout::LabelRole, m_xLabel);

  m_xLineEdit = new QLineEdit(m_formLayoutWidget);
  m_formLayout->setWidget(0, QFormLayout::FieldRole, m_xLineEdit);

  m_yLabel = new QLabel(m_formLayoutWidget);
  m_yLabel->setText("Y:");
  m_formLayout->setWidget(1, QFormLayout::LabelRole, m_yLabel);

  m_yLineEdit = new QLineEdit(m_formLayoutWidget);
  m_formLayout->setWidget(1, QFormLayout::FieldRole, m_yLineEdit);

  m_zLabel = new QLabel(m_formLayoutWidget);
  m_zLabel->setText("Z:");
  m_formLayout->setWidget(2, QFormLayout::LabelRole, m_zLabel);

  m_zLineEdit = new QLineEdit(m_formLayoutWidget);
  m_formLayout->setWidget(2, QFormLayout::FieldRole, m_zLineEdit);

  m_formLayoutWidget_2 = new QWidget(this);
  m_formLayoutWidget_2->setGeometry(QRect(220, 70, 213, 80));
  m_formLayout_2 = new QFormLayout(m_formLayoutWidget_2);
  m_formLayout_2->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  m_formLayout_2->setContentsMargins(0, 0, 0, 0);

  m_azimLabel = new QLabel(m_formLayoutWidget_2);
  m_azimLabel->setText("Azimuth:");
  m_elevLabel = new QLabel(m_formLayoutWidget_2);
  m_elevLabel->setText("Elevation:");
  m_formLayout_2->setWidget(0, QFormLayout::LabelRole, m_azimLabel);
  m_formLayout_2->setWidget(1, QFormLayout::LabelRole, m_elevLabel);

  m_azimLineEdit = new QLineEdit(m_formLayoutWidget_2);
  m_elevLineEdit = new QLineEdit(m_formLayoutWidget_2);
  m_formLayout_2->setWidget(0, QFormLayout::FieldRole, m_azimLineEdit);
  m_formLayout_2->setWidget(1, QFormLayout::FieldRole, m_elevLineEdit);

  QObject::connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(AcceptData()));
  QObject::connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QMetaObject::connectSlotsByName(this);
}

void
CameraPosDialog::SetCameraPos(double _x, double _y, double _z, double _azim, double _elev){
  ostringstream oss;
  oss << _x;
  m_xLineEdit->setText(oss.str().c_str());

  oss.str("");
  oss << _y;
  m_yLineEdit->setText(oss.str().c_str());

  oss.str("");
  oss << _z;
  m_zLineEdit->setText(oss.str().c_str());

  oss.str("");
  oss << _azim;
  m_azimLineEdit->setText(oss.str().c_str());

  oss.str("");
  oss << _elev;
  m_elevLineEdit->setText(oss.str().c_str());
}

void
CameraPosDialog::AcceptData(){
  double x = (m_xLineEdit->text()).toDouble();
  double y = (m_yLineEdit->text()).toDouble();
  double z = (m_zLineEdit->text()).toDouble();

  double azim = (m_azimLineEdit->text()).toDouble();
  double elev = (m_elevLineEdit->text()).toDouble();

  GetCameraFactory().GetCurrentCamera()->Set(Point3d(x, y, z), azim, elev);

  accept();
}
