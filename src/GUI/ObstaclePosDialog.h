#ifndef OBSTACLEPOSDIALOG_H_
#define OBSTACLEPOSDIALOG_H_

#include <QDialog>

#include "Models/BodyModel.h"

using namespace std;

class QPushButton;
class QLineEdit;
class QLabel;
class QValidator;
class QSlider;
class EnvModel;
class MultiBodyModel;
class MainWindow;

class ObstaclePosDialog : public QDialog {

  Q_OBJECT

  public:
    ObstaclePosDialog(MultiBodyModel* _multiBody, MainWindow* _mainWindow,  QWidget* _parent);

  public slots:
   // void AddToEnv();
    void DisplaySlidersValues(int _i);
    void ChangeSlidersValues();

  private:
    //Functions
    void SetUpLayout();
    void SetSlidersInit();
    void GetBoundingValues();
    void RefreshPosition();
    string GetCoord();
    //Model Variables
    EnvModel* m_envModel;
    MultiBodyModel* m_multiBody;
    BodyModel* m_obstacle;
    MainWindow* m_mainWindow;
    //Obstacle Variables
    string m_stringCoord;
    pair<int, int> m_boundingValues[3];
    bool m_valueEdited;
    //Qt Variables
    QPushButton* m_loadButton;
    QLineEdit* m_posLines[6];
    QLabel* m_xPosLabel;
    QLabel* m_yPosLabel;
    QLabel* m_zPosLabel;
    QLabel* m_xRotLabel;
    QLabel* m_yRotLabel;
    QLabel* m_zRotLabel;
    QLabel* m_posLabel;
    QLabel* m_coordLabel;
    QLabel* m_rotLabel;
    QSlider* m_sliders[6];
    QValidator* m_validator;
};

#endif

