#include <Q3Grid>
#include <Q3HBox>
#include <Q3MultiLineEdit>
#include <Q3ListBox>
#include <QLabel>
#include <QLineEdit>   
#include <QDialog> 
#include <QVBoxLayout> 
#include <QIntValidator> 
#include <QPushButton> 
#include <QFileDialog> 
#include <string> 

#include "MovieSaveDialog.h"

inline QStringList& Filters(){

  static QStringList filters;

  filters+="JPEG (*.jpg)";
  filters+="GIF (*.gif)";
  filters+="Encapsulated PostScript file (*.eps)";
  filters+="targa (*.tga)";
  filters+="Portable Network Graphics (*.png)";
  filters+="Microsoft Bitmap (*.bmp)";
  filters+="Computer Graphics Metafile (*.cgm)";
  filters+="PostScript Interchange (*.epi)";
  filters+="Microsoft Icon (*.ico)";
  filters+="Paintbrush File (*.pcx)";
  filters+="Portable Pixmap (*.ppm)";
  filters+="Adobe Portable Document Format (*.pdf)";

  return filters;
}

inline string Filter2Ext(const string filter){

  if(filter.find("jpg")!=string::npos)
    return ".jpg";
  if(filter.find("gif")!=string::npos)
    return ".gif";
  if(filter.find("eps")!=string::npos)
    return ".eps";
  if(filter.find("tga")!=string::npos)
    return ".tga";
  if(filter.find("png")!=string::npos)
    return ".png";
  if(filter.find("pdf")!=string::npos)
    return ".pdf";
  if(filter.find("bmp")!=string::npos)
    return ".bmp";
  if(filter.find("epi")!=string::npos)
    return ".epi";
  if(filter.find("cgm")!=string::npos)
    return ".cgm";
  if(filter.find("ico")!=string::npos)
    return ".ico";
  if(filter.find("pcx")!=string::npos)
    return ".pcx";
  if(filter.find("ppm")!=string::npos)
    return ".ppm";
  if(filter.find("pdf")!=string::npos)
    return ".pdf";
  return "";
}

MovieSaveDialog::MovieSaveDialog(QWidget *parent, const char *name, Qt::WFlags f)
  :QDialog(parent,f)
{
  m_sFileName="vizmo_movie";
  m_sFileExt="jpg";
  m_frameDigit=5;

  QVBoxLayout* vbox = new QVBoxLayout(this ,8);
  vbox->setAutoAdd(TRUE);
  Q3Grid* controls = new Q3Grid(2,Qt::Horizontal,this);
  controls->setSpacing(8);
  QLabel* l;

  l = new QLabel("StartFrame", controls); 
  l->setAlignment(Qt::AlignCenter);
  m_startFrame = new QLineEdit(controls);
  m_startFrame->setValidator(new QIntValidator(m_startFrame));

  l = new QLabel("EndFrame", controls); 
  l->setAlignment(Qt::AlignCenter);
  m_endFrame = new QLineEdit(controls);
  m_endFrame->setValidator(new QIntValidator(m_endFrame));

  l = new QLabel("Step Size", controls); 
  l->setAlignment(Qt::AlignCenter);
  m_stepSize = new QLineEdit(controls);
  m_stepSize->setValidator(new QIntValidator(m_stepSize));

  QPushButton* fileNameButton = new QPushButton("Select Name", controls);
  m_fnameLabel=new QLabel((m_sFileName+"#####." + m_sFileExt), controls);
  l->setAlignment(Qt::AlignCenter);

  QPushButton* go = new QPushButton("Go", controls);
  QPushButton* cancel = new QPushButton("Cancel", controls);

  connect(fileNameButton, SIGNAL(clicked()), this, SLOT(ShowFileDialog()));
  connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(go, SIGNAL(clicked()), this, SLOT(SaveImages()));
}

void
MovieSaveDialog::SaveImages(){

  StoreAttributes(); 
  QDialog::accept(); 
}

void MovieSaveDialog::UpdateAttributes(){

  QString tempStorage;

  tempStorage.setNum(m_startIntFrame);
  m_startFrame->setText(tempStorage);

  tempStorage.setNum(m_endIntFrame);
  m_endFrame->setText(tempStorage);

  tempStorage.setNum(m_stepIntSize);
  m_stepSize->setText(tempStorage);
}

void 
MovieSaveDialog::StoreAttributes(){

  QString tempStorage;
  bool conv;

  tempStorage = m_startFrame->text();
  m_startIntFrame = tempStorage.toInt(&conv, 10);

  tempStorage = m_endFrame->text();
  m_endIntFrame = tempStorage.toInt(&conv, 10);

  tempStorage = m_stepSize->text();
  m_stepIntSize = tempStorage.toInt(&conv, 10);    
}

void 
MovieSaveDialog::ShowFileDialog(){

  QFileDialog* fd = new QFileDialog(this, "File name", ".", QString::null);
  fd->setFileMode(QFileDialog::AnyFile);  
  fd->setFilters(Filters());
  if(fd->exec() == QDialog::Accepted){
    m_sFileName = fd->selectedFile(); 
    m_sFileExt = Filter2Ext(fd->selectedFilter().toStdString()).c_str(); 
    m_fnameLabel->setText(m_sFileName + m_sFileExt); 

    //find digit
    int _s = m_sFileName.find('#');
    int _g = m_sFileName.findRev('#');
    m_frameDigit=_g-_s;
    if(_s == _g) 
      m_frameDigit=4;
    m_sFileName = m_sFileName.left(_s);
  }
}












