////////////////////////////////////////////////////////////////////////////////
//A custom-made box (unfortunately) for multi-field input to set camera
//rotation coordinates.
////////////////////////////////////////////////////////////////////////////////

#ifndef CAMERA_POS_DIALOG_H
#define CAMERA_POS_DIALOG_H

#include <QtGui>
#include <fstream>
class Camera;
class MainWindow;

class CameraPosDialog : public QDialog {

  Q_OBJECT

  public:
    CameraPosDialog(MainWindow* _mainWindow, Camera* _camera);
    ~CameraPosDialog();
    void SetCamera(Camera* _camera);
    bool SaveCameraPosition();
    void LoadCameraPosition();

  private slots:
    void AcceptData();

  private:
    QLineEdit* m_lineEye[3];
    QLineEdit* m_lineAt[3];

    Camera* m_camera;
    MainWindow* m_mainWindow;
};

#endif // CAMERA_POS_DIALOG_H
