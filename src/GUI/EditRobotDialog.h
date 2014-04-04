#ifndef EDITROBOTDIALOG_H_
#define EDITROBOTDIALOG_H_

using namespace std;

#include <QtGui>

#include "Models/MultiBodyModel.h"

class MainWindow;

class EditRobotDialog : public QDialog {

  Q_OBJECT

  public:
    typedef MultiBodyModel::Joints Joints;
    typedef MultiBodyModel::Robot Robot;
    typedef MultiBodyModel::Robots Robots;

    EditRobotDialog(MainWindow* _mainWindow, QWidget* _parent);

  public slots:
    void ShowBase();
    void ShowJoint();
    void BaseFixedChecked();
    void BasePlanarChecked();
    void BaseVolumetricChecked();
    void BaseTranslationalChecked();
    void BaseRotationalChecked();
    void DeleteBase();
    void AddBase();
    void SaveBase();
    void UpdateBase();
    void CreateNewRobot();
    void JointNonActuatedChecked();
    void JointSphericalChecked();
    void JointRevoluteChecked();
    void DeleteJoint();
    void AddJoint();
    void SaveJoint();
    void UpdateJoint(bool _clicked = true);
    void Accept();

  private:
    //Functions
    void SetUpLayout();
    void DisplayBases();
    void BaseNotFixedChecked();
    void DisplayHideBaseAttributes(bool _visible);
    void DisplayHideJointAttributes(bool _visible);
    void DeleteAllExceptLastBase();
    void SaveJointsNames();
    void ChangeDOF();
    void RefreshVizmo();
    bool JointParamChecked();
    string SaveMultiBody();

    //Variables
    Robots m_newRobotModel, m_oldRobotModel;
    MultiBodyModel* m_robotBody;
    MainWindow* m_mainWindow;
    vector<string> m_oldJointsNames;
    vector<string> m_oldJointsDirectories;
    bool m_jointIsInit;
    bool m_baseIsInit;

    //Qt Variables
    QPushButton* m_okButton;
    QLabel* m_directory;
    QPushButton* m_addBaseButton;
    QPushButton* m_deleteBaseButton;
    QPushButton* m_addJointButton;
    QPushButton* m_deleteJointButton;
    QListWidget* m_baseList;
    QListWidget* m_jointList;
    QCheckBox* m_baseFixedCheck;
    QCheckBox* m_basePlanarCheck;
    QCheckBox* m_baseVolumetricCheck;
    QCheckBox* m_baseTranslationalCheck;
    QCheckBox* m_baseRotationalCheck;

    QWidget* m_baseWidget, * m_jointWidget;

    QPushButton* m_jointSaveAttributesButton;
    QPushButton* m_jointRestaureAttributesButton;
    QCheckBox* m_jointSphericalCheck;
    QCheckBox* m_jointRevoluteCheck;
    QDoubleSpinBox* m_jointLimits[2][2];
    QCheckBox* m_jointNonActuatedCheck;
    QDoubleSpinBox* m_jointPos[2][6];
    QLineEdit* m_jointAlphaLine;
    QLineEdit* m_jointALine;
    QLineEdit* m_jointDLine;
    QLineEdit* m_jointThetaLine;
    QPushButton* m_newRobotButton;
    QLineEdit* m_bodyNumberLine;
    QLineEdit* m_jointConnectionsLine1;
    QLineEdit* m_jointConnectionsLine2;
};

#endif


