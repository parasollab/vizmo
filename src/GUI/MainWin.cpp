#include "MainWin.h"

#include "Models/Vizmo.h"
#include "SceneWin.h"
#include "AnimationGUI.h"
#include "ItemSelectionGUI.h"
#include "TextGUI.h"
#include "FileListDialog.h"
#include "QueryGUI.h"
#include "MainMenu.h"
#include "OptionsBase.h"

#include <QKeyEvent>

#include "Icons/Eye.xpm"

VizmoMainWin::VizmoMainWin(QWidget* _parent)
  :QMainWindow(_parent), m_bVizmoInit(false){

  setMinimumSize(960, 700);
  setWindowTitle("Vizmo++");
  m_gl = NULL;
  m_animationGUI = m_animationDebugGUI = NULL;
  move(0,0);
  m_setQS = false;
  m_setQG = false;
  command = "";
  m_mainMenu = NULL;
  m_outbox = NULL;
  m_layout = NULL;
  m_allTogether = NULL;
  m_objTextLayout = NULL;
  m_layoutWidget = NULL;
  m_objectSelection = NULL;
  m_animationBarLayout = NULL;
}

VizmoMainWin::~VizmoMainWin(){

  //delete GL scene
  delete m_gl;
  //delete the animation bars
  delete m_animationGUI;
  delete m_animationDebugGUI;
  delete m_animationBarLayout;
  //delete item selection
  delete m_objectSelection;
  delete m_outbox;
  delete m_objTextLayout;
  //delete the menus
  delete m_mainMenu;
  delete m_allTogether;
  //delete the layouts last
  delete m_layout;
  delete m_layoutWidget;
}

bool
VizmoMainWin::Init(){

  this->setWindowIcon(QPixmap(eye));
  m_layoutWidget = new QWidget();

  //Create GLModel
  if((m_gl = new VizGLWin(this, this)) == NULL)
    return false;

  // Create Other GUI
  if(CreateGUI() == false)
    return false;

  SetUpLayout();
  statusBar()->showMessage("Ready");
  return true;
}

bool
VizmoMainWin::InitVizmo(){

  if(m_bVizmoInit)
    return true;

  m_bVizmoInit=true;

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
  m_gl->resetCamera();
  //reset guis
  m_animationGUI->reset();
  m_animationDebugGUI->reset();
  m_objectSelection->ResetLists();
  m_mainMenu->CallReset();
  GetVizmo().RandomizeCCColors();
  m_gl->updateGL();

  return true;
}

//Protected//////////////////////////////////////////////////////////////////

bool
VizmoMainWin::CreateGUI(){

  m_animationGUI = new VizmoAnimationGUI("Path", this, "Path");
  connect(m_animationGUI,SIGNAL(callUpdate()),this,SLOT(updateScreen()));

  m_animationDebugGUI=new VizmoAnimationGUI("Debug", this, "Debug");
  connect(m_animationDebugGUI,SIGNAL(callUpdate()),this,SLOT(updateScreen()));

  m_objectSelection = new VizmoItemSelectionGUI(this);
  connect(m_objectSelection, SIGNAL(CallUpdate()),this,SLOT(updateScreen()));

  m_mainMenu = new MainMenu(this);  //also creates the toolbars

  m_outbox = new TextGUI(this);

  connect(m_animationDebugGUI,SIGNAL(callUpdate()), m_outbox, SLOT(SetText()));
  connect(m_objectSelection, SIGNAL(UpdateTextGUI()), m_outbox, SLOT(SetText()));
  connect(m_gl, SIGNAL(selectByLMB()), m_objectSelection, SLOT(Select()));
  connect(m_gl, SIGNAL(clickByLMB()), m_objectSelection, SLOT(Select()));
  //HandleSelect now in Plum/MapObj/MapModel.cpp and temporarily disabled
  // connect(m_gl, SIGNAL(selectByLMB()), m_roadmapGUI, SLOT(handleSelect()));
  connect(m_gl, SIGNAL(clickByLMB()), m_outbox, SLOT(SetText()));
  connect(m_gl, SIGNAL(selectByLMB()), m_outbox, SLOT(SetText()));
  // connect(m_gl, SIGNAL(MRbyGLI()), m_roadmapGUI, SLOT(printRobCfg()));

  return true;
}

void
VizmoMainWin::SetUpLayout(){

  m_layout = new QGridLayout;
  m_layoutWidget->setLayout(m_layout); //Set this before actual layout specifications
  m_layout->setHorizontalSpacing(3);

  //The toolbars.
  //The m_allTogether toolbar holds them all together when window is expanded
  m_allTogether = new QToolBar(this);
  m_allTogether->addWidget(m_mainMenu->m_fileOptions->GetToolbar());
  m_allTogether->addWidget(m_mainMenu->m_sceneOptions->GetToolbar());
  m_allTogether->addWidget(m_mainMenu->m_environmentOptions->GetToolbar());
  m_allTogether->addWidget(m_mainMenu->m_roadmapOptions->GetToolbar());
  m_allTogether->addWidget(m_mainMenu->m_pathOptions->GetToolbar());
  m_allTogether->addWidget(m_mainMenu->m_captureOptions->GetToolbar());
  m_allTogether->addWidget(m_mainMenu->m_help->GetToolbar());

  m_layout->addWidget(m_allTogether, 1, 1, 1, 25);

  m_objTextLayout = new QVBoxLayout;
  m_objTextLayout->addWidget(m_objectSelection); //The Environment Objects list
  m_objTextLayout->addWidget(m_outbox);          //The TextGUI
  m_objTextLayout->setStretchFactor(m_objectSelection, 1);

  m_layout->addLayout(m_objTextLayout, 2, 1, 4, 5);

  m_animationBarLayout = new QVBoxLayout;
  m_animationBarLayout->addWidget(m_animationGUI);
  m_animationBarLayout->addWidget(m_animationDebugGUI);

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
  m_layout->setMenuBar(m_mainMenu->m_menubar);

  setCentralWidget(m_layoutWidget); //Finally, set layout onto MainWin
}

void
VizmoMainWin::keyPressEvent (QKeyEvent* _e){

  switch(_e->key()){
    case Qt::Key_Escape: qApp->quit();
  }
}

void
VizmoMainWin::updateScreen(){

    m_gl->updateGL();
}

