////////////////////////////////////////////////////////////////////////////////
///  This class is a tool for displaying a robot configuration in user-guided
///  methods. Drawing is managed by a TempObjsModel, while DOF updates are
///  handled with signals/slots.
////////////////////////////////////////////////////////////////////////////////

#ifndef ROBOT_AVATAR_H_
#define ROBOT_AVATAR_H_

#include <QtGui>

class Camera;
class CfgModel;
class TempObjsModel;

class RobotAvatar : public QObject {

  Q_OBJECT

  public:
    //specify input type
    enum InputType {Mouse = 0, CameraPath, Haptic};

    RobotAvatar(InputType _t = Mouse);
    RobotAvatar(CfgModel* _initialCfg, InputType _t = Mouse);
    ~RobotAvatar();

  public slots:
    void Connect();
    void Disconnect();

    void UpdateMouse();
    void UpdateCameraPath();
    void UpdateHaptic();

  private:
    InputType m_input;
    TempObjsModel* m_tempObjs;
    CfgModel* m_avatar;

    Camera* m_camera;
};

#endif
