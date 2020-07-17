#include "MainWindow.h"

#include <QStatusBar>
#include <QDockWidget>
#include <QGridLayout>
#include <QMessageBox>
#include <QMenuBar>

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
#include "Utilities/PMPLExceptions.h"

#include "Icons/Vizmo.xpm"

/*--------------------------- Singleton Definition ---------------------------*/

MainWindow* window;
MainWindow*& GetMainWindow() {return window;}

/*------------------------------- Construction -------------------------------*/

MainWindow::
MainWindow(const vector<string>& _filenames) : QMainWindow(nullptr),
    m_args(_filenames) {
  // Enforce singleton and set to this.
  if(GetMainWindow())
    throw RunTimeException(WHERE, "MainWindow error: there can only be one "
        "MainWindow.");
  GetMainWindow() = this;

  setMinimumSize(960, 700);
  setWindowTitle("Vizmo++");
  setWindowIcon(QPixmap(vizmoIcon));

  CreateGUI();
  SetUpLayout();
  InitVizmo();
  statusBar()->showMessage("Ready");
}


bool
MainWindow::
InitVizmo() {
  if(m_vizmoInit)
    return true;
  m_vizmoInit = true;

  if(m_args.empty())
    return true; //nothing to init...

  //Here we use the first argument, but in the future
  //we should use all of them to load files.
  FileListDialog flDialog(m_args, this);

  if(flDialog.exec() != QDialog::Accepted)
    return false;

  if(!GetVizmo().InitModels())
    return false;

  resize(width(),height());
  m_args.clear();
  m_gl->ResetCamera();
  #ifdef USE_SPACEMOUSE
  m_gl->ResetCursor();
  #endif
  //reset guis
  m_animationWidget->Reset();
  m_modelSelectionWidget->ResetLists();
  m_mainMenu->CallReset();

  return true;
}


void
MainWindow::
closeEvent(QCloseEvent* _event) {
  /// Executes clean-up prior to closing the application.
  static_cast<PlanningOptions*>(m_mainMenu->m_planningOptions)->HaltMPThread();
  GetVizmo().Clean();
  QMainWindow::closeEvent(_event);
}


void
MainWindow::
CreateGUI() {
  m_gl = new GLWidget(this);
  m_animationWidget = new AnimationWidget("Animation", this);
  m_modelSelectionWidget = new ModelSelectionWidget(m_gl, this);
  m_mainMenu = new MainMenu(this);  //also creates the toolbars
  m_textWidget = new TextWidget(this);
  m_toolTabWidget = new ToolTabWidget(this); //depends on main menu
  m_mainMenu->ConfigureToolTabMenu(); //initialize the tool tab menu
  m_dialogDock = new QDockWidget(this);

  // Set up timer to redraw and refresh GUI
  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), m_gl, SLOT(updateGL()));
  m_timer->start(33);

  connect(m_modelSelectionWidget, SIGNAL(UpdateTextWidget()),
      m_textWidget, SLOT(SetText()));
  connect(m_gl, SIGNAL(selectByLMB()), m_modelSelectionWidget, SLOT(Select()));
  connect(m_gl, SIGNAL(clickByLMB()), m_modelSelectionWidget, SLOT(Select()));
  connect(m_gl, SIGNAL(clickByLMB()), m_textWidget, SLOT(SetText()));
  connect(m_gl, SIGNAL(selectByLMB()), m_textWidget, SLOT(SetText()));
  connect(this, SIGNAL(Alert(QString)), this, SLOT(ShowAlert(QString)));
}


void
MainWindow::
SetUpLayout() {
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
  dialogTab->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
  m_dialogDock->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
  m_dialogDock->setWidget(dialogTab);
  m_dialogDock->setVisible(false);
  addDockWidget(Qt::RightDockWidgetArea, m_dialogDock);

  //Finally, set layout onto MainWin
  setCentralWidget(layoutWidget);
}

/*--------------------------------- Dialogs ----------------------------------*/

void
MainWindow::
HideDialogDock() {
  /// This slot must be connected to a QDialog. If the dialog to be hidden is
  /// the only visible dialog, the dock will be hidden as well.
  QDialog* dialog = static_cast<QDialog*>(sender());
  QTabWidget* tabs = static_cast<QTabWidget*>(m_dialogDock->widget());
  int index = tabs->indexOf(dialog);
  tabs->removeTab(index);
  if(tabs->count() == 0)
    m_dialogDock->hide();
}


void
MainWindow::
ShowDialog(QDialog* _dialog) {
  connect(_dialog, SIGNAL(finished(int)), this, SLOT(HideDialogDock()));
  QTabWidget* tabs = static_cast<QTabWidget*>(m_dialogDock->widget());
  tabs->addTab(_dialog, _dialog->windowTitle());
  tabs->setCurrentWidget(_dialog);
  if(!m_dialogDock->isVisible())
    m_dialogDock->show();
  _dialog->show();
}


void
MainWindow::
ResetDialogs() {
  QTabWidget* tabs = static_cast<QTabWidget*>(m_dialogDock->widget());
  while(tabs->count() > 0)
    tabs->currentWidget()->close();
  m_dialogDock->hide();
}

/*---------------------------------- Alerts ----------------------------------*/

void
MainWindow::
AlertUser(string _s) {
  /// This method emits a signal to MainWindow::ShowAlert, which actually
  /// displays the pop-up. This implementation allows clients to request a
  /// pop-up from outside the GUI thread.
  emit Alert(QString(_s.c_str()));
}


void
MainWindow::
ShowAlert(QString _s) {
  QMessageBox m(this);
  m.setText(_s);
  m.exec();
}

/*----------------------------------------------------------------------------*/
