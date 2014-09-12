////////////////////////////////////////////////////////////////////////////////
///  This class is a tool for displaying a robot configuration in user-guided
///  methods.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef AVATAR_MODEL_H_
#define AVATAR_MODEL_H_

#include "CfgModel.h"

using namespace std;

class AvatarModel : public CfgModel {

  public:
    //specify input type
    enum InputType {None = 0, Mouse, CameraPath, Haptic};

    AvatarModel(InputType _t = Mouse, const CfgModel& _initialCfg = CfgModel());

    //Access functions
    const InputType GetInputType() const {return m_input;}
    bool IsTracking() const {return m_tracking;}

    //Control functions
    void SetInputType(InputType _i) {m_input = _i;}
    void Enable() {m_tracking = true;}
    void Disable() {m_tracking = false;}
    void UpdatePosition(Point3d _p);

    //Model functions
    void DrawRender();
    void DrawSelect() {}
    void DrawSelected() {}
    void Print(ostream& _os) const;

    //mouse event handling
    bool PassiveMouseMotion(QMouseEvent* _e, Camera* _c);

  private:
    InputType m_input;
    bool m_tracking;
};

#endif
