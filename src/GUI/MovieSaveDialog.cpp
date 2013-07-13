#include "MovieSaveDialog.h"

#include <limits>

#include <QGridLayout> 
#include <QLabel>
#include <QLineEdit>   
#include <QIntValidator> 
#include <QPushButton> 
#include <QFileDialog> 

#include "vizmo2.h"
#include "Utilities/ImageFilters.h"

MovieSaveDialog::MovieSaveDialog(QWidget* _parent, Qt::WFlags _f) :
  QDialog(_parent, _f), 
  m_startFrame(0), m_endFrame(-1), m_stepSize(10),
  m_frameDigits(5), m_frameDigitStart(11),
  m_filename("vizmo_movie#####.jpg") {

    int maxint = std::numeric_limits<int>::max();

    if(GetVizmo().GetPathSize() > 0){ 
      m_endFrame = GetVizmo().GetPathSize()-1;
    }
    else if(GetVizmo().GetDebugSize() > 0){
      m_endFrame = GetVizmo().GetDebugSize()-1;
    }

    //1. Create subwidgets/members

    QString tmp;
    m_startFrameLabel = new QLabel("Start Frame", this);
    tmp.setNum(m_startFrame);
    m_startFrameEdit = new QLineEdit(tmp, this);
    m_startFrameEdit->setValidator(new QIntValidator(0, maxint, this));   

    m_endFrameLabel = new QLabel("EndFrame", this); 
    tmp.setNum(m_endFrame);
    m_endFrameEdit = new QLineEdit(tmp, this);
    m_endFrameEdit->setValidator(new QIntValidator(0, maxint, this));

    m_stepSizeLabel = new QLabel("Step Size", this); 
    tmp.setNum(m_stepSize);
    m_stepSizeEdit = new QLineEdit(tmp, this);
    m_stepSizeEdit->setValidator(new QIntValidator(0, maxint, this));

    m_selectNameButton = new QPushButton("Select Name", this);
    m_fileNameLabel = new QLabel(m_filename, this);

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
  m_startFrame = m_startFrameEdit->text().toUInt();
  m_endFrame = m_endFrameEdit->text().toUInt();
  m_stepSize = m_stepSizeEdit->text().toUInt();
  
  QDialog::accept(); 
}

void 
MovieSaveDialog::ShowFileDialog(){
  //set up the file dialog to select image filename
  QFileDialog fd(this, "Choose a name", ".", QString::null);
  fd.setFileMode(QFileDialog::AnyFile); 
  fd.setAcceptMode(QFileDialog::AcceptSave); 
  fd.setFilters(imageFilters);

  //if filename exists save image
  if(fd.exec() == QDialog::Accepted){
    QStringList files = fd.selectedFiles(); 
    if(!files.isEmpty()) {
      m_filename = GrabFilename(files[0], fd.selectedFilter());
      m_fileNameLabel->setText(m_filename); 

      //find digit
      int f = m_filename.indexOf('#');
      int l = m_filename.lastIndexOf('#');
      m_frameDigits = l - f + 1;
      m_frameDigitStart = f;
      if(m_frameDigitStart == size_t(-1)){
        m_frameDigits = 0;
        m_frameDigitStart = m_filename.lastIndexOf('.');
      }
    }
  }
}

