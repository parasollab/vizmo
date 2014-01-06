#include "CameraPosDialog.h"

#include <sstream>
using namespace std;

#include "Utilities/Camera.h"

CameraPosDialog::CameraPosDialog(QWidget* _parent) : QDialog(_parent) {
  //initialize objects
  setWindowTitle("Camera Position");

  QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

  QLabel* mainLabel = new QLabel("Enter a camera position and the point where the camera looks at:", this);

  QLabel* eyeLabel[4];
  QLabel* atLabel[4];

  eyeLabel[0] = new QLabel("Eye", this);
  eyeLabel[1] = new QLabel("x", this);
  eyeLabel[2] = new QLabel("y", this);
  eyeLabel[3] = new QLabel("z", this);
  m_lineEye[0] = new QLineEdit(this);
  m_lineEye[1] = new QLineEdit(this);
  m_lineEye[2] = new QLineEdit(this);
  atLabel[0] = new QLabel("At", this);
  atLabel[1] = new QLabel("x", this);
  atLabel[2] = new QLabel("y", this);
  atLabel[3] = new QLabel("z", this);
  m_lineAt[0] = new QLineEdit(this);
  m_lineAt[1] = new QLineEdit(this);
  m_lineAt[2] = new QLineEdit(this);

  //setup the layout
  QGridLayout* layout = new QGridLayout(this);
  setLayout(layout);

  layout->addWidget(mainLabel, 0, 0, 1, 4);
  layout->addWidget(eyeLabel[0], 1, 0);
  layout->addWidget(atLabel[0], 1, 2);
  for(size_t i = 0; i < 3; ++i) {
    layout->addWidget(eyeLabel[i+1], i+2, 0);
    layout->addWidget(m_lineEye[i], i+2, 1);
    layout->addWidget(atLabel[i+1], i+2, 2);
    layout->addWidget(m_lineAt[i], i+2, 3);
  }

  layout->addWidget(buttonBox, 5, 0, 1, 4);

  //connect signals/slots
  QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(AcceptData()));
  QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

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
