#ifndef CAMERA_POS_DIALOG_H
#define CAMERA_POS_DIALOG_H

#include <QtGui>
#include <QDialog>

#include <fstream>

class QLineEdit;

class Camera;
class MainWindow;

////////////////////////////////////////////////////////////////////////////////
/// \brief This dialog allows the user to set, save, or load a Camera
/// perspective.
////////////////////////////////////////////////////////////////////////////////
class CameraPosDialog : public QDialog {

  Q_OBJECT

  public:

    CameraPosDialog(MainWindow* _mainWindow, Camera* _camera);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Execute the Camera position dialog for a Camera.
    /// \param[in] _camera The Camera to position.
    void SetCamera(Camera* _camera);
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Save the current position and orientation of the current Camera.
    /// \return A \c bool indicating whether the user accepted or cancelled the
    /// save.
    bool SaveCameraPosition();
    void LoadCameraPosition(); ///< Load a saved Camera perspective.

  private slots:

    void AcceptData(); ///< Set the perspective of the current Camera.

  private:

    QLineEdit* m_lineEye[3];  ///< GUI component for editing the Camera position.
    QLineEdit* m_lineAt[3];   ///< GUI component for editing the viewing target.

    Camera* m_camera;         ///< Points to the current Camera.
    MainWindow* m_mainWindow; ///< Points to the MainWindow.
};

#endif
