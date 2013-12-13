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
  setWindowTitle("Set Camera Position and Rotation");

  m_buttonBox = new QDialogButtonBox(this);
  m_buttonBox->setGeometry(QRect(130, 170, 246, 32));
  m_buttonBox->setOrientation(Qt::Horizontal);
  m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

  m_label = new QLabel("Enter a 3-D point and rotation angle to position the camera:", this);
  m_label->setGeometry(QRect(10, 30, 421, 16));
  for(int i=0; i<3; i++){
    stringstream lab;
    lab<<"eye["<<i<<"]"<<endl;
    m_labelEye[i] = new QLabel(QString::fromStdString(lab.str()), this);
    m_labelEye[i]->setGeometry(QRect(20+140*i, 70, 50, 16));
    m_lineEye[i] = new QLineEdit(this);
    m_lineEye[i]->setGeometry(QRect(80+140*i, 70, 70, 20));
  }
  for(int i=0; i<3; i++){
    stringstream lab;
    lab<<"center["<<i<<"]"<<endl;
    m_labelCenter[i] = new QLabel(QString::fromStdString(lab.str()), this);
    m_labelCenter[i]->setGeometry(QRect(20+140*i, 100, 60, 16));
    m_lineCenter[i] = new QLineEdit(this);
    m_lineCenter[i]->setGeometry(QRect(80+140*i, 100, 70, 20));
  }
  for(int i=0; i<3; i++){
    stringstream lab;
    lab<<"eye["<<i<<"]"<<endl;
    m_labelUp[i] = new QLabel(QString::fromStdString(lab.str()), this);
    m_labelUp[i]->setGeometry(QRect(20+140*i, 130, 50, 16));
    m_lineUp[i] = new QLineEdit(this);
    m_lineUp[i]->setGeometry(QRect(80+140*i, 130, 70, 20));
  }

  QObject::connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(AcceptData()));
  QObject::connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QMetaObject::connectSlotsByName(this);
}

void
CameraPosDialog::SetCamera(Camera* _camera){
  m_camera = _camera;
  vector<Vector3d> p = m_camera->GetCameraPos();
  for(int i=0; i<3; i++){
    m_lineEye[i]->setText(QString::number(p[0][i]));
    m_lineCenter[i]->setText(QString::number(p[1][i]));
    m_lineUp[i]->setText(QString::number(p[2][i]));
  }
}

void
CameraPosDialog::AcceptData(){
  Vector3d eye, center, up;
  for(int i=0; i<3; i++){
    eye[i] = m_lineEye[i]->text().toDouble();
    center[i] = m_lineCenter[i]->text().toDouble();
    up[i] = m_lineUp[i]->text().toDouble();
  }
  m_camera->Set(eye,center,up);

  accept();
}
