#include "MainWindow.h"

#include "AnimationWidget.h"
#include "FileListDialog.h"
#include "GLWidget.h"
#include "MainMenu.h"
#include "ModelSelectionWidget.h"
#include "OptionsBase.h"
#include "PlanningOptions.h"
#include "TextWidget.h"
#include "ToolTabOptions.h"
#include "ToolTabWidget.h"
#include "Models/Vizmo.h"

#include "Icons/Vizmo.xpm"

MainWindow::
MainWindow(QWidget* _parent) : QMainWindow(_parent), m_vizmoInit(false) {
  setMinimumSize(960, 700);
  setWindowTitle("Vizmo++");
  m_gl = NULL;
  m_animationWidget = NULL;
  move(0,0);
  m_setQS = false;
  m_setQG = false;
  m_command = "";
  m_mainMenu = NULL;
  m_textWidget = NULL;
  m_modelSelectionWidget = NULL;
  m_toolTabWidget = NULL;
  m_dialogDock = NULL;
}

bool
MainWindow::
Init() {
  this->setWindowIcon(QPixmap(vizmoIcon));

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
MainWindow::
InitVizmo() {
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
MainWindow::
closeEvent(QCloseEvent* _event) {
  QThread* thread = ((PlanningOptions*)m_mainMenu->m_planningOptions)->GetMPThread();
  if(thread)
    delete thread;
  QMainWindow::closeEvent(_event);
}

bool
MainWindow::
CreateGUI() {
  m_animationWidget = new AnimationWidget("Animation", this);
  m_modelSelectionWidget = new ModelSelectionWidget(m_gl, this);
  m_mainMenu = new MainMenu(this);  //also creates the toolbars
  m_textWidget = new TextWidget(this);
  m_toolTabWidget = new ToolTabWidget(this); //depends on main menu
  m_mainMenu->m_toolTabOptions->Init(); //initialize the tool tab menu
  m_dialogDock = new QDockWidget(this);

  connect(m_animationWidget, SIGNAL(CallUpdate()), this, SLOT(UpdateScreen()));
  connect(m_modelSelectionWidget, SIGNAL(CallUpdate()), this, SLOT(UpdateScreen()));
  connect(m_modelSelectionWidget, SIGNAL(UpdateTextWidget()), m_textWidget, SLOT(SetText()));
  connect(m_gl, SIGNAL(selectByLMB()), m_modelSelectionWidget, SLOT(Select()));
  connect(m_gl, SIGNAL(clickByLMB()), m_modelSelectionWidget, SLOT(Select()));
  connect(m_gl, SIGNAL(clickByLMB()), m_textWidget, SLOT(SetText()));
  connect(m_gl, SIGNAL(selectByLMB()), m_textWidget, SLOT(SetText()));

  return true;
}

void
MainWindow::SetUpLayout(){
  QWidget* layoutWidget = new QWidget(this);
  QGridLayout* layout = new QGridLayout();
  layoutWidget->setLayout(layout); //Set this before actual layout specifications

  //The toolbars.
  //The allTogether toolbar holds them all together when window is expanded
  QToolBar* allTogether = new QToolBar(this);
  allTogether->addWidget(m_mainMenu->m_fileOptions->GetToolbar());
  allTogether->addWidget(m_mainMenu->m_captureOptions->GetToolbar());
  allTogether->addWidget(m_animationWidget);
  allTogether->addWidget(m_mainMenu->m_help->GetToolbar());

  //The menubar and main toolbar
  QDockWidget* topDock = new QDockWidget();
  topDock->layout()->setMenuBar(m_mainMenu->m_menuBar);
  topDock->setWidget(allTogether);
  topDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
  addDockWidget(Qt::TopDockWidgetArea, topDock);

  //Invisible row prevents top dock from messing up gl scene
  layout->setRowMinimumHeight(1, 10);

  //The GL Scene
  layout->addWidget(m_gl, 2, 2, 8, 32);
  layout->setRowStretch(2, 3);  //Fill as much vertical space as possible
  layout->setRowStretch(3, 3);
  layout->setRowStretch(4, 3);
  for(int i = 2; i <= 32; i++)
    layout->setColumnStretch(i, 2); //...also helps GL scene expand better

  //The model selection tree
  layout->addWidget(m_modelSelectionWidget, 2, 1, 2, 1);

  //The tool tab
  layout->addWidget(m_toolTabWidget, 4, 1, 7, 1);

  //The text display area
  layout->addWidget(m_textWidget, 10, 2, 1, 32);

  //The dialog dock. All dialogs built to width 200.
  QTabWidget* dialogTab = new QTabWidget(m_dialogDock);
  dialogTab->setFixedWidth(205);
  m_dialogDock->setFixedWidth(205);
  m_dialogDock->setWidget(dialogTab);
  m_dialogDock->setVisible(false);
  addDockWidget(Qt::RightDockWidgetArea, m_dialogDock);

  //Finally, set layout onto MainWin
  setCentralWidget(layoutWidget);
}

void
MainWindow::
keyPressEvent(QKeyEvent* _e) {
  switch(_e->key()){
    case Qt::Key_Escape: qApp->quit();
  }
}

void
MainWindow::
UpdateScreen() {
  m_gl->updateGL();
}

void
MainWindow::
HideDialogDock() {
  QTabWidget* tabs = dynamic_cast<QTabWidget*>(m_dialogDock->widget());
  QDialog* dialog = dynamic_cast<QDialog*>(sender());
  int index = tabs->indexOf(dialog);

  tabs->removeTab(index);
  if(tabs->count() == 0)
    m_dialogDock->hide();
}

//Display a dialog in the tabbed dialog dock
void
MainWindow::
ShowDialog(QDialog* _dialog) {
  connect(_dialog, SIGNAL(finished(int)), this, SLOT(HideDialogDock()));
  QTabWidget* tabs = dynamic_cast<QTabWidget*>(m_dialogDock->widget());
  tabs->addTab(_dialog, _dialog->windowTitle());
  tabs->setCurrentWidget(_dialog);
  if(!m_dialogDock->isVisible())
    m_dialogDock->show();
  _dialog->show();
}

void
MainWindow::
ResetDialogs() {
  QTabWidget* tabs = dynamic_cast<QTabWidget*>(m_dialogDock->widget());
  while(tabs->count() > 0)
    tabs->currentWidget()->close();
  m_dialogDock->hide();
}
