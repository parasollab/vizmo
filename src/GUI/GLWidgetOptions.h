#ifndef GL_WIDGET_OPTIONS_H_
#define GL_WIDGET_OPTIONS_H_

#include "OptionsBase.h"

class CameraPosDialog;
class Model;


////////////////////////////////////////////////////////////////////////////////
/// \brief Provides functions related to the OpenGL scene.
////////////////////////////////////////////////////////////////////////////////
class GLWidgetOptions : public OptionsBase {

  Q_OBJECT

  typedef vector<Model*>::iterator MIT;

  public:

    GLWidgetOptions(QWidget* _parent);

  private slots:

    //gl functions
    void ResetCamera();           ///< Return the camera to its starting position.
    void SetCameraPosition();     ///< Set the camera position.
    void ChangeBGColor();         ///< Change the background color.
    void ChangeObjectColor();     ///< Change the selected models' colors.
    void ShowGeneralContextMenu();///< Display the right-click menu.
    void MakeSolid();             ///< Render the selected object in solid mode.
    void MakeWired();             ///< Render the selected object in wire mode.
    void MakeInvisible();         ///< Don't render the selected object.
    void ShowObjectNormals();     ///< Display normals of the selected model.
    void SaveCameraPosition();    ///< Save the current viewing perspective.
    void LoadCameraPosition();    ///< Load a viewing perspective from file.
    void ToggleCameraFree();      ///< Toggle camera free-floating mode.
    void ResetCameraUp();         ///< Reset the camera's up direction.

  private:

    //gui management
    void CreateActions();
    void SetHelpTips();
    void SetUpSubmenu();
    void SetUpToolTab();
    void Reset();

    QPointer<CameraPosDialog> m_cameraPosDialog; ///< The camera position dialog.
    QMenu* m_modifySelected;                  ///< Submenu for rendering options.
};

#endif
