#include "RegionSamplerDialog.h"

#include "Models/Vizmo.h"
#include "Models/CfgModel.h"
#include "Models/EdgeModel.h"
#include "Models/MapModel.h"

RegionSamplerDialog::
RegionSamplerDialog(const vector<string>& _samplers, QWidget* _parent) :
    QDialog(_parent), m_samplerLabel("") {
  //create widget layout
  QVBoxLayout* layout = new QVBoxLayout(this);

  //create radio buttons
  for(vector<string>::const_iterator citer = _samplers.begin();
      citer != _samplers.end(); ++citer) {
    QRadioButton* button = new QRadioButton((*citer).c_str(), this);
    connect(button, SIGNAL(clicked()), this, SLOT(ChangeSampler()));
    layout->addWidget(button);
    //set first button checked by default
    if(citer == _samplers.begin())
      button->setChecked(true);
  }

  //create accept button
  QPushButton* setButton = new QPushButton("Set", this);
  connect(setButton, SIGNAL(clicked()), this, SLOT(Accept()));
  layout->addWidget(setButton);

  //apply the layout
  setLayout(layout);
}

void
RegionSamplerDialog::
ChangeSampler() {
  QRadioButton* button = static_cast<QRadioButton*>(sender());
  m_samplerLabel = button->text().toStdString();
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
