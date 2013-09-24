/*****************************************************************
 * A custom dialog for adjusting roadmap NODE SIZE with a slider
 * **************************************************************/

#ifndef NODE_SIZE_DIALOG_H
#define NODE_SIZE_DIALOG_H

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

class NodeSizeDialog : public QDialog {

  Q_OBJECT

  public:
    NodeSizeDialog(QWidget* _parent = 0, RoadmapOptions* _accessParent = 0);

  private slots:
    void ResizeNodes(); //Scale the nodes live while dialog open

  private:
    void SetUpDialog(QDialog* _dialog);
    void Reset(); //Restore default when new file is opened
    QDialogButtonBox* m_okayCancel;
    QSlider* m_slider;
    QLabel* m_instructions;
    QLabel* m_value;
    RoadmapOptions* m_parent;

};

#endif
