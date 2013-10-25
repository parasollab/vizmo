////////////////////////////////////////////////////////////////////////////////
//A custom-made box (unfortunately) for multi-field input to set camera
//rotation coordinates.
////////////////////////////////////////////////////////////////////////////////

#ifndef CAMERA_POS_DIALOG_H
#define CAMERA_POS_DIALOG_H

#include <QDialog>

class QDialogButtonBox;
class QFormLayout;
class QLabel;
class QLineEdit;

class Camera;

class CameraPosDialog : public QDialog {

  Q_OBJECT

  public:
    CameraPosDialog(QWidget* _parent);
    void SetCamera(Camera* _camera);

  private slots:
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

    Camera* m_camera;
};

#endif // CAMERA_POS_DIALOG_H
