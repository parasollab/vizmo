#ifndef REGION_SAMPLER_DIALOG_H_
#define REGION_SAMPLER_DIALOG_H_

using namespace std;

#include <QtGui>

class RegionSamplerDialog : public QDialog {

  Q_OBJECT

  public:
    RegionSamplerDialog(const vector<string>& _samplers, QWidget* _parent);

  private slots:
    void ChangeSampler();
    void Accept();

  private:
    string m_samplerLabel;
};

#endif
