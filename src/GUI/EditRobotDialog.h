#ifndef EDITROBOTDIALOG_H_
#define EDITROBOTDIALOG_H_

#include <iostream>
#include <QDialog>

using namespace std;

class MultiBodyModel;
class ConnectionModel;
class BodyModel;
class MainWindow;

class QPushButton;
class QLineEdit;
class QDoubleSpinBox;
class QLabel;
class QValidator;
class QCheckBox;
class QListWidget;
class QMessageBox;

class EditRobotDialog : public QDialog {

  Q_OBJECT

  public:
    EditRobotDialog(MainWindow* _mainWindow, QWidget* _parent);
    typedef vector<ConnectionModel*> Joints;
    typedef pair<BodyModel*, Joints> Robot;
    typedef vector<Robot> Robots;

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
    void TypeUpdateBase(int _state);
    void UpdateBase();
    void CreateNewRobot();
    void JointNonActuatedChecked();
    void JointSphericalChecked();
    void JointRevoluteChecked();
    void DeleteJoint();
    void AddJoint();
    void SaveJoint();
    void UpdateJoint();
    void TypeUpdateJoint(int _state);
    void Accept();

  private:
    //Functions
    void SetUpLayout();
    void DisplayBases();
    void BaseNotFixedChecked();
    void DisplayHideBaseAttributes(bool _visible);
    void DisplayHideJointAttributes(bool _visible);
    void JointNewTypeChecked(bool _isTrue);
    void ClearJointLines();
    void OpenMessageBox(string _message);
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
    QLabel* m_baseAreaLabel;
    QLabel* m_jointAreaLabel;
    QLabel* m_attributesLabel;
    QLineEdit* m_directoryLine;
    QLabel* m_directoryLabel;
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
    QLabel* m_baseTypeLabel;
    QLabel* m_baseMovementLabel;
    QValidator* m_validator;
    QMessageBox* m_messageBox;
    QPushButton* m_jointSaveAttributesButton;
    QPushButton* m_jointRestaureAttributesButton;
    QCheckBox* m_jointSphericalCheck;
    QLabel* m_jointTypeLabel;
    QCheckBox* m_jointRevoluteCheck;
    QLabel* m_jointLimitsLabel;
    QDoubleSpinBox* m_jointLimits[2][2];
    QLabel* m_jointLimitLabel1;
    QLabel* m_jointLimitLabel2;
    QLabel* m_jointPositionLabel11;
    QLabel* m_jointPositionLabel12;
    QCheckBox* m_jointNonActuatedCheck;
    QDoubleSpinBox* m_jointPos[2][6];
    QLabel* m_jointXLabel[2];
    QLabel* m_jointYLabel[2];
    QLabel* m_jointZLabel[2];
    QLabel* m_jointAlphaLabel[2];
    QLabel* m_jointBetaLabel[2];
    QLabel* m_jointGammaLabel[2];
    QLabel* m_jointPositionLabel22;
    QLabel* m_jointPositionLabel21;
    QLabel* m_jointDHParametersLabel1;
    QLabel* m_jointDHParametersLabel2;
    QLineEdit* m_jointAlphaLine;
    QLineEdit* m_jointALine;
    QLineEdit* m_jointDLine;
    QLineEdit* m_jointThetaLine;
    QLabel* m_jointAlphaDHLabel;
    QLabel* m_jointALabel;
    QLabel* m_jointDLabel;
    QLabel* m_jointThetaDHLabel;
    QPushButton* m_newRobotButton;
    QLabel* m_bodyNumberLabel;
    QLineEdit* m_bodyNumberLine;
    QLabel* m_jointConnectionsLabel;
    QLineEdit* m_jointConnectionsLine1;
    QLineEdit* m_jointConnectionsLine2;
};

#endif


