#include "ObstaclePosDialog.h"

#include <iostream>
#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QValidator>
#include <QSlider>

#include "Transformation.h"
#include "MainWindow.h"
#include "GLWidget.h"
#include "Models/Vizmo.h"
#include "Utilities/IOUtils.h"
#include "Models/PolyhedronModel.h"

ObstaclePosDialog::ObstaclePosDialog(MultiBodyModel* _multiBody, MainWindow* _mainWindow,
    QWidget* _parent) : QDialog(_parent) {
  resize(315, 307);
  setWindowTitle("Obstacle Position");
  m_loadButton = new QPushButton("OK", this);
  m_xPosLabel = new QLabel("x", this);
  m_yPosLabel = new QLabel("y", this);
  m_zPosLabel = new QLabel("z", this);
  m_alphaLabel = new QLabel(QChar(0x03B1), this);
  m_betaLabel = new QLabel(QChar(0x03B2), this);
  m_gammaLabel = new QLabel(QChar(0x03B3), this);
  for(int i=0; i<6; i++)
    m_sliders[i] = new QSlider(this);
  for(int i=0; i<6; i++)
    m_posLines[i] = new QLineEdit("0", this);
  m_rotLabel = new QLabel("<b>Rotation<b>", this);
  m_coordLabel = new QLabel("<b>Coordinates<b>", this);
  m_validator = new QDoubleValidator();

  connect(m_loadButton, SIGNAL(clicked()), this, SLOT(accept()));
  for(int i=0; i<6; i++){
    connect(m_sliders[i], SIGNAL(valueChanged(int)), this, SLOT(DisplaySlidersValues(int)));
    connect(m_posLines[i], SIGNAL(editingFinished()), this, SLOT(ChangeSlidersValues()));
  }
  m_mainWindow=_mainWindow;
  m_multiBody=_multiBody;
  m_obstacle=m_multiBody->GetBodies().back();
  m_valueEdited=false;
  GetBoundingValues();
  SetSlidersInit();
  SetUpLayout();
}

void
ObstaclePosDialog::SetUpLayout(){
  m_loadButton->setGeometry(QRect(170,260,91,31));
  m_loadButton->setAutoDefault(false);
  m_xPosLabel->setGeometry(QRect(20,40,16,21));
  m_yPosLabel->setGeometry(QRect(20,70,16,21));
  m_zPosLabel->setGeometry(QRect(20,100,16,21));
  m_alphaLabel->setGeometry(QRect(20,160,16,21));
  m_betaLabel->setGeometry(QRect(20,190,16,21));
  m_gammaLabel->setGeometry(QRect(20,220,16,21));
  m_rotLabel->setGeometry(QRect(20,130,91,21));
  m_coordLabel->setGeometry(QRect(20,10,91,21));
  for(int i=0; i<6; i++)
    m_sliders[i]->setOrientation(Qt::Horizontal);
  for(int i=0; i<3; i++){
    m_sliders[i]->setGeometry(QRect(40, 40+i*30, 201, 19));
    m_sliders[i+3]->setGeometry(QRect(40, 160+i*30, 201, 19));
    m_posLines[i]->setValidator(m_validator);
    m_posLines[i+3]->setValidator(m_validator);
    m_posLines[i]->setGeometry(QRect(260,40+i*30,41,21));
    m_posLines[i+3]->setGeometry(QRect(260,160+i*30,41,21));
 }
  QDialog::show();
}

void
ObstaclePosDialog::SetSlidersInit(){
  for(int i=0; i<3; i++){
    m_sliders[i]->setMinimum(m_boundingValues[i].first);
    m_sliders[i]->setMaximum(m_boundingValues[i].second);
  }
  for(int i=3; i<6; i++){
    m_sliders[i]->setMinimum(-180);
    m_sliders[i]->setMaximum(180);
  }
  ostringstream transform;
  transform<<m_obstacle->GetTransform();
  string transformString=transform.str();
  istringstream splitTransform(transformString);
  string splittedTransform[6]={"","","","","",""};
  int j=0;
  do{
    splitTransform>>splittedTransform[j];
    j++;
  }while(splitTransform);
  string temp;
  temp=splittedTransform[3];
  splittedTransform[3]=splittedTransform[5];
  splittedTransform[5]=temp;
  for(int i=0; i<6; i++)
    m_sliders[i]->setValue(atoi(splittedTransform[i].c_str()));
}

void
ObstaclePosDialog::DisplaySlidersValues(int _i ){
  if(!m_valueEdited){
    for(int i=0;i<6; i++){
      if(m_sliders[i]->value()==_i){
        stringstream ss;
        ss<<_i;
        m_posLines[i]->setText(ss.str().c_str());
        RefreshPosition();
      }
    }
  }
}

void
ObstaclePosDialog::ChangeSlidersValues(){
  m_valueEdited=true;
  for(int i=0; i<6; i++){
    int val=atoi(m_posLines[i]->text().toStdString().c_str());
    if(val!=m_sliders[i]->value())
      m_sliders[i]->setValue(val);
      RefreshPosition();
  }
  m_valueEdited=false;
}

void
ObstaclePosDialog::GetBoundingValues(){
  string type = GetVizmo().GetEnv()->GetBoundaryType();
  string coord = GetVizmo().GetEnv()->GetBoundary()->GetCoord();
  stringstream streamCoord(coord);
  string word;
  string values[6]={"0","0","0","0","0","0"};
  vector<string> tripleLimits;
  int i=0;
  if(type=="BOX"){
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
    for(int j=0; j<3; j++){
      m_boundingValues[j]=std::make_pair(atoi(values[2*j].c_str()),atoi(values[2*j+1].c_str()));
    }
  }
  else{
    while(getline(streamCoord, word, ' ')){
      if(word!=""){
        values[i]=word;
        i++;
      }
    }
    for(int j=0; j<3; j++){
      m_boundingValues[j]=std::make_pair(-atoi(values[3].c_str()),atoi(values[3].c_str()));
    }
  }
}

string
ObstaclePosDialog::GetCoord(){
  string position[6]={"0","0","0","0","0","0"};
  string stringCoord;
  for(int i=0; i<6; i++){
    if(m_posLines[i]->text().toStdString() !="")
      position[i]=m_posLines[i]->text().toStdString();
    stringCoord+=(position[i]+" ");
  }
  return stringCoord;
}

void
ObstaclePosDialog::RefreshPosition(){
  istringstream coord(GetCoord());
  Transformation transform;
  transform = ReadField<Transformation>(coord, WHERE, "failed reading new transformation");
  const Transformation &t=transform;
  m_obstacle->SetTransform(t);
  m_obstacle->UpdateModel();
  m_mainWindow->GetGLScene()->updateGL();
}

