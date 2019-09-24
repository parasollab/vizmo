#include "HelpOptions.h"

#include <QtGui>
#include <QWhatsThis>
#include <QMessageBox>
#include <QAction>
#include <QToolBar>

#include "MainWindow.h"

#include "Icons/About.xpm"


HelpOptions::
HelpOptions(QWidget* _parent) : OptionsBase(_parent, "Help") {
  CreateActions();
  SetUpSubmenu();
  SetUpToolbar();
}

/*----------------------------- GUI Management -------------------------------*/

void
HelpOptions::
CreateActions() {
  //1. Create actions and add them to the map
  m_actions["whatsThis"] = QWhatsThis::createAction(this);
  m_actions["about"] = new QAction(QPixmap(abouticon), tr("&About"), this);

  //2. Set other specifications as necessary
  //...nothing to do in this case

  //3. Make connections
  connect(m_actions["about"], SIGNAL(triggered()), this, SLOT(ShowAboutBox()));
}


void
HelpOptions::
SetUpToolbar() {
  m_toolbar = new QToolBar(GetMainWindow());
  m_toolbar->addAction(m_actions["whatsThis"]);
}

/*----------------------------- Help Functions -------------------------------*/

void
HelpOptions::
ShowAboutBox() {
  QMessageBox::about(
      this,"Vizmo++\n",
      "A 3D Vizualiztion tool\n"
      "Active Developers:"
      "Jory Denny\n"
      "Read Sandstrom\n"
      "Brennen Taylor\n"
      "Matthew Cohen\n"
      "Alumni:\n"
      "Jyh-Ming Lien\n"
      "Aimee Vargas Estrada\n"
      "Bharatinder Singh Sandhu\n"
      "Nicole Julian\n"
      );
}
