#ifndef REGION_SAMPLER_DIALOG_H_
#define REGION_SAMPLER_DIALOG_H_

using namespace std;

#include <QtGui>

class RegionSamplerDialog : public QDialog {

  Q_OBJECT

  public:
    RegionSamplerDialog(const vector<string>& _samplers, QWidget* _parent, Qt::WFlags _f = 0);

  private slots:
    void ChangeSampler();
    void Accept();

  private:
    void SetUpSubwidgets(const vector<string>& _samplers);

    vector<QRadioButton*> m_samplerRadioButtons;
    vector<QLabel*> m_nameAndLabels;
    vector<string> m_samplerLabels;

    string m_samplerLabel;
};

#endif
