#ifndef GL_WIDGET_OPTIONS_H_
#define GL_WIDGET_OPTIONS_H_

#include "OptionsBase.h"
#include "Models/MapModel.h"


class CameraPosDialog;

class GLWidgetOptions : public OptionsBase {

  Q_OBJECT

  public:

    typedef vector<Model*>::iterator MIT;

    GLWidgetOptions(QWidget* _parent, MainWindow* _mainWindow);

  private slots:
    void ResetCamera();
    void SetCameraPosition();
    void ChangeBGColor();
    void ChangeObjectColor();
    void ShowGeneralContextMenu(); //right-click generated menu when NOT on an object
    void MakeSolid();
    void MakeWired();
    void MakeInvisible();
    void ShowObjectNormals();
    void SaveCameraPosition();
    void LoadCameraPosition();

  private:
    void CreateActions();
    void SetUpToolbar(); //two buttons: Reset camera and change BG color
    void SetUpToolTab();
    void Reset();
    void SetHelpTips();
    void SetUpCustomSubmenu();

    QPointer<CameraPosDialog> m_cameraPosDialog;

    QMenu* m_modifySelected;
};

#endif
