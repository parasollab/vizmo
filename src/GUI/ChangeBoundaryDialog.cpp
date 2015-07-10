#include "ChangeBoundaryDialog.h"
#include "ModelSelectionWidget.h"

#include "GUI/BoundingBoxWidget.h"
#include "GUI/BoundingSphereWidget.h"
#include "GUI/GLWidget.h"
#include "GUI/MainWindow.h"
#include "Models/BoundaryModel.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"


ChangeBoundaryDialog::
ChangeBoundaryDialog(MainWindow* _mainWindow) : QDialog(_mainWindow) {
  //initialize dialog values
  setFixedSize(200, 275);
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

  layout->addWidget(boxButton, 0, 0, 1, 2);
  layout->addWidget(sphereButton, 1, 0, 1, 2);
  layout->addWidget(m_boxWidget, 2, 0, 1, 2);
  layout->addWidget(m_sphereWidget, 2, 0, 1, 2);
  layout->addWidget(loadButton, 3, 0);
  layout->addWidget(cancelButton, 3, 1);

  const string& name = GetVizmo().GetEnv()->GetBoundary()->Name();
  if(name == "Bounding Box")
    ChangeToBoxDialog();
  else
    ChangeToSphereDialog();

  //set delete on close
  setAttribute(Qt::WA_DeleteOnClose);
}


void
ChangeBoundaryDialog::
SetBoundary() {
  if(m_isBox)
    m_boxWidget->SetBoundary();
  else
    m_sphereWidget->SetBoundary();

  vector<Model*>& sel = GetVizmo().GetSelectedModels();
  sel.clear();
  sel.push_back(GetVizmo().GetEnv()->GetBoundary().get());
  GetMainWindow()->GetModelSelectionWidget()->ResetLists();
  GetMainWindow()->GetGLWidget()->updateGL();
  accept();
}


void
ChangeBoundaryDialog::
ChangeToSphereDialog() {
  m_isBox = false;
  m_boxWidget->setVisible(false);
  m_sphereWidget->setVisible(true);
}


void
ChangeBoundaryDialog::
ChangeToBoxDialog() {
  m_isBox = true;
  m_boxWidget->setVisible(true);
  m_sphereWidget->setVisible(false);
}
