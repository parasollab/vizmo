#ifndef REGION_SAMPLER_DIALOG_H_
#define REGION_SAMPLER_DIALOG_H_

#include <QtGui>
#include <QDialog>

class QWidget;
class QButtonGroup;

class RegionSamplerDialog : public QDialog {

  Q_OBJECT

  public:
    RegionSamplerDialog(QWidget* _parent);

  private slots:
    void Accept();

  private:
    std::string GetSampler();

    QButtonGroup* m_radioGroup;
};

#endif
