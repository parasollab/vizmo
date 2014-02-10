#include "HelpOptions.h"

#include <QtGui>

#include "OptionsBase.h"
#include "MainWindow.h"

#include "Icons/About.xpm"

HelpOptions::HelpOptions(QWidget* _parent, MainWindow* _mainWindow)
  : OptionsBase(_parent, _mainWindow) {
    CreateActions();
    SetUpSubmenu("Help");
    SetUpToolbar();
  }

void
HelpOptions::CreateActions(){

  //1. Create actions and add them to the map
  QAction* whatsThis = QWhatsThis::createAction(this);
  m_actions["whatsThis"] = whatsThis;
  QAction* about = new QAction(QPixmap(abouticon), tr("&About"), this);
  m_actions["about"] = about;

  //2. Set other specifications as necessary
  //...nothing to do in this case

  //3. Make connections
  connect(m_actions["about"], SIGNAL(triggered()), this, SLOT(ShowAboutBox()));
}

void
HelpOptions::SetUpToolbar(){
  m_toolbar = new QToolBar(m_mainWindow);
  m_toolbar->addAction(m_actions["whatsThis"]);
}

void
HelpOptions::Reset(){} //Nothing in this case...

//Help tips from OTHER options classes are used in the WhatsThis defined here,
//so of course SetHelpTips is not implemented here!
void
HelpOptions::SetHelpTips(){}

void
HelpOptions::ShowAboutBox(){

  QMessageBox::about(
      this,"Vizmo++\n",
      "A 3D Vizualiztion tool\n"
      "Authors:\n"
      "Jyh-Ming Lien\n"
      "Aimee Vargas Estrada\n"
      "Bharatinder Singh Sandhu\n"
      );
}












