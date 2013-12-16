#include "CameraPosDialog.h"

#include <sstream>
using namespace std;

#include "Utilities/Camera.h"

CameraPosDialog::CameraPosDialog(QWidget* _parent) : QDialog(_parent) {
  //initialize objects
  setWindowTitle("Camera Position");

  m_buttonBox = new QDialogButtonBox(this);
  m_buttonBox->setOrientation(Qt::Horizontal);
  m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

  m_label = new QLabel("Enter a camera position and the point where the camera looks at:", this);

  m_labelEye[0] = new QLabel("Eye", this);
  m_labelEye[1] = new QLabel("x", this);
  m_labelEye[2] = new QLabel("y", this);
  m_labelEye[3] = new QLabel("z", this);
  m_lineEye[0] = new QLineEdit(this);
  m_lineEye[1] = new QLineEdit(this);
  m_lineEye[2] = new QLineEdit(this);
  m_labelAt[0] = new QLabel("At", this);
  m_labelAt[1] = new QLabel("x", this);
  m_labelAt[2] = new QLabel("y", this);
  m_labelAt[3] = new QLabel("z", this);
  m_lineAt[0] = new QLineEdit(this);
  m_lineAt[1] = new QLineEdit(this);
  m_lineAt[2] = new QLineEdit(this);

  //setup the layout
  QGridLayout* layout = new QGridLayout(this);
  setLayout(layout);

  layout->addWidget(m_label, 0, 0, 1, 4);
  layout->addWidget(m_labelEye[0], 1, 0);
  layout->addWidget(m_labelAt[0], 1, 2);
  for(size_t i = 0; i < 3; ++i) {
    layout->addWidget(m_labelEye[i+1], i+2, 0);
    layout->addWidget(m_lineEye[i], i+2, 1);
    layout->addWidget(m_labelAt[i+1], i+2, 2);
    layout->addWidget(m_lineAt[i], i+2, 3);
  }

  layout->addWidget(m_buttonBox, 5, 0, 1, 4);

  //connect signals/slots
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
