#include "CameraPosDialog.h"

#include <sstream>
using namespace std;

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>

#include "Utilities/Camera.h"

CameraPosDialog::CameraPosDialog(QWidget* _parent) : QDialog(_parent) {
  resize(450, 210);
  setWindowTitle("Camera Position");

  m_buttonBox = new QDialogButtonBox(this);
  m_buttonBox->setGeometry(QRect(130, 170, 246, 32));
  m_buttonBox->setOrientation(Qt::Horizontal);
  m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

  m_label = new QLabel("Enter a camera position and the point where the camera looks at:", this);
  m_label->setGeometry(QRect(10, 30, 421, 16));
  for(size_t i=0; i<3; i++){
    stringstream lab;
    lab << "eye[" << i << "]" << endl;
    m_labelEye[i] = new QLabel(QString::fromStdString(lab.str()), this);
    m_labelEye[i]->setGeometry(QRect(20+140*i, 70, 50, 16));
    m_lineEye[i] = new QLineEdit(this);
    m_lineEye[i]->setGeometry(QRect(80+140*i, 70, 70, 20));
  }
  for(size_t i=0; i<3; i++){
    stringstream lab;
    lab << "at[" << i << "]" << endl;
    m_labelAt[i] = new QLabel(QString::fromStdString(lab.str()), this);
    m_labelAt[i]->setGeometry(QRect(20+140*i, 100, 60, 16));
    m_lineAt[i] = new QLineEdit(this);
    m_lineAt[i]->setGeometry(QRect(80+140*i, 100, 70, 20));
  }

  QObject::connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(AcceptData()));
  QObject::connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QMetaObject::connectSlotsByName(this);
}

void
CameraPosDialog::SetCamera(Camera* _camera){
  m_camera = _camera;
  const Vector3d& eye = m_camera->GetEye();
  Vector3d at = m_camera->GetAt();
  for(size_t i=0; i<3; i++){
    m_lineEye[i]->setText(QString::number(eye[i]));
    m_lineAt[i]->setText(QString::number(at[i]));
  }
}

void
CameraPosDialog::AcceptData(){
  Vector3d eye, at;
  for(size_t i=0; i < 3; i++) {
    eye[i] = m_lineEye[i]->text().toDouble();
    at[i] = m_lineAt[i]->text().toDouble();
  }
  m_camera->Set(eye, at);

  accept();
}
