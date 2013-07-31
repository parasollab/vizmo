/*****************************************************************
 * A custom dialog for adjusting roadmap NODE SIZE with a slider
 * **************************************************************/

#ifndef SIZE_SLIDER_DIALOG_H
#define SIZE_SLIDER_DIALOG_H

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QSlider>

#include "RoadmapOptions.h"

using namespace std;

class RoadmapOptions;

class SizeSliderDialog : public QDialog{

  Q_OBJECT

  string m_mode;
  int m_oldValue;

  public:
    SizeSliderDialog(string _mode, QWidget* _parent = 0, RoadmapOptions* _accessParent = 0);

    QDialogButtonBox* m_okayCancel;
    QSlider* m_slider;
    QLabel* m_instructions;
    QLabel* m_value;
    RoadmapOptions* m_parent;

    void SetUpDialog(QDialog* _dialog);
    void Reset(); //Restore default when new file is opened

  public slots:
    virtual void show();        //Overriden to save initial value
    void ResizeNodes();         //If node mode, scale the nodes live while dialog open
    void ChangeEdgeThickness(); //If edge mode, scale the edges
    virtual void reject();      //Overriden to restore previous size upon pressing cancel
};

#endif




























