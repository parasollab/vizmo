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
#ifdef PMPCfg
  QRadioButton* regionStrategyButton = new QRadioButton("RegionStrategy", this);
  QRadioButton* pathStrategyButton = new QRadioButton("PathStrategy", this);
  QRadioButton* irrtStrategyButton = new QRadioButton("IRRT", this);
  QRadioButton* regionRRTButton = new QRadioButton("RegionRRT", this);
  QRadioButton* sparkRegionButton = new QRadioButton("SparkRegion", this);
#elif defined(PMPState)
  QRadioButton* kinoRegionRRTButton = new QRadioButton("KinodynamicRegionRRT", this);
#endif

  m_radioGroup = new QButtonGroup;
  m_radioGroup->setExclusive(true);
#ifdef PMPCfg
  m_radioGroup->addButton(regionStrategyButton);
  m_radioGroup->addButton(pathStrategyButton);
  m_radioGroup->addButton(irrtStrategyButton);
  m_radioGroup->addButton(regionRRTButton);
  m_radioGroup->addButton(sparkRegionButton);
#elif defined(PMPState)
  m_radioGroup->addButton(kinoRegionRRTButton);
#endif

  //make default selection
#ifdef PMPCfg
  regionStrategyButton->setChecked(true);
#elif defined(PMPState)
  kinoRegionRRTButton->setChecked(true);
#endif

  //add standard ok and cancel buttons
  okCancel->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(okCancel, SIGNAL(accepted()), this, SLOT(accept()));
  connect(okCancel, SIGNAL(rejected()), this, SLOT(close()));

  QVBoxLayout* layout = new QVBoxLayout;
  setLayout(layout);

#ifdef PMPCfg
  layout->addWidget(regionStrategyButton);
  layout->addWidget(pathStrategyButton);
  layout->addWidget(irrtStrategyButton);
  layout->addWidget(regionRRTButton);
  layout->addWidget(sparkRegionButton);
#elif defined(PMPState)
  layout->addWidget(kinoRegionRRTButton);
#endif
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
