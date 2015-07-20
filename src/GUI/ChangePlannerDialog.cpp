#include "ChangePlannerDialog.h"

#include "GUI/MainWindow.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

ChangePlannerDialog::
ChangePlannerDialog(MainWindow* _mainWindow) : QDialog(_mainWindow),
  m_radioGroup(NULL) {

    //initialize dialog values
    setWindowTitle("Strategy Selections");

    const vector<string>& strategies = GetVizmo().GetAllStrategies();
    QVBoxLayout* layout = new QVBoxLayout;
    m_radioGroup = new QButtonGroup;

    for(const auto& s : strategies) {
      QRadioButton* strategy = new QRadioButton(s.c_str(), this);
      m_radioGroup->addButton(strategy);
      layout->addWidget(strategy);
    }

    if(m_radioGroup->buttons().size())
      m_radioGroup->buttons().first()->setChecked(true);
    m_radioGroup->setExclusive(true);

    QDialogButtonBox* okCancel = new QDialogButtonBox(this);
    okCancel->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel); 
    layout->addWidget(okCancel);
    setLayout(layout);

    //set accept to Accept for picking the planner
    connect(okCancel, SIGNAL(accepted()), this, SLOT(accept()));
    connect(okCancel, SIGNAL(rejected()), this, SLOT(close()));
  }

string
ChangePlannerDialog::
GetPlanner() {
  string temp = m_radioGroup->checkedButton()->text().toStdString();
  return temp.substr(temp.rfind(':') + 1);
}
