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
    void SaveBase();
    void DeleteBase();
    void AddBase();
    void CreateNewRobot();
    void JointNonActuatedChecked();
    void JointSphericalChecked();
    void JointRevoluteChecked();
    void DeleteJoint();
    void AddJoint();
    void SaveJoint();
    void Accept();

  private:
    //Functions
    void SetUpLayout();
    void DisplayBases();
    void BaseNotFixedChecked();
    void ClearBaseLines();
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
    QLabel* m_basePositionLabel;
    QLabel* m_baseRotationLabel;
    QLineEdit* m_baseXLine;
    QLabel* m_baseXLabel;
    QLineEdit* m_baseYLine;
    QLabel* m_baseYLabel;
    QLineEdit* m_baseZLine;
    QLabel* m_baseZLabel;
    QLineEdit* m_baseXRotLine;
    QLabel* m_baseXRotLabel;
    QLineEdit* m_baseYRotLine;
    QLabel* m_baseYRotLabel;
    QLineEdit* m_baseZRotLine;
    QLabel* m_baseZRotLabel;
    QPushButton* m_saveAttributesButton;
    QPushButton* m_restaureAttributesButton;
    QValidator* m_validator;
    QMessageBox* m_messageBox;
    QPushButton* m_jointSaveAttributesButton;
    QPushButton* m_jointRestaureAttributesButton;
    QCheckBox* m_jointSphericalCheck;
    QLabel* m_jointTypeLabel;
    QCheckBox* m_jointRevoluteCheck;
    QLabel* m_jointLimitsLabel;
    QLineEdit* m_jointLimitsLine11;
    QLineEdit* m_jointLimitsLine12;
    QLineEdit* m_jointLimitsLine21;
    QLineEdit* m_jointLimitsLine22;
    QLabel* m_jointLimitLabel1;
    QLabel* m_jointLimitLabel2;
    QLabel* m_jointPositionLabel11;
    QLabel* m_jointPositionLabel12;
    QCheckBox* m_jointNonActuatedCheck;
    QLineEdit* m_jointX1Line;
    QLineEdit* m_jointY1Line;
    QLineEdit* m_jointZ1Line;
    QLineEdit* m_jointXRot1Line;
    QLineEdit* m_jointYRot1Line;
    QLineEdit* m_jointZRot1Line;
    QLabel* m_jointX1Label;
    QLabel* m_jointY1Label;
    QLabel* m_jointZ1Label;
    QLabel* m_jointXRot1Label;
    QLabel* m_jointYRot1Label;
    QLabel* m_jointZRot1Label;
    QLineEdit* m_jointY2Line;
    QLabel* m_jointPositionLabel22;
    QLineEdit* m_jointZ2Line;
    QLabel* m_jointY2Label;
    QLineEdit* m_jointZRot2Line;
    QLabel* m_jointPositionLabel21;
    QLabel* m_jointZRot2Label;
    QLabel* m_jointX2Label;
    QLabel* m_jointXRot2Label;
    QLabel* m_jointYRot2Label;
    QLineEdit* m_jointYRot2Line;
    QLineEdit* m_jointX2Line;
    QLabel* m_jointZ2Label;
    QLineEdit* m_jointXRot2Line;
    QLabel* m_jointDHParametersLabel1;
    QLabel* m_jointDHParametersLabel2;
    QLineEdit* m_jointAlphaLine;
    QLineEdit* m_jointALine;
    QLineEdit* m_jointDLine;
    QLineEdit* m_jointThetaLine;
    QLabel* m_jointAlphaLabel;
    QLabel* m_jointALabel;
    QLabel* m_jointDLabel;
    QLabel* m_jointThetaLabel;
    QPushButton* m_newRobotButton;
    QLabel* m_bodyNumberLabel;
    QLineEdit* m_bodyNumberLine;
    QLabel* m_jointConnectionsLabel;
    QLineEdit* m_jointConnectionsLine1;
    QLineEdit* m_jointConnectionsLine2;
};

#endif


