#include "CameraPosDialog.h" 
#include "Utilities/GL/gliCamera.h" 

using namespace std; 

CameraPosDialog::CameraPosDialog(QWidget* _parent)
  :QDialog(_parent) {
  SetUpDialog(this); 
}

void 
CameraPosDialog::AcceptData(){ 

  double x = (m_xLineEdit->text()).toDouble();  
  double y = (m_yLineEdit->text()).toDouble();  
  double z = (m_zLineEdit->text()).toDouble(); 
  
  double azim = (m_azimLineEdit->text()).toDouble(); 
  double elev = (m_elevLineEdit->text()).toDouble(); 

  gliGetCameraFactory().getCurrentCamera()->SetByUser(x, y, z, azim, elev); 

  accept(); 
}






