#include "RegionSamplerDialog.h"

#include "Models/Vizmo.h"
#include "Models/CfgModel.h"
#include "Models/EdgeModel.h"
#include "Models/MapModel.h"
#include "Utilities/AlertUser.h"

RegionSamplerDialog::
RegionSamplerDialog(const vector<string>& _samplers, QWidget* _parent, Qt::WFlags _f) :
  QDialog(_parent, _f), m_samplerLabel("") {
    SetUpSubwidgets(_samplers);
  }

void
RegionSamplerDialog::
SetUpSubwidgets(const vector<string>& _samplers) {

  QGridLayout* layout = new QGridLayout;
  setLayout(layout);

  QPushButton* setButton = new QPushButton("Set", this);
  //QPushButton* cancelButton = new QPushButton("Cancel", this);

  for(vector<string>::const_iterator citer = _samplers.begin(); citer != _samplers.end(); ++citer) {
    m_samplerRadioButtons.push_back(new QRadioButton(this));
    m_nameAndLabels.push_back(new QLabel(GetVizmo().GetSamplerNameAndLabel((*citer).c_str()).c_str(), this));
    m_samplerLabels.push_back((*citer).c_str());
  }

  m_samplerRadioButtons[0]->setChecked(true);
  m_samplerLabel = m_samplerLabels[0];

  for(size_t i = 0; i < _samplers.size(); ++i) {
    layout->addWidget(m_samplerRadioButtons[i], i, 0);
    layout->addWidget(m_nameAndLabels[i], i, 1);

    connect(m_samplerRadioButtons[i], SIGNAL(clicked()), this, SLOT(ChangeSampler()));
  }

  //setButton->setFixedWidth(cancelButton->width());
  layout->addWidget(setButton, _samplers.size(), 1);
  //layout->addWidget(cancelButton, _samplers.size(), 2);

  connect(setButton, SIGNAL(clicked()), this, SLOT(Accept()));
  //connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
}

void
RegionSamplerDialog::
ChangeSampler() {
  for(size_t i = 0; i < m_samplerRadioButtons.size(); ++i) {
    if(m_samplerRadioButtons[i]->isChecked()) {
      m_samplerLabel = m_samplerLabels[i];
    }
  }
}

void
RegionSamplerDialog::
Accept() {
  // Set the sampler that the selelcted region will use
  vector<Model*>& sel = GetVizmo().GetSelectedModels();

  RegionModel* r = (RegionModel*)sel[0];
  r->SetSampler(m_samplerLabel);
  accept();
}
