#include "MovieSaveDialog.h"

#include <limits>

#include "Models/DebugModel.h"
#include "Models/PathModel.h"
#include "Models/Vizmo.h"
#include "Utilities/ImageFilters.h"

MovieSaveDialog::MovieSaveDialog(QWidget* _parent, Qt::WFlags _f) :
  QDialog(_parent, _f),

  m_startFrame(0), m_endFrame(-1), m_stepSize(10),
  m_frameDigits(5), m_frameDigitStart(11),
  m_filename("vizmo_movie#####.jpg") {

    int maxint = std::numeric_limits<int>::max();

    Vizmo& vizmo = GetVizmo();
    if(PathModel* path = vizmo.GetPath()){
      if(size_t size = path->GetSize())
        m_endFrame = size-1;
    }
    else if(DebugModel* debug = vizmo.GetDebug()){
      if(size_t size = debug->GetSize())
        m_endFrame = size-1;
    }

    //1. Create subwidgets/members
    QString tmp;
    QLabel* startFrameLabel = new QLabel("Start Frame", this);
    tmp.setNum(m_startFrame);
    m_startFrameEdit = new QLineEdit(tmp, this);
    m_startFrameEdit->setValidator(new QIntValidator(0, maxint, this));

    QLabel* endFrameLabel = new QLabel("End Frame", this);
    tmp.setNum(m_endFrame);
    m_endFrameEdit = new QLineEdit(tmp, this);
    m_endFrameEdit->setValidator(new QIntValidator(0, maxint, this));

    QLabel* stepSizeLabel = new QLabel("Step Size", this);
    tmp.setNum(m_stepSize);
    m_stepSizeEdit = new QLineEdit(tmp, this);
    m_stepSizeEdit->setValidator(new QIntValidator(0, maxint, this));

    QPushButton* selectNameButton = new QPushButton("Select Name", this);
    m_fileNameLabel = new QLabel(m_filename, this);

    QPushButton* goButton = new QPushButton("Go", this);
    QPushButton* cancelButton = new QPushButton("Cancel", this);

    //2. Make connections
    connect(selectNameButton, SIGNAL(clicked()), this, SLOT(ShowFileDialog()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(goButton, SIGNAL(clicked()), this, SLOT(SaveImages()));

    //3. Set up layout
    QGridLayout* layout = new QGridLayout;
    setLayout(layout);

    layout->addWidget(startFrameLabel, 1, 1);
    layout->addWidget(endFrameLabel, 2, 1);
    layout->addWidget(stepSizeLabel, 3, 1);
    layout->addWidget(selectNameButton, 4, 1);
    layout->addWidget(goButton, 5, 1);

    layout->addWidget(m_startFrameEdit, 1, 2);
    layout->addWidget(m_endFrameEdit, 2, 2);
    layout->addWidget(m_stepSizeEdit, 3, 2);
    layout->addWidget(m_fileNameLabel, 4, 2);
    layout->addWidget(cancelButton, 5, 2);
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

