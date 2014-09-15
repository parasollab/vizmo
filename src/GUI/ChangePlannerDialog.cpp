#include "ChangePlannerDialog.h"

#include "GUI/MainWindow.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

ChangePlannerDialog:: 
ChangePlannerDialog(MainWindow* _mainWindow) : QDialog(_mainWindow), 
  m_radioGroup(NULL) {

  //initialize dialog values
  setWindowTitle("Strategy Selections");

  //construct objects
  QDialogButtonBox* okCancel = new QDialogButtonBox(this);
  QRadioButton* regionStrategyButton = new QRadioButton("RegionStrategy", this);
  QRadioButton* pathStrategyButton = new QRadioButton("PathStrategy", this);
  QRadioButton* irrtStrategyButton = new QRadioButton("IRRT", this);

  m_radioGroup = new QButtonGroup;
  m_radioGroup->setExclusive(true);
  m_radioGroup->addButton(regionStrategyButton);
  m_radioGroup->addButton(pathStrategyButton);
  m_radioGroup->addButton(irrtStrategyButton);

  //make default selection
  regionStrategyButton->setChecked(true);

  //add standard ok and cancel buttons
  okCancel->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(okCancel, SIGNAL(accepted()), this, SLOT(accept()));
  connect(okCancel, SIGNAL(rejected()), this, SLOT(close()));
  
  QVBoxLayout* layout = new QVBoxLayout;
  setLayout(layout);

  layout->addWidget(regionStrategyButton);
  layout->addWidget(pathStrategyButton);
  layout->addWidget(irrtStrategyButton);
  layout->addWidget(okCancel);
}

string
ChangePlannerDialog::
GetPlanner() {
   return m_radioGroup->checkedButton()->text().toStdString();
}

//Also, look for other ok, cancel options
//in "ok" slot, call accept 
//FUTURE: strategies should not be hard coded
