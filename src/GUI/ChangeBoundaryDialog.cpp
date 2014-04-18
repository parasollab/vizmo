#include "ChangeBoundaryDialog.h"

#include "GUI/BoundingBoxWidget.h"
#include "GUI/BoundingSphereWidget.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

ChangeBoundaryDialog::ChangeBoundaryDialog(QWidget* _parent) : QDialog(_parent) {
  //initialize dialog values
  resize(300, 150);
  setWindowTitle("Change Boundary");

  //construct objects
  QPushButton* loadButton = new QPushButton("OK", this);
  QPushButton* cancelButton = new QPushButton("Cancel", this);
  QRadioButton* boxButton = new QRadioButton("Bounding Box", this);
  QRadioButton* sphereButton = new QRadioButton("Bounding Sphere", this);

  m_boxWidget = new BoundingBoxWidget(this);
  m_sphereWidget = new BoundingSphereWidget(this);


  m_isBox = true;

  connect(loadButton, SIGNAL(clicked()), this, SLOT(SetBoundary()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(sphereButton, SIGNAL(clicked()), this, SLOT(ChangeToSphereDialog()));
  connect(boxButton, SIGNAL(clicked()), this, SLOT(ChangeToBoxDialog()));

  //set up layout
  QGridLayout* layout = new QGridLayout;
  setLayout(layout);

  layout->addWidget(boxButton, 0, 0);
  layout->addWidget(sphereButton, 0, 1);
  layout->addWidget(m_boxWidget, 1, 0, 1, 2);
  layout->addWidget(m_sphereWidget, 1, 0, 1, 2);
  layout->addWidget(loadButton, 2, 0);
  layout->addWidget(cancelButton, 2, 1);

  const string& name = GetVizmo().GetEnv()->GetBoundary()->Name();
  if(name == "Bounding Box")
    ChangeToBoxDialog();
  else
    ChangeToSphereDialog();

  QDialog::show();
}

void
ChangeBoundaryDialog::SetBoundary() {
  if(m_isBox)
    m_boxWidget->SetBoundary();
  else
    m_sphereWidget->SetBoundary();
  accept();
}

void
ChangeBoundaryDialog::ChangeToSphereDialog(){
  m_isBox = false;
  m_boxWidget->setVisible(false);
  m_sphereWidget->setVisible(true);
}


void
ChangeBoundaryDialog::ChangeToBoxDialog(){
  m_isBox = true;
  m_boxWidget->setVisible(true);
  m_sphereWidget->setVisible(false);
}

