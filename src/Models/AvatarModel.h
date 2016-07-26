#ifndef AVATAR_MODEL_H_
#define AVATAR_MODEL_H_

#include "CfgModel.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
/// \brief   A tool for displaying a user-input configuration.
/// \details The avatar is controlled by one of mouse click-and-drag, camera
///          following, or haptic input. The control device is indicated by
///          m_input, and the avatar configuration will move in response to
///          input from that device.
////////////////////////////////////////////////////////////////////////////////
class AvatarModel : public CfgModel {

  public:

    // Input device type
    enum InputType {None = 0, Mouse, CameraPath, Haptic};

    // Construction
    AvatarModel(InputType _t = Mouse, const CfgModel& _initialCfg = CfgModel());

    // Access functions
    const InputType GetInputType() const {return m_input;}
    bool IsTracking() const {return m_tracking;}

    // Control functions
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Change the input device.
    /// \param[in] _i The new input device to use.
    void SetInputType(InputType _i) {m_input = _i;}
    void Enable() {m_tracking = true;}    ///< Make avatar follow user input.
    void Disable() {m_tracking = false;}  ///< Make avatar ignore user input.
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Update the avatar's position in workspace.
    /// \param[in] _p The avatar's new position.
    void UpdatePosition(Point3d _p);

    // Model functions
    void DrawRender();              ///< Render the avatar.
    void DrawSelect() {}            ///< This object cannot be selected.
    void DrawSelected() {}          ///< This object cannot be selected.
    void Print(ostream& _os) const; ///< Print input device and cfg data.

    // Mouse event handling
    ////////////////////////////////////////////////////////////////////////////
    /// \brief Capture mouse motion when using Mouse input type.
    /// \param[in] _e The mouse event.
    /// \param[in] _c The camera perspective.
    /// \return       A bool indicating whether the event was handled or
    ///               ignored. Always returns \c false if input type is not
    ///               Mouse.
    bool PassiveMouseMotion(QMouseEvent* _e, Camera* _c);

  private:

    InputType m_input;  ///< The device used to control the avatar.
    bool m_tracking;    ///< Indicates whether the avatar is currently in use.
};

#endif
