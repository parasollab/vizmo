#ifndef TRANSFORM_TOOL_H_
#define TRANSFORM_TOOL_H_

#include <Vector.h>
#include <Quaternion.h>
#include <Transformation.h>
using namespace mathtool;

#include <QObject>
#include <QKeyEvent>

////////////////////////////////////////////////////////////////////////////////
/// @brief Transform tool to set a transformation by moving click-and-drag
///        object in GL scene
///
/// @todo Make 2d version of transform tool
/// @todo make joint version of transform tool
/// @todo like transform to objects
////////////////////////////////////////////////////////////////////////////////
class TransformTool : public QObject {

  Q_OBJECT

  public:

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Movement type after selecting with mouse
    ////////////////////////////////////////////////////////////////////////////
    enum class MovementType {
      None,      ///< No movement
      XAxis,    ///< X translation
      YAxis,    ///< Y translation
      ZAxis,    ///< Z translation
      XAxisRot, ///< Rotation around X-axis of model
      YAxisRot, ///< Rotation around Y-axis of model
      ZAxisRot, ///< Rotation around Z-axis of model
      ViewPlane ///< Translation in viewing plane
    };

    ////////////////////////////////////////////////////////////////////////////
    /// @name Construction
    /// @{

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Constructor
    TransformTool(const Transformation& _t = Transformation());

    /// @}
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    /// @name Modifiers
    /// @{

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Turn rotations on and off
    /// @param _on On/off
    void SetRotationsOn(bool _on) {m_rotationsOn = _on;}

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Set transform of tool
    /// @param _t Transformation
    void SetTransform(const Transformation& _t);

    /// @}
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    /// @name Rendering
    /// @{

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Draw transformation tool for GL_RENDER
    void Draw();
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Draw transformation tool for GL_SELECT
    void DrawSelect();
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Select portion of object for transformation
    /// @param _x X window coordinate
    /// @param _y Y window coordinate
    bool Select(int _x, int _y);

    /// @}
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    /// @name Event handling
    /// @{

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Mouse pressed action
    /// @param _e Mouse Event
    bool MousePressed(QMouseEvent* _e);
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Mouse released action
    /// @param _e Mouse Event
    bool MouseReleased(QMouseEvent* _e);
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Mouse motion action
    /// @param _e Mouse Event
    bool MouseMotion(QMouseEvent* _e);
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Key pressed action
    /// @param _e Key Event
    bool KeyPressed(QKeyEvent* _e);

    /// @}
    ////////////////////////////////////////////////////////////////////////////

  signals:

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Signal of new translation
    /// @param _t Translation
    void TranslationChanged(const Vector3d& _t);
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Signal of new rotation
    /// @param _r Rotation
    void RotationChanged(const Quaternion& _r);

  public slots:

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Set new translation
    /// @param _t Translation
    void ChangeTranslation(const Vector3d& _t);
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Set new rotation
    /// @param _r Rotation
    void ChangeRotation(const Quaternion& _r);

  private:
    bool m_rotationsOn;          ///< Consider rotations or not
    MovementType m_movementType; ///< Which axis is selected
    Point3d m_translation;       ///< World vector for translation
    Point3d m_oldTranslation;    ///< Old translation
    Quaternion m_rotation;       ///< Rotation
    Quaternion m_oldRotation;    ///< Old rotation
    int m_hitX, m_hitY;          ///< Mouse hit in window coordinates
    Point3d m_hitWorld;          ///< World position of (m_hitX, m_hitY)
};

#endif
