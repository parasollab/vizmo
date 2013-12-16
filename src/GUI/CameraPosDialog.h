////////////////////////////////////////////////////////////////////////////////
//A custom-made box (unfortunately) for multi-field input to set camera
//rotation coordinates.
////////////////////////////////////////////////////////////////////////////////

#ifndef CAMERA_POS_DIALOG_H
#define CAMERA_POS_DIALOG_H

#include <QtGui>

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
    QLabel* m_labelEye[4];
    QLabel* m_labelAt[4];
    QLineEdit* m_lineEye[3];
    QLineEdit* m_lineAt[3];

    Camera* m_camera;
};

#endif // CAMERA_POS_DIALOG_H
