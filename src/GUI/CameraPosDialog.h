/*****************************************************************************
** A custom-made box (unfortunately) for multi-field input to set camera 
**rotation coordinates.  
********************************************************************************/

#ifndef CAMERA_POS_DIALOG_H
#define CAMERA_POS_DIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QWidget>

class CameraPosDialog : public QDialog {
 
  Q_OBJECT

  public:
    CameraPosDialog(QWidget* _parent = 0); 

    QDialogButtonBox* m_buttonBox;
    QLabel* m_label;
    QWidget* m_formLayoutWidget;
    QFormLayout* m_formLayout;
    QLabel* m_xLabel;
    QLineEdit* m_xLineEdit;
    QLabel* m_yLabel;
    QLineEdit* m_yLineEdit;
    QLabel* m_zLabel;
    QLineEdit* m_zLineEdit;
    QWidget* m_formLayoutWidget_2;
    QFormLayout* m_formLayout_2;
    QLabel* m_azimLabel;
    QLabel* m_elevLabel; 
    QLineEdit* m_azimLineEdit;
    QLineEdit* m_elevLineEdit; 

    public slots: 
      void AcceptData(); 

  void SetUpDialog(QDialog* _dialog){
                                                                      
      _dialog->resize(450, 245); 
      _dialog->setWindowTitle("Set Camera Position and Rotation");  

      m_buttonBox = new QDialogButtonBox(_dialog);
      m_buttonBox->setGeometry(QRect(100, 210, 341, 32));
      m_buttonBox->setOrientation(Qt::Horizontal);
      m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    
      m_label = new QLabel(_dialog);
      m_label->setText("Enter a 3-D point and rotation angle to position the camera"); 
      m_label->setGeometry(QRect(10, 30, 421, 16));

      m_formLayoutWidget = new QWidget(_dialog);
      m_formLayoutWidget->setGeometry(QRect(30, 70, 172, 95));
      m_formLayout = new QFormLayout(m_formLayoutWidget);
      m_formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
      m_formLayout->setContentsMargins(0, 0, 0, 0);
    
      m_xLabel = new QLabel(m_formLayoutWidget);
      m_xLabel->setText("X:"); 
      m_formLayout->setWidget(0, QFormLayout::LabelRole, m_xLabel);
    
      m_xLineEdit = new QLineEdit(m_formLayoutWidget);
      m_formLayout->setWidget(0, QFormLayout::FieldRole, m_xLineEdit);
    
      m_yLabel = new QLabel(m_formLayoutWidget);
      m_yLabel->setText("Y:"); 
      m_formLayout->setWidget(1, QFormLayout::LabelRole, m_yLabel);
    
      m_yLineEdit = new QLineEdit(m_formLayoutWidget);
      m_formLayout->setWidget(1, QFormLayout::FieldRole, m_yLineEdit);
    
      m_zLabel = new QLabel(m_formLayoutWidget);
      m_zLabel->setText("Z:"); 
      m_formLayout->setWidget(2, QFormLayout::LabelRole, m_zLabel);
    
      m_zLineEdit = new QLineEdit(m_formLayoutWidget);
      m_formLayout->setWidget(2, QFormLayout::FieldRole, m_zLineEdit);
    
      m_formLayoutWidget_2 = new QWidget(_dialog);
      m_formLayoutWidget_2->setGeometry(QRect(220, 70, 213, 80));
      m_formLayout_2 = new QFormLayout(m_formLayoutWidget_2);
      m_formLayout_2->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
      m_formLayout_2->setContentsMargins(0, 0, 0, 0);
    
      m_azimLabel = new QLabel(m_formLayoutWidget_2);
      m_azimLabel->setText("Azimuth:");
      m_elevLabel = new QLabel(m_formLayoutWidget_2);
      m_elevLabel->setText("Elevation:"); 
      m_formLayout_2->setWidget(0, QFormLayout::LabelRole, m_azimLabel);
      m_formLayout_2->setWidget(1, QFormLayout::LabelRole, m_elevLabel);
      
      m_azimLineEdit = new QLineEdit(m_formLayoutWidget_2);
      m_elevLineEdit = new QLineEdit(m_formLayoutWidget_2); 
      m_formLayout_2->setWidget(0, QFormLayout::FieldRole, m_azimLineEdit);
      m_formLayout_2->setWidget(1, QFormLayout::FieldRole, m_elevLineEdit);
      
      QObject::connect(m_buttonBox, SIGNAL(accepted()), _dialog, SLOT(AcceptData()));
      QObject::connect(m_buttonBox, SIGNAL(rejected()), _dialog, SLOT(reject()));

      QMetaObject::connectSlotsByName(_dialog);
    
  } 
};

#endif // CAMERA_POS_DIALOG_H 





