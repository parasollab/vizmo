#include <QGridLayout> 
#include <QLabel>
#include <QLineEdit>   
#include <QDialog> 
#include <QIntValidator> 
#include <QPushButton> 
#include <QFileDialog> 
#include <string> 

#include "MovieSaveDialog.h"

inline QStringList& 
Filters(){

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

inline string 
Filter2Ext(const string filter){

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

MovieSaveDialog::MovieSaveDialog(QWidget* _parent, Qt::WFlags _f)
  :QDialog(_parent, _f)
{
  m_sFileName = "vizmo_movie";
  m_sFileExt = "jpg";
  m_frameDigit = 5;
  
  //1. Create subwidgets/members 
  m_startFrameLabel = new QLabel("Start Frame", this); 
  m_startFrameEdit = new QLineEdit(this);
  m_startFrameEdit->setValidator(new QIntValidator(this));   

  m_endFrameLabel = new QLabel("EndFrame", this); 
  m_endFrameEdit = new QLineEdit(this);
  m_endFrameEdit->setValidator(new QIntValidator(this));

  m_stepSizeLabel = new QLabel("Step Size", this); 
  m_stepSizeEdit = new QLineEdit(this);
  m_stepSizeEdit->setValidator(new QIntValidator(this));

  m_selectNameButton = new QPushButton("Select Name", this);
  m_fileNameLabel = new QLabel((m_sFileName+"#####." + m_sFileExt), this);

  m_go = new QPushButton("Go", this);
  m_cancel = new QPushButton("Cancel", this);

  //2. Make connections
  connect(m_selectNameButton, SIGNAL(clicked()), this, SLOT(ShowFileDialog()));
  connect(m_cancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_go, SIGNAL(clicked()), this, SLOT(SaveImages()));

  //3. Set up layout
  SetUpLayout(); 
}

void 
MovieSaveDialog::SetUpLayout(){

  m_layout = new QGridLayout;
  this->setLayout(m_layout); 

  m_layout->addWidget(m_startFrameLabel, 1, 1); 
  m_layout->addWidget(m_startFrameEdit, 1, 2); 
  m_layout->addWidget(m_endFrameLabel, 2, 1); 
  m_layout->addWidget(m_endFrameEdit, 2, 2); 
  m_layout->addWidget(m_stepSizeLabel, 3, 1); 
  m_layout->addWidget(m_stepSizeEdit, 3, 2);  
 
  m_layout->addWidget(m_selectNameButton, 4, 1); 
  m_layout->addWidget(m_fileNameLabel, 4, 2); 
  
  m_layout->addWidget(m_go, 5, 1); 
  m_layout->addWidget(m_cancel, 5, 2);
}

void
MovieSaveDialog::SaveImages(){

  StoreAttributes(); 
  QDialog::accept(); 
}

void 
MovieSaveDialog::UpdateAttributes(){

  QString tempStorage;

  tempStorage.setNum(m_startIntFrame);
  m_startFrameEdit->setText(tempStorage);

  tempStorage.setNum(m_endIntFrame);
  m_endFrameEdit->setText(tempStorage);

  tempStorage.setNum(m_stepIntSize);
  m_stepSizeEdit->setText(tempStorage);
}

void 
MovieSaveDialog::StoreAttributes(){

  QString tempStorage;
  bool conv;

  tempStorage = m_startFrameEdit->text();
  m_startIntFrame = tempStorage.toInt(&conv, 10);

  tempStorage = m_endFrameEdit->text();
  m_endIntFrame = tempStorage.toInt(&conv, 10);

  tempStorage = m_stepSizeEdit->text();
  m_stepIntSize = tempStorage.toInt(&conv, 10);    
}

void 
MovieSaveDialog::ShowFileDialog(){

  QFileDialog* fd = new QFileDialog(this, "File name", ".", QString::null);
  fd->setFileMode(QFileDialog::AnyFile); 
  fd->setAcceptMode(QFileDialog::AcceptSave); 
  fd->setFilters(Filters());
  if(fd->exec() == QDialog::Accepted){
    QStringList files = fd->selectedFiles(); 
    if(!files.isEmpty())
      m_sFileName = files[0]; 
    m_sFileExt = Filter2Ext(fd->selectedFilter().toStdString()).c_str(); 
    m_fileNameLabel->setText(m_sFileName + m_sFileExt); 

    //find digit
    int _s = m_sFileName.indexOf('#');
    int _g = m_sFileName.lastIndexOf('#');
    m_frameDigit=_g-_s;
    if(_s == _g) 
      m_frameDigit=4;
    m_sFileName = m_sFileName.left(_s);
  }
}












