/*****************************************************************
 * A slider dialog with customizable parameters. Useful for size
 * changes and possibly things like colors...
 * **************************************************************/

#ifndef SLIDER_DIALOG_H
#define SLIDER_DIALOG_H

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QSlider>
#include <QString>

using namespace std;

class SliderDialog : public QDialog {

  Q_OBJECT

  public:
    SliderDialog(QString _windowTitle, QString _instrustions,
        int _rangeStart, int _rangeEnd, int _startValue,
        QWidget* _parent = NULL);

    QSlider* GetSlider() { return m_slider; }
    double GetSliderValue() { return m_slider->value(); }
    virtual void show();

    void Reset();

  private:
    QDialogButtonBox* m_okayCancel;
    QSlider* m_slider;
    QLabel* m_instructions;
    QLabel* m_value;
    int m_startValue; //For percent calculation
    int m_oldValue;

  private slots:
    virtual void reject();      //Overriden to restore previous size upon pressing cancel
    void UpdatePercentile();    //Display value text as slider is moved
};

#endif
