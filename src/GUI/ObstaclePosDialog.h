#ifndef OBSTACLE_POS_DIALOG_H_
#define OBSTACLE_POS_DIALOG_H_

#include <QtGui>
#include <QDialog>

#include "Models/BodyModel.h"

#include "Utilities/TransformTool.h"

class QLineEdit;
class QSlider;

class EnvModel;
class MainWindow;
class StaticMultiBodyModel;

class ObstaclePosDialog : public QDialog {

  Q_OBJECT

  public:
    ObstaclePosDialog(MainWindow* _mainWindow,
        const vector<StaticMultiBodyModel*>& _multiBody);
    ~ObstaclePosDialog();

  signals:
    void TranslationChanged(const Vector3d& _t);
    void RotationChanged(const Quaternion& _r);

  public slots:
    void DisplaySlidersValues(int _i);
    void ChangeSlidersValues();
    void ChangeTranslation(const Vector3d& _t);
    void ChangeRotation(const Quaternion& _r);

  private:
    void SetUpLayout();
    void SetSlidersInit();

    void RefreshPosition(bool _emit);

    //Model Variables
    vector<StaticMultiBodyModel*> m_multiBody;
    bool m_oneObst;

    //Obstacle Variables
    Vector3d m_center;

    //Qt Variables
    QLineEdit* m_posLines[6];
    QSlider* m_sliders[6];

    TransformTool m_transformTool;
};

#endif

