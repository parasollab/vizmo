/* Implementation of node, edge, etc(?) scaling dialog with slider */

//TODO: Possibly just make SizeSlider a base class due to the repeated code, or
//take constructor arguments to customize range, etc. for node or edge.

#include "SizeSliderDialog.h"

#include "SceneWin.h"
#include "Models/Vizmo.h"

SizeSliderDialog::SizeSliderDialog(string _mode, QWidget* _parent, RoadmapOptions* _accessParent)
  :QDialog(_parent){

  m_parent = _accessParent;
  m_mode = _mode;
  SetUpDialog(this);
  m_oldValue = m_slider->value();
}

void
SizeSliderDialog::SetUpDialog(QDialog* _dialog){

  _dialog->resize(477, 139);

  if(m_mode == "node")
    _dialog->setWindowTitle(tr("Node Scaling"));
  if(m_mode == "edge")
    _dialog->setWindowTitle(tr("Edge Thickness"));

  m_okayCancel = new QDialogButtonBox(_dialog);
  m_okayCancel->setGeometry(QRect(290, 100, 181, 32));
  m_okayCancel->setOrientation(Qt::Horizontal);
  m_okayCancel->setStandardButtons(QDialogButtonBox::Cancel
    |QDialogButtonBox::Ok);

  m_slider = new QSlider(_dialog);
  m_slider->setGeometry(QRect(19, 60, 441, 20));
  m_slider->setOrientation(Qt::Horizontal);
  //QSlider only works on ints, so value will be divided to map for actual scaling
  //Large range ensures smooth mouse movement
  if(m_mode == "node")
    m_slider->setRange(0, 2500);
  if(m_mode == "edge")
    m_slider->setRange(100, 1000);

  if(m_mode == "node")
    m_slider->setValue(1000);
  if(m_mode == "edge")
    m_slider->setValue(1);

  if(m_mode == "node")
    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(ResizeNodes()));
  if(m_mode == "edge")
    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(ChangeEdgeThickness()));

  m_instructions = new QLabel(_dialog);
  m_instructions->setGeometry(QRect(20, 20, 311, 17));

  if(m_mode == "node")
    m_instructions->setText(tr("Drag the slider to scale the nodes"));
  if(m_mode == "edge")
    m_instructions->setText(tr("Drag the slider to change the edge thickness"));

  m_value = new QLabel(_dialog);
  m_value->setGeometry(QRect(20, 90, 70, 17)); //long width for plenty of room
  m_value->setText("100%");

  connect(m_okayCancel, SIGNAL(accepted()), _dialog, SLOT(accept()));
  connect(m_okayCancel, SIGNAL(rejected()), _dialog, SLOT(reject()));

  QMetaObject::connectSlotsByName(_dialog);
}

void
SizeSliderDialog::show(){

  m_oldValue = m_slider->value();
  QDialog::show();
}

void
SizeSliderDialog::ResizeNodes(){

    double resize = ((double)m_slider->value()) / (double)1000;
    ostringstream oss;
    oss << (resize*100) << "%";
    QString qs((oss.str()).c_str());
    m_value->setText(qs);
    //Resize the other mode as well to prevent abrupt size change when switching
    //back and using the slider again
    if(m_parent->GetNodeShape() == "Point"){
      GetVizmo().ChangeNodesSize(resize, "Box");
      GetVizmo().ChangeNodesSize(resize, "Point");
    }
    else if(m_parent->GetNodeShape() == "Box"){
      GetVizmo().ChangeNodesSize(resize, "Point");
      GetVizmo().ChangeNodesSize(resize, "Box");
    }
    m_parent->GetMainWin()->GetGLScene()->updateGL();
}

void
SizeSliderDialog::ChangeEdgeThickness(){

    double resize = ((double)m_slider->value()) / (double)100;
    ostringstream oss;
    oss << (resize*100) << "%";
    QString qs((oss.str()).c_str());
    m_value->setText(qs);

    GetVizmo().ChangeEdgeThickness(resize);
    m_parent->GetMainWin()->GetGLScene()->updateGL();
}

void
SizeSliderDialog::reject(){

  m_slider->setSliderPosition(m_oldValue);
  QDialog::reject();
}

void
SizeSliderDialog::Reset(){

  if(m_mode == "node")
    m_slider->setSliderPosition(1000);
  if(m_mode == "edge")
    m_slider->setSliderPosition(0);
}





