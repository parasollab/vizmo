#include "ChangeBoundaryDialog.h"

#include <iostream>

#include "MainWindow.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

ChangeBoundaryDialog::ChangeBoundaryDialog(QWidget* _parent) : QDialog(_parent) {

  resize(323, 196);
  setWindowTitle("Change Boundary");
  QPushButton* cancelButton = new QPushButton("Cancel", this);
  QPushButton* loadButton = new QPushButton("OK", this);
  m_boxButton = new QRadioButton("Bounding Box", this);
  m_sphereButton = new QRadioButton("Bounding Sphere", this);
  m_lineBoxX1 = new QLineEdit("-25", this);
  m_lineBoxX2 = new QLineEdit("25", this);
  m_lineBoxY1 = new QLineEdit("-15", this);
  m_lineBoxY2 = new QLineEdit("15", this);
  m_lineBoxZ1 = new QLineEdit("-25", this);
  m_lineBoxZ2 = new QLineEdit("25", this);
  m_lineSphereX = new QLineEdit("0", this);
  m_lineSphereY = new QLineEdit("0", this);
  m_lineSphereZ = new QLineEdit("0", this);
  m_lineSphereR = new QLineEdit("25", this);
  m_labelBoxX = new QLabel("<b>X<b>", this);
  m_labelBoxY = new QLabel("<b>Y<b>", this);
  m_labelBoxZ = new QLabel("<b>Z<b>", this);
  m_labelSphereX = new QLabel("<b>X<b>", this);
  m_labelSphereY = new QLabel("<b>Y<b>", this);
  m_labelSphereZ = new QLabel("<b>Z<b>", this);
  m_labelSphereR = new QLabel("<b>Radius<b>", this);
  m_checkIsSurface = new QCheckBox("Surface Boundary", this);
  QValidator* validator = new QDoubleValidator();

  m_isBox = true;
  m_isSurface = false;

  connect(loadButton, SIGNAL(clicked()), this, SLOT(SetBoundary()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
  connect(m_sphereButton, SIGNAL(clicked()), this, SLOT(ChangeToSphereDialog()));
  connect(m_boxButton, SIGNAL(clicked()), this, SLOT(ChangeToBoxDialog()));
  connect(m_checkIsSurface, SIGNAL(clicked()), this, SLOT(ChangeToSurface()));

  loadButton->setGeometry(QRect(210,150,95,31));
  cancelButton->setGeometry(QRect(100,150,95,31));
  m_lineBoxX1->setValidator(validator);
  m_lineBoxX2->setValidator(validator);
  m_lineBoxY1->setValidator(validator);
  m_lineBoxY2->setValidator(validator);
  m_lineBoxZ1->setValidator(validator);
  m_lineBoxZ2->setValidator(validator);
  m_lineSphereX->setValidator(validator);
  m_lineSphereY->setValidator(validator);
  m_lineSphereZ->setValidator(validator);
  m_lineSphereR->setValidator(validator);
  SetUpLayout();
}

void
ChangeBoundaryDialog::SetUpLayout(){
  m_boxButton->setGeometry(QRect(10,10,124,26));
  m_boxButton->setChecked(true);
  m_sphereButton->setGeometry(QRect(170,10,151,26));
  m_lineBoxX1->setGeometry(QRect(20,100,41,31));
  m_lineBoxX2->setGeometry(QRect(70,100,41,31));
  m_lineBoxY1->setGeometry(QRect(120,100,41,31));
  m_lineBoxY2->setGeometry(QRect(170,100,41,31));
  m_lineBoxZ1->setGeometry(QRect(220,100,41,31));
  m_lineBoxZ2->setGeometry(QRect(270,100,41,31));
  m_labelBoxX->setGeometry(QRect(60,70,21,21));
  m_labelBoxY->setGeometry(QRect(160,70,21,21));
  m_labelBoxZ->setGeometry(QRect(260,70,21,21));
  m_lineSphereX->setGeometry(QRect(30,100,41,31));
  m_lineSphereY->setGeometry(QRect(100,100,41,31));
  m_lineSphereZ->setGeometry(QRect(180,100,41,31));
  m_lineSphereR->setGeometry(QRect(250,100,41,31));
  m_labelSphereX->setGeometry(QRect(40,70,21,21));
  m_labelSphereY->setGeometry(QRect(110,70,21,21));
  m_labelSphereZ->setGeometry(QRect(190,70,21,21));
  m_labelSphereR->setGeometry(QRect(240,70,51,21));
  m_checkIsSurface->setGeometry(QRect(80,40,141,26));
  DisplaySphereItems(false);
  ShowCurrentValues();
  QDialog::show();
}

void
ChangeBoundaryDialog::ShowCurrentValues(){
  string type = GetVizmo().GetEnv()->GetBoundaryType();
  stringstream streamCoord;
  streamCoord << *GetVizmo().GetEnv()->GetBoundary();
  string word;
  string values[6]={"0","0","0","0","0","0"};
  vector<string> tripleLimits;
  int i=0;
  if(type=="BOX"){
    ChangeToBoxDialog();
    while(getline(streamCoord, word, ' ')){
      if(word!="")
        tripleLimits.push_back(word);
    }
    for(vector<string>::iterator it=tripleLimits.begin(); it!=tripleLimits.end(); it++){
      stringstream streamLimits(*it);
      while(getline(streamLimits, word, ':')){
        values[i]=word;
        i++;
      }
    }
    m_lineBoxX1->setText(values[0].c_str());
    m_lineBoxX2->setText(values[1].c_str());
    m_lineBoxY1->setText(values[2].c_str());
    m_lineBoxY2->setText(values[3].c_str());
    m_lineBoxZ1->setText(values[4].c_str());
    m_lineBoxZ2->setText(values[5].c_str());
  }
  else{
    ChangeToSphereDialog();
    while(getline(streamCoord, word, ' ')){
      if(word!=""){
        values[i]=word;
        i++;
      }
    }
    m_lineSphereX->setText(values[0].c_str());
    m_lineSphereY->setText(values[1].c_str());
    m_lineSphereZ->setText(values[2].c_str());
    m_lineSphereR->setText(values[3].c_str());
  }
}

void
ChangeBoundaryDialog::ChangeToSurface(){
  m_isSurface=m_checkIsSurface->isChecked();
  m_lineBoxZ1->setEnabled(!m_isSurface);
  m_lineBoxZ2->setEnabled(!m_isSurface);
}

void
ChangeBoundaryDialog::ChangeToSphereDialog(){
  m_isBox = false;
  m_isSurface = false;
  m_checkIsSurface->setVisible(false);
  m_checkIsSurface->setChecked(false);
  m_lineBoxZ1->setEnabled(true);
  m_lineBoxZ2->setEnabled(true);
  m_boxButton->setChecked(false);
  m_sphereButton->setChecked(true);
  DisplayBoxItems(false);
  DisplaySphereItems(true);
}


void
ChangeBoundaryDialog::ChangeToBoxDialog(){
  m_isBox = true;
  m_checkIsSurface->setVisible(true);
  m_boxButton->setChecked(true);
  m_sphereButton->setChecked(false);
  DisplayBoxItems(true);
  DisplaySphereItems(false);
}

void
ChangeBoundaryDialog::DisplaySphereItems(bool _i){
  m_lineSphereX->setVisible(_i);
  m_lineSphereY->setVisible(_i);
  m_lineSphereZ->setVisible(_i);
  m_lineSphereR->setVisible(_i);
  m_labelSphereX->setVisible(_i);
  m_labelSphereY->setVisible(_i);
  m_labelSphereZ->setVisible(_i);
  m_labelSphereR->setVisible(_i);
}

void
ChangeBoundaryDialog::DisplayBoxItems(bool _i){
  m_lineBoxX1->setVisible(_i);
  m_lineBoxX2->setVisible(_i);
  m_lineBoxY1->setVisible(_i);
  m_lineBoxY2->setVisible(_i);
  m_lineBoxZ1->setVisible(_i);
  m_lineBoxZ2->setVisible(_i);
  m_labelBoxX->setVisible(_i);
  m_labelBoxY->setVisible(_i);
  m_labelBoxZ->setVisible(_i);
}

void
ChangeBoundaryDialog::SetBoundary(){
  EnvModel* env = GetVizmo().GetEnv();
  string line="";
  string type="";
  if(m_isBox==true){
    string boxCoord[6]={"0","0","0","0","0","0"};
    if(m_lineBoxX1->text().toStdString() !="")
      boxCoord[0]=m_lineBoxX1->text().toStdString();
    if(m_lineBoxX2->text().toStdString() !="")
      boxCoord[1]=m_lineBoxX2->text().toStdString();
    if(m_lineBoxY1->text().toStdString() !="")
      boxCoord[2]=m_lineBoxY1->text().toStdString();
    if(m_lineBoxY2->text().toStdString() !="")
      boxCoord[3]=m_lineBoxY2->text().toStdString();
    if(m_lineBoxZ1->text().toStdString() !="")
      boxCoord[4]=m_lineBoxZ1->text().toStdString();
    if(m_lineBoxZ2->text().toStdString() !="")
      boxCoord[5]=m_lineBoxZ2->text().toStdString();
    int size = 2+1*(!m_isSurface);
    for(int i=0;i<size;i++)
      line += (boxCoord[2*i]+":"+boxCoord[2*i+1]+" ");
    type="BOX";
  }
  else{
    string sphereCoord[4]={"0","0","0","0"};
    if(m_lineSphereX->text().toStdString() !="")
      sphereCoord[0]=m_lineSphereX->text().toStdString();
    if(m_lineSphereY->text().toStdString() !="")
      sphereCoord[1]=m_lineSphereY->text().toStdString();
    if(m_lineSphereZ->text().toStdString() !="")
      sphereCoord[2]=m_lineSphereZ->text().toStdString();
    if(m_lineSphereR->text().toStdString() !="")
      sphereCoord[3]=m_lineSphereR->text().toStdString();
    for(int i=0;i<4;i++)
      line += (sphereCoord[i]+" ");
    type="SPHERE";
  }
  istringstream coord(line);
  env->ChangeBoundary(type, coord);
  accept();
}
