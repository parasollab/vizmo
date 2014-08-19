////////////////////////////////////////////////////////////////////////////////
///  This class is a tool for displaying a robot configuration in user-guided
///  methods. Drawing is managed by a TempObjsModel, while DOF updates are
///  handled with signals/slots.
////////////////////////////////////////////////////////////////////////////////

#ifndef ROBOT_AVATAR_H_
#define ROBOT_AVATAR_H_

#include <QtGui>

#include "Models/CfgModel.h"

class Camera;
class TempObjsModel;

using namespace std;

class RobotAvatar : public QObject {

  Q_OBJECT

  public:
    //specify input type
    enum InputType {Mouse = 0, CameraPath, Haptic};

    RobotAvatar(InputType _t = Mouse);
    RobotAvatar(CfgModel* _initialCfg, InputType _t = Mouse);
    ~RobotAvatar();

    //access data
    const vector<double> GetCurrPos() const {return m_avatar->GetPosition();}
    const vector<double> GetPrevPos() const {return m_prev->GetPosition();}
    const vector<double> GetCurrRot() const {return m_avatar->GetOrientation();}
    const vector<double> GetPrevRot() const {return m_prev->GetOrientation();}
    const CfgModel& GetCurrCfg() const {return *m_avatar;}
    const CfgModel& GetPrevCfg() const {return *m_prev;}

    //update current/prev position
    void UpdatePos() {m_prev->SetData(m_avatar->GetData());}

    //bring mouse to avatar
    void SummonMouse();

    //enable/disable signals/slots
    void Connect();
    void Disconnect();

  public slots:
    void UpdateMouse();
    void UpdateCameraPath();
    void UpdateHaptic();

  private:
    InputType m_input;
    TempObjsModel* m_tempObjs;
    CfgModel* m_avatar, * m_prev;

    Camera* m_camera;
};

#endif
