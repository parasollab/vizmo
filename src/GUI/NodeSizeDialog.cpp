/* Implementation of node scaling dialog with slider */

#include "NodeSizeDialog.h"
#include "vizmo2.h"
#include "SceneWin.h"

using namespace std;

NodeSizeDialog::NodeSizeDialog(QWidget* _parent, RoadmapOptions* _accessParent)
  :QDialog(_parent){

  m_parent = _accessParent;
  SetUpDialog(this);
}

void
NodeSizeDialog::SetUpDialog(QDialog* _dialog){

  _dialog->resize(477, 139);
  _dialog->setWindowTitle(tr("Node Scaling"));

  m_okayCancel = new QDialogButtonBox(_dialog);
  m_okayCancel->setGeometry(QRect(290, 100, 181, 32));
  m_okayCancel->setOrientation(Qt::Horizontal);
  m_okayCancel->setStandardButtons(QDialogButtonBox::Cancel
    |QDialogButtonBox::Ok);

  m_slider = new QSlider(_dialog);
  m_slider->setGeometry(QRect(19, 60, 441, 20));
  m_slider->setOrientation(Qt::Horizontal);
  //QSlider only works on ints, so value will be divided to map for actual scaling
  m_slider->setRange(0, 10000);
  m_slider->setSliderPosition(1000);
  connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(ResizeNodes()));

  m_instructions = new QLabel(_dialog);
  m_instructions->setGeometry(QRect(20, 20, 311, 17));
  m_instructions->setText(tr("Drag the slider to scale the nodes"));

  m_value = new QLabel(_dialog);
  m_value->setGeometry(QRect(20, 90, 70, 17)); //long width for plenty of room
  m_value->setText("100%");

  connect(m_okayCancel, SIGNAL(accepted()), _dialog, SLOT(accept()));
  connect(m_okayCancel, SIGNAL(rejected()), _dialog, SLOT(reject()));

  QMetaObject::connectSlotsByName(_dialog);
}

void
NodeSizeDialog::ResizeNodes(){

    double resize = ((double)m_slider->value()) / (double)1000;
    ostringstream oss;
    oss << (resize*100) << "%";
    QString qs((oss.str()).c_str());
    m_value->setText(qs);
    //Make sure correct shape appears: choose order based on current mode
    //Probably inefficient...but not detectably slow
    if(m_parent->GetNodeShape() == "Robot"){
      GetVizmo().ChangeNodesSize(resize, "Box");
      GetVizmo().ChangeNodesSize(resize, "Robot");
    }
    else if(m_parent->GetNodeShape() == "Box"){
      GetVizmo().ChangeNodesSize(resize, "Robot");
      GetVizmo().ChangeNodesSize(resize, "Box");
    }
    m_parent->GetMainWin()->GetGLScene()->updateGL();
}

void
NodeSizeDialog::Reset(){

  m_slider->setSliderPosition(1000);
}
