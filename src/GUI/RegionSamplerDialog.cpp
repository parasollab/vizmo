#include "RegionSamplerDialog.h"

#include <QVBoxLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QPushButton>

#include "Models/Vizmo.h"
#include "Models/CfgModel.h"
#include "Models/EdgeModel.h"
#include "Models/MapModel.h"

RegionSamplerDialog::
RegionSamplerDialog(QWidget* _parent) :
  QDialog(_parent) {

    setWindowTitle("Sampler Selection");

    //create widget layout
    const vector<string>& samplers = GetVizmo().GetAllSamplers();
    QVBoxLayout* layout = new QVBoxLayout(this);
    m_radioGroup = new QButtonGroup;

    //create radio buttons
    for(const auto& s : samplers) {
      QRadioButton* button = new QRadioButton(s.c_str(), this);
      m_radioGroup->addButton(button);
      layout->addWidget(button);
    }

    if(m_radioGroup->buttons().size())
      m_radioGroup->buttons().first()->setChecked(true);
    m_radioGroup->setExclusive(true);

    //create accept button
    QPushButton* setButton = new QPushButton("Set", this);
    connect(setButton, SIGNAL(clicked()), this, SLOT(Accept()));
    layout->addWidget(setButton);

    //apply the layout
    setLayout(layout);

    if(m_radioGroup->buttons().size() < 2) {
      setVisible(false);
      QTimer::singleShot(0, this, SLOT(Accept()));
    }
  }

string
RegionSamplerDialog::
GetSampler() {
  if(m_radioGroup->buttons().size()) {
    string temp = m_radioGroup->checkedButton()->text().toStdString();
    return temp.substr(temp.rfind(':') + 1);
  }
  else
    return "";
}

void
RegionSamplerDialog::
Accept() {
  // Set the sampler that the selelcted region will use
  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  RegionModel* r = (RegionModel*)sel[0];
  r->SetSampler(GetSampler());

  accept();
}
