#ifndef OBSTACLEPOSDIALOG_H_
#define OBSTACLEPOSDIALOG_H_

#include <QtGui>

#include "Models/BodyModel.h"

class EnvModel;
class MainWindow;
class MultiBodyModel;

class ObstaclePosDialog : public QDialog {

  Q_OBJECT

  public:
    ObstaclePosDialog(const vector<MultiBodyModel*>& _multiBody, MainWindow* _mainWindow,  QWidget* _parent);

  public slots:
    void DisplaySlidersValues(int _i);
    void ChangeSlidersValues();

  private:
    void SetUpLayout();
    void SetSlidersInit();

    void RefreshPosition();

    //Model Variables
    vector<MultiBodyModel*> m_multiBody;
    bool m_oneObst;

    //Obstacle Variables
    Vector3d m_center;
    bool m_valueEdited;

    //Qt Variables
    MainWindow* m_mainWindow;
    QLineEdit* m_posLines[6];
    QSlider* m_sliders[6];
};

#endif

