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

    void SetCameraPos(double _x, double _y, double _z, double _azim, double _elev);

  public slots:
      void AcceptData();

  private:
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
};

#endif // CAMERA_POS_DIALOG_H
