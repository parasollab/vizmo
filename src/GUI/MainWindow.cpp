#include "MainWindow.h"

#include <QtGui>

#include "AnimationWidget.h"
#include "EnvironmentOptions.h"
#include "FileListDialog.h"
#include "GLWidget.h"
#include "MainMenu.h"
#include "ModelSelectionWidget.h"
#include "OptionsBase.h"
#include "TextWidget.h"
#include "Models/Vizmo.h"

#include "Icons/Eye.xpm"

MainWindow::MainWindow(QWidget* _parent)
  : QMainWindow(_parent), m_vizmoInit(false){

  setMinimumSize(960, 700);
  setWindowTitle("Vizmo++");
  m_gl = NULL;
  m_animationWidget = NULL;
  move(0,0);
  m_setQS = false;
  m_setQG = false;
  m_command = "";
  m_mainMenu = NULL;
  m_outbox = NULL;
  m_layout = NULL;
  m_allTogether = NULL;
  m_objTextLayout = NULL;
  m_layoutWidget = NULL;
  m_modelSelectionWidget = NULL;
  m_animationBarLayout = NULL;
}

bool
MainWindow::Init(){
  this->setWindowIcon(QPixmap(eye));
  m_layoutWidget = new QWidget(this);

  //Create Model
  if((m_gl = new GLWidget(this, this)) == NULL)
    return false;

  // Create Other GUI
  if(CreateGUI() == false)
    return false;

  SetUpLayout();
  statusBar()->showMessage("Ready");

  return true;
}

bool
MainWindow::InitVizmo(){
  if(m_vizmoInit)
    return true;

  m_vizmoInit=true;

  if(m_args.empty())
    return true; //nothing to init...

  //Here we use the first argument, but in the future
  //we should use all of them to load files.
  FileListDialog flDialog(m_args[0], this);

  if(flDialog.exec() != QDialog::Accepted)
    return false;

  if(!GetVizmo().InitModels())
    return false;

  resize(width(),height());
  m_args.clear();
  m_gl->ResetCamera();
  //reset guis
  m_animationWidget->Reset();
  m_modelSelectionWidget->ResetLists();
  m_mainMenu->CallReset();
  m_gl->updateGL();

  return true;
}

void
MainWindow::closeEvent(QCloseEvent* _event) {
  QThread* thread = ((EnvironmentOptions*)m_mainMenu->m_environmentOptions)->GetMPThread();
  if(thread)
    delete thread;
  QMainWindow::closeEvent(_event);
}

bool
MainWindow::CreateGUI(){
  m_animationWidget = new AnimationWidget("Animation", this);
  connect(m_animationWidget, SIGNAL(CallUpdate()), this, SLOT(UpdateScreen()));

  m_modelSelectionWidget = new ModelSelectionWidget(m_gl, this);
  connect(m_modelSelectionWidget, SIGNAL(CallUpdate()), this, SLOT(UpdateScreen()));

  m_mainMenu = new MainMenu(this);  //also creates the toolbars

  m_outbox = new TextWidget(this);

  connect(m_modelSelectionWidget, SIGNAL(UpdateTextWidget()), m_outbox, SLOT(SetText()));
  connect(m_gl, SIGNAL(selectByLMB()), m_modelSelectionWidget, SLOT(Select()));
  connect(m_gl, SIGNAL(clickByLMB()), m_modelSelectionWidget, SLOT(Select()));
  //HandleSelect now in Plum/MapObj/MapModel.cpp and temporarily disabled
  // connect(m_gl, SIGNAL(selectByLMB()), m_roadmapGUI, SLOT(handleSelect()));
  connect(m_gl, SIGNAL(clickByLMB()), m_outbox, SLOT(SetText()));
  connect(m_gl, SIGNAL(selectByLMB()), m_outbox, SLOT(SetText()));
  // connect(m_gl, SIGNAL(MRbyGLI()), m_roadmapGUI, SLOT(printRobCfg()));

  return true;
}

void
MainWindow::SetUpLayout(){
  m_layout = new QGridLayout();
  m_layoutWidget->setLayout(m_layout); //Set this before actual layout specifications
  m_layout->setHorizontalSpacing(3);

  //The toolbars.
  //The m_allTogether toolbar holds them all together when window is expanded
  m_allTogether = new QToolBar(this);
  m_allTogether->addWidget(m_mainMenu->m_fileOptions->GetToolbar());
  m_allTogether->addWidget(m_mainMenu->m_glWidgetOptions->GetToolbar());
  m_allTogether->addWidget(m_mainMenu->m_environmentOptions->GetToolbar());
  m_allTogether->addWidget(m_mainMenu->m_roadmapOptions->GetToolbar());
  m_allTogether->addWidget(m_mainMenu->m_pathOptions->GetToolbar());
  m_allTogether->addWidget(m_mainMenu->m_queryOptions->GetToolbar());
  m_allTogether->addWidget(m_mainMenu->m_captureOptions->GetToolbar());
  m_allTogether->addWidget(m_mainMenu->m_help->GetToolbar());

  m_layout->addWidget(m_allTogether, 1, 1, 1, 25);

  m_objTextLayout = new QVBoxLayout();
  m_objTextLayout->addWidget(m_modelSelectionWidget); //The Environment Objects list
  m_objTextLayout->addWidget(m_outbox);          //The TextWidget
  m_objTextLayout->setStretchFactor(m_modelSelectionWidget, 1);

  m_layout->addLayout(m_objTextLayout, 2, 1, 4, 5);

  m_animationBarLayout = new QVBoxLayout();
  m_animationBarLayout->addWidget(m_animationWidget);

  m_layout->addLayout(m_animationBarLayout, 8, 1, 9, 25);


  //The GL Scene
  m_layout->addWidget(m_gl, 2, 6, 4, 25);
  m_layout->setRowStretch(1, 3);
  m_layout->setRowStretch(2, 3);  //help the GL scene expand better
  m_layout->setRowStretch(3, 3);
  m_layout->setRowStretch(4, 3);

  for(int i = 6; i <= 25; i++)
    m_layout->setColumnStretch(i, 2); //...also helps GL scene expand better

  //The menubar
  m_layout->setMenuBar(m_mainMenu->m_menuBar);

  setCentralWidget(m_layoutWidget); //Finally, set layout onto MainWin
}

void
MainWindow::keyPressEvent(QKeyEvent* _e) {
  switch(_e->key()){
    case Qt::Key_Escape: qApp->quit();
  }
}

void
MainWindow::UpdateScreen() {
  m_gl->updateGL();
}

