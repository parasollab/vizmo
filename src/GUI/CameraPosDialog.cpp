#include "CameraPosDialog.h"
#include "MainWindow.h"

#include <sstream>
using namespace std;

#include "Utilities/Camera.h"

CameraPosDialog::
CameraPosDialog(MainWindow* _mainWindow, Camera* _camera)
  : QDialog(_mainWindow), m_mainWindow(_mainWindow) {
    //initialize objects
    setWindowTitle("Camera Position");
    setFixedSize(200, 400);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    QLabel* mainLabel = new QLabel("Enter a camera position and\nthe point the camera looks at:", this);
    mainLabel->setStyleSheet("font:9pt");

    QLabel* eyeLabel[4];
    QLabel* atLabel[4];

    eyeLabel[0] = new QLabel("Eye", this);
    eyeLabel[0]->setStyleSheet("font:9pt");
    eyeLabel[1] = new QLabel("x", this);
    eyeLabel[2] = new QLabel("y", this);
    eyeLabel[3] = new QLabel("z", this);
    m_lineEye[0] = new QLineEdit(this);
    m_lineEye[1] = new QLineEdit(this);
    m_lineEye[2] = new QLineEdit(this);
    atLabel[0] = new QLabel("At", this);
    atLabel[0]->setStyleSheet("font:9pt");
    atLabel[1] = new QLabel("x", this);
    atLabel[2] = new QLabel("y", this);
    atLabel[3] = new QLabel("z", this);
    m_lineAt[0] = new QLineEdit(this);
    m_lineAt[1] = new QLineEdit(this);
    m_lineAt[2] = new QLineEdit(this);

    //setup the layout
    QGridLayout* layout = new QGridLayout(this);
    setLayout(layout);

    layout->addWidget(mainLabel, 0, 0, 1, 2);
    layout->addWidget(eyeLabel[0], 2, 0);
    layout->addWidget(atLabel[0], 6, 0);
    for(size_t i = 0; i < 3; ++i) {
      layout->addWidget(eyeLabel[i+1], i+3, 0);
      layout->addWidget(m_lineEye[i], i+3, 1);
      layout->addWidget(atLabel[i+1], i+7, 0);
      layout->addWidget(m_lineAt[i], i+7, 1);
    }
    layout->addItem(new QSpacerItem(200, 15), 11, 0, 1, 2);
    layout->addWidget(buttonBox, 12, 0, 1, 2);

    //connect signals/slots
    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(AcceptData()));
    QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QMetaObject::connectSlotsByName(this);
    SetCamera(_camera);
    setAttribute(Qt::WA_DeleteOnClose);
  }

CameraPosDialog::
~CameraPosDialog() {}

void
CameraPosDialog::
SetCamera(Camera* _camera) {
  m_camera = _camera;
  const Vector3d& eye = m_camera->GetEye();
  Vector3d at = m_camera->GetAt();
  for(size_t i=0; i<3; i++) {
    m_lineEye[i]->setText(QString::number(eye[i]));
    m_lineAt[i]->setText(QString::number(at[i]));
  }
}

void
CameraPosDialog::
AcceptData() {
  Vector3d eye, at;
  for(size_t i=0; i < 3; i++) {
    eye[i] = m_lineEye[i]->text().toDouble();
    at[i] = m_lineAt[i]->text().toDouble();
  }
  m_camera->Set(eye, at);

  accept();
}
bool
CameraPosDialog::
SaveCameraPosition() {
  QString fn= QFileDialog::getSaveFileName (this,"Choose a file name for the Camera ",
      QString::null, "Files(*.camera)");

  if(!fn.isEmpty()) {
    string filename = fn.toStdString();
    if(filename.find(".camera") == string::npos)
      filename += ".camera";
    ofstream ofs;
    ofs.open(filename.c_str());
    ofs << "Camera Configuration: Eye, At" << endl
        << m_camera->GetEye() << endl
        << m_camera->GetAt();
    ofs.close();

    accept();
    return true;
  }
  reject();
  return false;

}
void
CameraPosDialog::
LoadCameraPosition() {
  QString fn = QFileDialog::getOpenFileName(this,  "Choose a Camera to open",
      QString::null, "Files (*.camera)");

  if(!fn.isEmpty()) {
    string filename = fn.toStdString();
    ifstream ifs;
    ifs.open(filename.c_str());
    string fileHeader;
    getline(ifs, fileHeader);
    double x,y,z;
    ifs >> x >> y >> z;
    Vector3d eye(x, y, z);
    ifs >> x >> y >> z;
    Vector3d at(x, y, z);
    m_camera->Set(eye, at);

    ifs.close();
    accept();
  }
  else {
    reject();
    m_mainWindow->statusBar()->showMessage("Loading aborted");
  }
}


